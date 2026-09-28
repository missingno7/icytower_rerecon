# portable-sdl3: provenance and Phase 0 baseline

`portable-sdl3` is a modern source port derived from the frozen matching
reconstruction. It is not a matching build and makes no matching claims.

| item | value |
|---|---|
| branch point (frozen `main`) | `70ece2aaf6ff79f52165e38fa3c346160a7e298d` "Oracle freeze: equivalence tier applied; play v21 and draw_frame N2 published as EQUIVALENT" |
| frozen claim | 253 game functions in 25 units: 251 `FUNCTION_MATCH`, `play` and `draw_frame` `EQUIVALENT` (see `FREEZE.md`) |
| recorded on | 2026-09-28 |

## Phase 0 checks on the frozen tree (before any divergence)

Run on the branch point, with the historical toolchain in `build/local/`:

| check | result |
|---|---|
| `python tools/test.py` | `Ran 57 tests in 49.3s` - `OK` |
| `python tools/audit.py` | `function_counts: FUNCTION_MATCH 251, EQUIVALENT 2`, 25 historical TUs, one recovery authority (`recovery.json`) |
| `python tools/link.py` | ordinary link closed; the linked executable is kept as `build/oracle-ref/icytower-ref.exe` (sha256 `2b2ca932a98d0c03e6b8b24856d50f1d0390b35525bd7ea28d7c0a6e8bf9d8e8`) |
| `icytower-ref.exe -check MissingNO_best_score_5059.itr` | headless replay check prints claimed = actual results (score 5059, floor 204, combo 32) |

## Three layers

```
historical frozen reconstruction   (main @ 70ece2a: src/, recovery.json, FREEZE.md, tools/)
        |
        v
behavioural reference oracle       (build/oracle-ref/icytower-ref.exe, linked from the frozen
                                    sources with the historical GCC 4.4 + Allegro 4.4.1 toolchain;
                                    its headless "-check REPLAY -all" mode is the replay oracle)
        |
        v
portable SDL3 implementation       (this branch: CMakeLists.txt, compat/, port/, adapted src/)
```

`FREEZE.md`, `recovery.json` and the matching tools stay as historical provenance. They describe
the frozen commit, not this branch: once the game sources here diverge, `tools/verify.py`,
`tools/test.py` and `recovery.json` no longer describe them and are not updated to do so.
Behaviour on this branch is checked against the reference oracle instead (see
`docs/port/TESTING.md`).

The reference executable is rebuilt from the frozen tree with

```
git worktree add build/frozen-tree 70ece2aaf6ff79f52165e38fa3c346160a7e298d
cd build/frozen-tree && python tools/link.py      # needs build/local (bootstrap.py)
```

or, while `src/` on this branch still equals the frozen sources, `python tools/link.py`.
