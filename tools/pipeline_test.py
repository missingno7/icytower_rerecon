"""Self-tests for tools/pipeline.py (compiler-pipeline diagnostics) on fixed dump text.

Kept outside tests/ on purpose, like diag_test.py: pipeline.py is search tooling, not part of
the proof context.
  python tools/pipeline_test.py
"""
import re, unittest
import pipeline as P

# bb-reorder STC events of GCC 4.4.1 for handle_player_collision_vector_2 (late-thread body,
# -fdump-rtl-bbro-details); the recorded "Getting bb" order is the ground truth for the heap.
STC_LOG = """STC - round 1
Getting bb 2
Basic block 2 was visited in trace 0
  Possible start of next round: 4 (key: -1371)
Basic block 5 was visited in trace 0
Basic block 7 was visited in trace 0
  Possible start of this round: 11 (key: -8550)
Basic block 9 was visited in trace 0
  Possible start of next round: 12 (key: -1450)
  Possible start of next round: 10 (key: -3550)
Changing key for bb 12 from -1450 to -1146450.
Changing key for bb 10 from -3550 to -1358550.
Getting bb 11
Basic block 11 was visited in trace 1
Basic block 13 was visited in trace 1
  Possible start of this round: 17 (key: -5685)
Basic block 14 was visited in trace 1
  Possible start of next round: 16 (key: -685)
  Possible start of next round: 15 (key: -4315)
Changing key for bb 16 from -685 to -1069285.
Changing key for bb 15 from -4315 to -1435815.
Getting bb 17
Basic block 17 was visited in trace 2
  Possible start of this round: 18 (key: -5000)
Basic block 19 was visited in trace 2
  Possible start of next round: 22 (key: -3900)
  Possible start of next round: 21 (key: -3600)
Changing key for bb 22 from -3900 to -1143900.
Changing key for bb 21 from -3600 to -1363600.
Getting bb 18
Basic block 18 was visited in trace 3
Basic block 23 was visited in trace 3
  Possible start of next round: 25 (key: -2800)
Basic block 24 was visited in trace 3
Basic block 26 was visited in trace 3
  Possible start of this round: 32 (key: -10000)
Basic block 27 was visited in trace 3
  Possible start of next round: 28 (key: -1166)
Changing key for bb 28 from -1166 to -1117766.
Changing key for bb 32 from -10000 to -1503400.
Getting bb 32
Basic block 32 was visited in trace 4
STC - round 2
Getting bb 15
Basic block 15 was visited in trace 5
Getting bb 21
Basic block 21 was visited in trace 6
Basic block 22 was visited in trace 6
Getting bb 10
Basic block 10 was visited in trace 7
Getting bb 12
  Possible start point of next round: 12 (key: -1146450)
Getting bb 28
  Possible start point of next round: 28 (key: -1117766)
Getting bb 16
  Possible start point of next round: 16 (key: -1069285)
Getting bb 25
Basic block 25 was visited in trace 8
Getting bb 4
  Possible start point of next round: 4 (key: -1371)
STC - round 3
Getting bb 12
Basic block 12 was visited in trace 9
Getting bb 28
Basic block 28 was visited in trace 10
  Possible start of this round: 30 (key: -583)
Basic block 29 was visited in trace 10
Basic block 31 was visited in trace 10
Getting bb 16
Basic block 16 was visited in trace 11
Getting bb 4
Basic block 4 was visited in trace 12
  Possible start of next round: 6 (key: -384)
Changing key for bb 6 from -384 to -1038784.
Getting bb 30
Basic block 30 was visited in trace 13
STC - round 4
Getting bb 6
Basic block 6 was visited in trace 14""".splitlines()

PROFILE = """;; Function f (f)

Predictions for bb 7
  DS theory heuristics: 19.9%
  first match heuristics (ignored): 28.0%
  combined heuristics: 19.9%
  opcode values nonequal (on trees) heuristics: 28.0%
  early return (on trees) heuristics: 39.0%
Predictions for bb 9
  DS theory heuristics (ignored): 3.2%
  first match heuristics: 2.0%
  combined heuristics: 2.0%
  loop exit heuristics: 2.0%
"""

BLOCKS = """f (int x)
{
  # BLOCK 7
  # PRED: 5 (false,exec)
  [f.c : 12] D.1_39 = [f.c : 12] p_3->status;
  [f.c : 12] if (D.1_39 == 1)
    goto <bb 20>;
  else
    goto <bb 8>;
  # SUCC: 20 (true,exec) 8 (false,exec)

  # BLOCK 8
  # PRED: 7 (false,exec)
  [f.c : 13] play_sound (s_1, 1, 1);
  # SUCC: 20 [100.0%]  (fallthru,exec)

  # BLOCK 20
  # PRED: 7 (true,exec) 8 [100.0%]  (fallthru,exec)
  [f.c : 20] return;
  # SUCC: EXIT [100.0%]

}
"""


class PipelineTests(unittest.TestCase):
    def test_dempster_shafer_matches_gcc(self):
        self.assertAlmostEqual(P.ds_combine([0.28, 0.39]), 0.199, places=3)   # nonequal + early return
        self.assertAlmostEqual(P.ds_combine([0.72, 0.39]), 0.622, places=3)
        self.assertAlmostEqual(P.ds_combine([0.28, 0.29]), 0.137, places=3)   # nonequal + call
        self.assertEqual(P.ds_combine([]), 0.5)

    def test_parse_predictions(self):
        p = P.parse_predictions(PROFILE)
        self.assertEqual(sorted(p), [7, 9])
        names = {n: (ig, v) for n, ig, v in p[7]}
        self.assertEqual(names['combined'], (False, 19.9))
        self.assertEqual(names['early return (on trees)'], (False, 39.0))
        self.assertTrue(any(n == 'first match' and not ig for n, ig, _ in p[9]))

    def test_parse_blocks_edges_calls_and_returns(self):
        b = P.parse_blocks(BLOCKS)
        self.assertEqual((b[7]['then'], b[7]['else']), (20, 8))
        self.assertEqual(b[7]['cond'], 'if (D.1_39 == 1)')
        self.assertEqual(b[7]['line'], 12)
        self.assertEqual(b[8]['calls'], ['play_sound'])
        self.assertTrue(b[20]['returns'])
        d = P.definitions(b)
        self.assertEqual(P.describe_cond(b[7], d), 'if (D.1_39 == 1)   [D.1_39 = p_3->status]')

    def test_fingerprint(self):
        ev = [{'pass': 'dom1', 'cond': 'if (D_81 == 0)   [D_81 = p->status]'},
              {'pass': 'vrp2', 'cond': 'if (D_73 == 0)   [D_73 = right_2 + left_1]'}]
        P.classify_events(ev, [('SUM', r'left_\d+ \+ right|right_\d+ \+ left'), ('EDGE', r'left_\d+ != right')])
        self.assertEqual([e['class'] for e in ev], ['-', 'SUM'])
        self.assertTrue(P.fingerprint_ok(ev, {'SUM': {'vrp2', 'dom2'}}, {'EDGE'}))
        self.assertFalse(P.fingerprint_ok(ev, {'SUM': {'vrp1'}}, set()))
        ev.append({'pass': 'dom1', 'cond': 'if (left_21 != right_22)', 'class': 'EDGE'})
        self.assertFalse(P.fingerprint_ok(ev, {'SUM': None}, {'EDGE'}))

    def test_heap_replay_reproduces_gcc(self):
        recorded = [int(m[1]) for l in STC_LOG for m in [re.match(r'Getting bb (\d+)', l)] if m]
        self.assertEqual([b for _, b in P.stc_replay(STC_LOG)], recorded)

    def test_heap_key_override_changes_tie_order(self):
        base = [b for r, b in P.stc_replay(STC_LOG) if r == 3]
        self.assertEqual(base[:2], [12, 28])
        moved = [b for r, b in P.stc_replay(STC_LOG, {28: -1200000}) if r == 3]
        self.assertEqual(moved[:2], [28, 12])

    def test_fibheap_orders_by_key(self):
        h = P.FibHeap()
        for k, d in [(5, 'a'), (-3, 'b'), (2, 'c'), (-3, 'd'), (0, 'e')]: h.insert(k, d)
        out = [h.extract() for _ in range(5)]
        self.assertEqual(set(out[:2]), {'b', 'd'})
        self.assertEqual(out[2:], ['e', 'c', 'a'])

    def test_conditions_are_balanced(self):
        src = 'if (a && (b || c)) x(); if (f(g(1))) y();'
        self.assertEqual([src[a:b] for a, b in P.conditions(src)], ['a && (b || c)', 'f(g(1))'])


if __name__ == '__main__':
    unittest.main()
