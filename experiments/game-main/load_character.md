# load_character research, 2026-09-25

The target oracle body is 330 bytes. Earlier body spellings retained 15 differing bytes, mostly because the candidate kept `filename` in `ebx` and `name` in `esi`, while the oracle uses `esi` for `filename` and `ebx` for `name`.

A disassembly comparison identified a concrete liveness difference: after building the local 1024-byte `buf`, the oracle passes that buffer pointer as the second path string to `log2file`; the reconstructed body passed `filename`. The oracle first keeps `filename` in `esi`, then reuses `esi` for `buf`, which stays live through logging. The scratch body [log_buf_path.c](../../candidates/load_character/log_buf_path.c) changes only that argument and reduces the gap from 15 bytes to 4. The function extent is exact; all relocations and direct transfers are proven, and no exact peers regressed.

The remaining four differing bytes are register fields at function offsets 178, 185, 248, and 254. Around the two post-load bitmap checks, the oracle uses `edi` then `ecx`; the candidate uses `ecx` then `edx`. An inverted status ternary deduplicated to the same output. Explicit duplicated log branches and a success-first control-flow rewrite regressed to 156 and 146 differing bytes, respectively.

The 4-byte candidate remains `DIFFER`; do not promote it. No canonical source/state changes or tests. Detailed outcomes are in [load_character.jsonl](load_character.jsonl).
## Follow-up register-allocation checks

Rounds 10 and 12 did not create a new effective output. Explicit `NULL` comparisons, a local bitmap alias, and both shared-log status-pointer assignment orders all deduplicated to the same four-byte mismatch. Round 11's separate loader-return temporary produced a new 315-byte body but regressed to 283 differing bytes.

The oracle line table and disassembly localize the remaining gap to two bitmap field loads: at `0x40ff29` the oracle loads to `edi` and tests it, while the best candidate uses `ecx`; at `0x40ff6f` the oracle loads to `ecx`, while the candidate uses `edx`. The surrounding instructions and branches match. The register spellings tried so far do not alter allocation, and none of these variants is promotable. Further work should test a different live-range or control-flow family rather than another Boolean spelling.
