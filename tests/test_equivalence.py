"""Equivalence tier: records are carried only while unchanged and granted only through a bound certificate."""
import sys, unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from verify import carry_equivalence
from promote import check_certificate

class EquivalenceTierTests(unittest.TestCase):
    def units(self, status, sha):
        return {'t': {'functions': {'f': {'status': status, 'body_sha256': sha}}, 'whole_text_equal': False, 'object_match': False, 'cu_match': False}}

    def test_record_carried_upgraded_or_refused(self):
        prior = {'t': {'functions': {'f': {'status': 'EQUIVALENT', 'body_sha256': 'b', 'equivalence': {'divergences': 0}}}}}
        kept = carry_equivalence(prior, self.units('DIFFER', 'b'))
        self.assertEqual(kept['t']['functions']['f']['status'], 'EQUIVALENT')
        self.assertEqual(kept['t']['functions']['f']['equivalence'], {'divergences': 0})
        up = carry_equivalence(prior, self.units('FUNCTION_MATCH', 'c'))
        self.assertEqual(up['t']['functions']['f']['status'], 'FUNCTION_MATCH')
        with self.assertRaises(ValueError): carry_equivalence(prior, self.units('DIFFER', 'changed'))
        with self.assertRaises(ValueError): carry_equivalence(prior, self.units('MISSING', 'b'))
        plain = carry_equivalence({'t': {'functions': {'f': {'status': 'DIFFER', 'body_sha256': 'b'}}}}, self.units('DIFFER', 'b'))
        self.assertEqual(plain['t']['functions']['f']['status'], 'DIFFER')
        self.assertEqual(carry_equivalence(None, self.units('DIFFER', 'b'))['t']['functions']['f']['status'], 'DIFFER')

    def cert(self):
        return {'function': 'f', 'target': 't', 'candidate_body_sha256': 'b', 'method': 'lockstep simulation modulo ...', '_path': 'evidence/equivalence/f.json',
                'simulation': {'states': 1, 'pairs': 1, 'divergences': 0, 'unverified_skipped': 0},
                'line_table': {'identical_lines': 1, 'differing_lines': 0}, 'exact_peers_changed': [], 'note': 'n'}

    def test_certificate_must_bind_and_protect(self):
        row = {'body_sha256': 'b'}
        rec = check_certificate(self.cert(), 'f', 't', row, {'peer'})
        self.assertEqual(rec['status'], 'EQUIVALENT'); self.assertEqual(rec['equivalence']['divergences'], 0)
        for field, value in [('function', 'g'), ('target', 'u'), ('candidate_body_sha256', 'x'), ('method', 'eyeballed'), ('exact_peers_changed', ['peer'])]:
            c = self.cert(); c[field] = value
            with self.subTest(field=field), self.assertRaises(ValueError): check_certificate(c, 'f', 't', row, {'peer'})
        c = self.cert(); c['exact_peers_changed'] = ['not_exact']
        self.assertEqual(check_certificate(c, 'f', 't', row, {'peer'})['status'], 'EQUIVALENT')
        c = self.cert(); del c['simulation']['divergences']
        with self.assertRaises(ValueError): check_certificate(c, 'f', 't', row, {'peer'})

if __name__ == '__main__':
    unittest.main()
