# calc_replay_checksum_131 recovery, 2026-09-25

Accepted: 177-byte FUNCTION_MATCH. The final game total for this pass is 216
FUNCTION_MATCH, 36 DIFFER, one CODEGEN_SIMILAR. The replay TU has eight exact
functions, six DIFFER and one CODEGEN_SIMILAR. recovery.json is authoritative.

Before recovery, all instructions matched except the order of two independent
loads at offsets 144-150 in the replay-data loop. The original loaded
`cycle_count` before `key_flags`. Expressing the checksum as separate indexed
terms, `i * cycle_count * 3 + i * key_flags * 5`, makes the historical compiler
emit the original load order and every byte. Reversing only the additive terms
or multiplying the combined sum by `i` did not. Search record:
`calc_replay_checksum_131.jsonl`, round 1, `index_inner`.
Accepted body (relative to experiments/):
`bodies/04d64959199be5056fef29fcf836ff20cef572641bc0cb2c1552caaa69fb9120.c`.

Promotion passed exact peer/body protection, data/BSS, independent clean-build
reproducibility, ordinary link of all 25 units, and the 45-test suite. Fresh
replay verification confirms eight exact functions. No game was executed and
no whole-object/CU/executable equality is claimed. Accepted `src/replay.c`
SHA256: `ef08cac2b6cc853f139f94ec85a17c2473c1f1d13ea502e566910607b805895c`.

The subsequent `my_strcmp` search found that the original different-directory
branch returns -1 specifically for byte value 1. Nested property conditions and
inverted result branches improved block similarity but stayed DIFFER; see its
four recorded rounds. No unmatched comparator variant was published.
