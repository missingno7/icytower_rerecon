"""Self-tests for the experimental diagnostics (tools/diag.py), on synthetic rows.

Kept outside tests/ on purpose: diag.py is search tooling, not part of the proof context.
  python tools/diag_test.py
"""
import hashlib, tempfile, unittest
from pathlib import Path
import diag
from diag import classify, summarize


def rows(listing, original):
    out = []
    for off, asm in listing:
        r = {'off': off, 'asm': asm, 'reloc': []}
        if original: r['va'] = 0x401000 + off
        out.append(r)
    return out


# test; je else; then-store falls into a shared return tail; else-store has its own copy.
COPIED = [(0x0, 'push   %ebp'), (0x1, 'mov    %esp,%ebp'), (0x3, 'test   %eax,%eax'),
          (0x5, 'je     10 <_f+0x10>'), (0x7, 'movl   $0x1,0x8(%edx)'), (0xe, 'leave'), (0xf, 'ret'),
          (0x10, 'movl   $0x0,0x8(%edx)'), (0x17, 'leave'), (0x18, 'ret')]
# The same code with the else-store jumping back to the one shared tail.
SHARED = [(0x0, 'push   %ebp'), (0x1, 'mov    %esp,%ebp'), (0x3, 'test   %eax,%eax'),
          (0x5, 'je     10 <_f+0x10>'), (0x7, 'movl   $0x1,0x8(%edx)'), (0xe, 'leave'), (0xf, 'ret'),
          (0x10, 'movl   $0x0,0x8(%edx)'), (0x17, 'jmp    e <_f+0xe>')]


def run(orig, cand):
    a = classify(rows(orig, True), rows(cand, False), 'f')
    return a, summarize(a)


class DiagTests(unittest.TestCase):
    def test_identical_is_exact(self):
        a, s = run(COPIED, COPIED)
        self.assertEqual(a['issues'], [])
        self.assertEqual(s['bucket'], 0)

    def test_epilogue_copy_is_layout_not_content(self):
        a, s = run(COPIED, SHARED)
        self.assertEqual(s['tails'], (2, 1))
        self.assertEqual(s['paired'], s['blocks'][0])
        self.assertEqual(set(s['kinds']), {'layout(epilogue-dup)'})
        self.assertEqual(s['bucket'], 3)

    def test_epilogue_copy_classified_in_both_directions(self):
        a, s = run(SHARED, COPIED)
        self.assertEqual(set(s['kinds']), {'layout(epilogue-dup)'})
        self.assertEqual(s['bucket'], 3)

    def test_changed_constant_is_content(self):
        other = [(o, a.replace('$0x0,', '$0x2,')) for o, a in COPIED]
        a, s = run(COPIED, other)
        self.assertEqual(set(s['kinds']), {'operands'})
        self.assertEqual(s['bucket'], 1)

    def test_inverse_branch_polarity_is_not_exact(self):
        orig = [(0, 'test %eax,%eax'), (2, 'je 8 <_f+0x8>'),
                (4, 'movl $0x1,0x8(%edx)'), (7, 'ret'),
                (8, 'movl $0x0,0x8(%edx)'), (11, 'ret')]
        cand = [(o, a.replace('je 8', 'jne 8')) for o, a in orig]
        a, s = run(orig, cand)
        self.assertEqual(s['bucket'], 1)
        self.assertEqual(set(s['kinds']), {'true'})
        self.assertTrue(a['issues'])

    def test_unpaired_candidate_block_is_not_exact(self):
        orig = [(0, 'test %eax,%eax'), (2, 'je 8 <_f+0x8>'),
                (4, 'movl $0x1,0x8(%edx)'), (7, 'ret'),
                (8, 'movl $0x0,0x8(%edx)'), (11, 'ret')]
        cand = orig + [(12, 'movl $0x2,0xc(%edx)'), (15, 'jmp 0 <_f>')]
        a, s = run(orig, cand)
        self.assertEqual(len(a['cfree']), 1)
        self.assertEqual(a['issues'], [])
        self.assertEqual(s['bucket'], 4)

    def test_cache_fingerprint_tracks_same_path_compiler_change(self):
        with tempfile.TemporaryDirectory(dir=diag.BUILD) as folder:
            compiler = Path(folder) / 'cc1.exe'
            compiler.write_bytes(b'compiler-v1')
            first = diag._file_snapshot([compiler])
            compiler.write_bytes(b'compiler-v2')
            second = diag._file_snapshot([compiler])
        self.assertNotEqual(first, second)

    def test_compile_cache_tracks_header_changes(self):
        if not (diag.TC / 'bin/gcc.exe').is_file():
            self.skipTest('locked compiler is unavailable')
        with tempfile.TemporaryDirectory(dir=diag.BUILD) as folder:
            root = Path(folder)
            include = root / 'include'
            include.mkdir()
            header = include / 'cache_probe.h'
            header.write_text('#define CACHE_PROBE_VALUE 1\n', encoding='ascii')
            old_diag = diag.DIAG
            diag.DIAG = root / 'diag-cache'
            config = {'source': 'cache_probe.c', 'default': '-O0', 'flags': [],
                      'includes': [str(include)]}
            text = '#include "cache_probe.h"\nint cache_probe(void) { return CACHE_PROBE_VALUE; }\n'
            try:
                first, _ = diag.compile_tu(config, text)
                first_hash = hashlib.sha256(first.read_bytes()).hexdigest()
                header.write_text('#define CACHE_PROBE_VALUE 2\n', encoding='ascii')
                second, _ = diag.compile_tu(config, text)
                second_hash = hashlib.sha256(second.read_bytes()).hexdigest()
                again, _ = diag.compile_tu(config, text)
            finally:
                diag.DIAG = old_diag
        self.assertNotEqual(first.parent, second.parent)
        self.assertNotEqual(first_hash, second_hash)
        self.assertEqual(again, second)

    def test_register_rename_is_allocation(self):
        other = [(o, a.replace('%edx', '%ecx')) for o, a in COPIED]
        a, s = run(COPIED, other)
        self.assertEqual(set(s['kinds']), {'registers'})
        self.assertEqual(s['bucket'], 2)

    def test_relocated_displacement_is_masked(self):
        orig = rows([(0x0, 'mov    0x4facc8(%esi),%eax'), (0x6, 'movl   $0x5,0x4facd0(%esi)'), (0x10, 'ret')], True)
        cand = rows([(0x0, 'mov    0x0(%esi),%eax'), (0x6, 'movl   $0x5,0x8(%esi)'), (0x10, 'ret')], False)
        cand[0].update(reloc=['_frames'], rpos=[2], len=6); cand[1].update(reloc=['_frames'], rpos=[2], len=10)
        s = summarize(classify(orig, cand, 'f'))
        self.assertEqual(s['bucket'], 0)

    def test_relocated_immediate_is_masked(self):
        orig = rows([(0x0, 'movl   $0x4d4b83,0x18(%esp)'), (0x8, 'ret')], True)
        cand = rows([(0x0, 'movl   $0x63,0x18(%esp)'), (0x8, 'ret')], False)
        cand[0].update(reloc=['.rdata'], rpos=[4], len=8)
        self.assertEqual(summarize(classify(orig, cand, 'f'))['bucket'], 0)

    def test_register_swap_is_not_cfg(self):
        # two abs() fix-up blocks; the candidate swaps which variable lives in %edi vs %esi
        orig = [(0x0, 'push   %ebp'), (0x1, 'test   %edi,%edi'), (0x3, 'js     20 <_f+0x20>'),
                (0x5, 'test   %esi,%esi'), (0x7, 'js     28 <_f+0x28>'), (0x9, 'mov    %edi,0x8(%edx)'),
                (0xc, 'mov    %esi,0xc(%edx)'), (0xf, 'ret'),
                (0x20, 'neg    %edi'), (0x22, 'jmp    5 <_f+0x5>'), (0x28, 'neg    %esi'), (0x2a, 'jmp    9 <_f+0x9>')]
        cand = [(o, a.replace('%edi', '%TMP').replace('%esi', '%edi').replace('%TMP', '%esi')) for o, a in orig]
        s = summarize(classify(rows(orig, True), rows(cand, False), 'f'))
        self.assertEqual(set(s['kinds']), {'registers'})

    def test_spilled_variable_swap_is_allocation(self):
        # original keeps x in %edi and spills y; the candidate spills x instead (shape of collision_old)
        orig = [(0x0, 'push   %ebp'), (0x1, 'mov    %ecx,%edi'), (0x3, 'sub    %eax,%edi'), (0x5, 'js     20 <_f+0x20>'),
                (0x7, 'fldl   0x8(%edx)'), (0xa, 'mov    %ebx,%esi'), (0xc, 'sub    %edx,%esi'),
                (0xe, 'mov    %esi,-0x2c(%ebp)'), (0x11, 'js     28 <_f+0x28>'), (0x13, 'ret'),
                (0x20, 'neg    %edi'), (0x22, 'jmp    7 <_f+0x7>'),
                (0x28, 'neg    %esi'), (0x2a, 'mov    %esi,-0x2c(%ebp)'), (0x2d, 'jmp    13 <_f+0x13>')]
        cand = [(0x0, 'push   %ebp'), (0x1, 'mov    %ecx,%esi'), (0x3, 'sub    %eax,%esi'),
                (0x5, 'mov    %esi,-0x2c(%ebp)'), (0x8, 'js     20 <_f+0x20>'),
                (0xa, 'fldl   0x8(%edx)'), (0xd, 'mov    %ebx,%edi'), (0xf, 'sub    %edx,%edi'),
                (0x11, 'js     28 <_f+0x28>'), (0x13, 'ret'),
                (0x20, 'neg    %esi'), (0x22, 'mov    %esi,-0x2c(%ebp)'), (0x25, 'jmp    a <_f+0xa>'),
                (0x28, 'neg    %edi'), (0x2a, 'jmp    13 <_f+0x13>')]
        s = summarize(classify(rows(orig, True), rows(cand, False), 'f'))
        self.assertEqual(s['bucket'], 2)
        self.assertTrue(set(s['kinds']) <= {'spills', 'registers', 'cfg-edges(alloc)', 'stack-slots'}, s['kinds'])

    def test_deterministic(self):
        first = [(k, x[0].id, n) for k, x, _, n in run(COPIED, SHARED)[1]['earliest']]
        for _ in range(3):
            self.assertEqual([(k, x[0].id, n) for k, x, _, n in run(COPIED, SHARED)[1]['earliest']], first)


if __name__ == '__main__':
    unittest.main()
