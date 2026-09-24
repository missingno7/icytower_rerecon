import copy, struct, unittest
from control_transfers import relative, resolve, tail_layout

def instruction(address, raw, asm='jmp target'):
    return {'address': address, 'bytes': raw.hex(), 'mnemonic': asm.split()[0], 'assembly': asm}

def fixture():
    old = [instruction(4096, b'\x90', 'nop'), instruction(4097, b'\xe9' + struct.pack('<i', 1792 - 4102))]
    new = [instruction(256, b'\x90', 'nop'), instruction(257, b'\xeb' + struct.pack('<b', 144 - 259))]
    candidates = [{'name': 'callee', 'low_pc': 144}, {'name': 'f', 'low_pc': 256}]
    originals = [{'name': 'callee', 'va': 1792}, {'name': 'f', 'va': 4096}]
    row = {'name': 'f', 'va': 4096, 'candidate_offset': 256, 'candidate_size': 3, 'original_size': 6, 'status': 'DIFFER', 'instructions': new, 'relocations': [], 'direct_transfers': [], 'instruction_boundaries_verified': True, 'source_body_sha256': 'body'}
    return (row, old, candidates, originals)

class TransferTests(unittest.TestCase):

    def test_all_supported_relative_widths(self):
        for (raw, width, kind) in [(b'\xe8\x00\x00\x00\x00', 4, 'call'), (b'\xe9\x00\x00\x00\x00', 4, 'jmp'), (b'\xeb\x00', 1, 'jmp'), (b'u\x00', 1, 'conditional'), (b'\x0f\x85\x00\x00\x00\x00', 4, 'conditional'), (b'\xe3\x00', 1, 'conditional')]:
            d = relative(instruction(100, raw))
            self.assertEqual(d['operand_size'], width)
            self.assertEqual(d['kind'], kind)
            self.assertEqual(d['target'], 100 + len(raw))

    def test_short_transfer_byte_equality_is_not_target_proof(self):
        code = bytearray(b'\xeb\x00')
        reference = bytes(code)
        rows = [instruction(16, bytes(code))]
        transfers = resolve(code, reference, 16, 18, 4096, rows, [{'name': 'wrong', 'low_pc': 18}], [{'name': 'wrong', 'va': 4101}], [])
        self.assertFalse(transfers[0]['equal'])
        self.assertEqual(transfers[0]['resolved_value'], 3)
        self.assertNotEqual(bytes(code), reference)
        code = bytearray(reference)
        transfers = resolve(code, reference, 16, 18, 4096, rows, [{'name': 'wrong', 'low_pc': 18}], [{'name': 'wrong', 'va': 8192}], [])
        self.assertEqual(bytes(code), reference)
        self.assertFalse(all((t['equal'] for t in transfers)))

    def test_short_exact_target_and_ambiguous_or_unknown_entries(self):
        rows = [instruction(16, b'\xeb\x00')]
        target = {'name': 'g', 'low_pc': 18}

        def check(candidates, originals):
            return resolve(bytearray(b'\xeb\x00'), b'\xeb\x00', 16, 18, 4096, rows, candidates, originals, [])[0]
        self.assertTrue(check([target], [{'name': 'g', 'va': 4098}])['equal'])
        self.assertFalse(check([target, dict(target, name='alias')], [{'name': 'g', 'va': 4098}])['equal'])
        self.assertFalse(check([], [])['equal'])

    def test_internal_edges_and_real_coff_relocations_are_not_reinterpreted(self):
        self.assertEqual(resolve(bytearray(b'\xeb\x00\xc3'), b'\xeb\x00\xc3', 0, 3, 4096, [instruction(0, b'\xeb\x00')], [], [], []), [])
        self.assertEqual(resolve(bytearray(b'\xe8\x00\x00\x00\x00'), b'\xe8\x00\x00\x00\x00', 0, 5, 4096, [instruction(0, b'\xe8\x00\x00\x00\x00')], [], [], [{'function_offset': 1}]), [])

    def test_unknown_encoding_and_extent_crossing_are_not_silent(self):
        row = instruction(0, b'f\xe9\x00\x00', 'data16 jmp 4')
        result = resolve(bytearray(b'f\xe9\x00\x00'), b'f\xe9\x00\x00', 0, 4, 4096, [row], [], [], [])
        self.assertFalse(result[0]['equal'])
        self.assertEqual(result[0]['operand_size'], 0)
        code = bytearray(b'\xeb')
        result = resolve(code, b'\xeb', 0, 1, 4096, [instruction(0, b'\xeb\x00')], [], [], [])
        self.assertFalse(result[0]['equal'])
        self.assertEqual(len(code), 1)

    def test_wrong_prefix_target_call_or_reference_cannot_claim_layout(self):
        for mutation in ('prefix', 'target', 'call', 'reference', 'prefix-relocation', 'ambiguous', 'boundaries'):
            (row, old, candidates, originals) = fixture()
            reference = None
            if mutation == 'prefix':
                row['instructions'][0]['bytes'] = 'cc'
            elif mutation == 'target':
                row['instructions'][-1]['bytes'] = 'eb8e'
            elif mutation == 'call':
                old[-1]['bytes'] = 'e8' + old[-1]['bytes'][2:]
            elif mutation == 'reference':
                reference = b'bad'
            elif mutation == 'prefix-relocation':
                row['relocations'] = [{'function_offset': 0, 'equal': False, 'resolved_value': None}]
            elif mutation == 'ambiguous':
                candidates.append(dict(candidates[0], name='alias'))
            else:
                row['instruction_boundaries_verified'] = False
            self.assertIsNone(tail_layout(row, old, candidates, originals, reference=reference), mutation)

    def test_long_encoding_that_could_have_been_short_is_not_a_layout_proof(self):
        (row, old, candidates, originals) = fixture()
        originals[0]['va'] = 4080
        old[-1]['bytes'] = (b'\xe9' + struct.pack('<i', 4080 - 4102)).hex()
        self.assertIsNone(tail_layout(row, old, candidates, originals))
        old = [instruction(4096 + i, b'\x90', 'nop') for i in range(125)] + [instruction(4221, b'\xe9' + struct.pack('<i', -130))]
        row.update(instructions=[instruction(256 + i, b'\x90', 'nop') for i in range(125)] + [instruction(381, b'\xeb\x81')], candidate_size=127, original_size=130)
        self.assertIsNone(tail_layout(row, old, [{'name': 'f', 'low_pc': 256}], [{'name': 'f', 'va': 4096}]))

    def test_reverse_short_to_near_relaxation_has_the_same_strict_contract(self):
        old = [instruction(4096, b'\x90', 'nop'), instruction(4097, b'\xeb' + struct.pack('<b', 4080 - 4099))]
        new = [instruction(256, b'\x90', 'nop'), instruction(257, b'\xe9' + struct.pack('<i', 1024 - 262))]
        row = {'va': 4096, 'candidate_offset': 256, 'candidate_size': 6, 'original_size': 3, 'instructions': new, 'instruction_boundaries_verified': True}
        proof = tail_layout(row, old, [{'name': 'g', 'low_pc': 1024}], [{'name': 'g', 'va': 4080}])
        self.assertIsNotNone(proof)
        self.assertEqual(proof['original_encoding_bytes'], 2)
        self.assertEqual(proof['candidate_encoding_bytes'], 5)
