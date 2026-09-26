"""Self-tests for the experimental diagnostics (tools/diag.py), on synthetic rows.

Kept outside tests/ on purpose: diag.py is search tooling, not part of the proof context.
  python tools/diag_test.py
"""
import unittest
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

    def test_deterministic(self):
        first = [(k, x[0].id, n) for k, x, _, n in run(COPIED, SHARED)[1]['earliest']]
        for _ in range(3):
            self.assertEqual([(k, x[0].id, n) for k, x, _, n in run(COPIED, SHARED)[1]['earliest']], first)


if __name__ == '__main__':
    unittest.main()
