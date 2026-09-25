# `calc_replay_checksum` scratch search, 2026-09-25

No candidate has been promoted. The canonical source and `recovery.json` were not changed by this search. The accepted function currently compiles to a 675-byte body with 631 differing bytes against the 676-byte oracle body.

## Corrected arithmetic interpretation

The oracle computes `17 * biggest_lost_combo + 17` in `ecx`, then adds `127 * no_combo_top_floor`. Later, at `0x41bb3d`, `lea edx,[eax+ecx*2]` combines twice that subtotal with the floor/speed/gravity subtotal. This supports the reconstructed outer factor of two; it is applied later in the instruction stream rather than by scaling each initial coefficient. The earlier 213-difference `floor_pair_from_best.c` omits that factor and is mathematically wrong for the oracle. Do not treat it as a viable promotion candidate.

## Scratch results

The 213-difference candidate remains useful only as an instruction-order clue: it has the correct function extent, a 290-byte exact suffix, and no unproved relocations or transfers, but it has the wrong checksum formula. Reversing its split initialization statements and spelling the initial `17 * (value + 1)` term did not change its effective output. Combining the terms into one expression did produce the oracle's `0x60`-before-`0x5c` load order, but shifted register allocation through the scalar prelude and regressed to 606 differing bytes.

Correctly doubled spellings while retaining the best floor and final data-loop orderings remain `DIFFER`:

- `gravity_initial_group_from_best.c`: 615 differing bytes.
- `double_initial_from_best.c`: 618 differing bytes.
- `delayed_initial_double_from_best.c`: 633 differing bytes.
- `factor_left_from_best.c`: 643 differing bytes.

No exact peers were lost. There are no new production changes and no tests were run because no candidate is promotable. Detailed compiler responses are in [calc_replay_checksum.jsonl](calc_replay_checksum.jsonl); candidate bodies are under `candidates/calc_replay_checksum/`.
