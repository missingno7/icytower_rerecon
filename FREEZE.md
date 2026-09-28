# Frozen reconstruction state (2026-09-28)

This file records what the reconstruction proves, what it only demonstrates, and what is still
different, so that a later correction can start from facts. The oracle (the original
`icytower15.exe`) is verifier-only and is never executed.

## Code: 253 game functions in 25 historical units

| status | count | meaning |
|---|---|---|
| FUNCTION_MATCH | 251 | byte-identical to the original after relocation masking; whole `.text` contribution equal in 24 of 25 units |
| EQUIVALENT (pending publication) | 2 | `play`, `draw_frame`: not byte-exact; behaviour verified by lockstep simulation and by runtime replay |

`recovery.json` still records `play` and `draw_frame` as DIFFER with the canonical bodies. The
faithful bodies and their certificates are:

| function | body | certificate | identical line-table lines | instructions (ours / original) | simulation |
|---|---|---|---|---|---|
| play | `candidates/rootcause37/play_linefaithful_v20.c` | `evidence/equivalence/play.json` | 664 of 710 on the canonical TU (672 with the prototype blocks collapsed) | 3687 / 3685 | 602 states, 4887 pairs, 2 divergences (both a value-id tracking artefact at the doubled guard on line 4814: the same `esi` value under two ids), 0 exact peers changed |
| draw_frame | `candidates/rootcause37/linefaithful_n2.c` | `evidence/equivalence/draw_frame.json` | 137 of 151 | 1926 / 1927 | 184 states, 2819 pairs, 14 divergences at lines 2530 (stripe offset order) and 2594-2636 (idle-pose x87 forms and the p_im/customFrame copies), covered by symbolic x87 replay |

What differs in `play` (all layout, no missing or extra operation): the order of cold traces after
the epilogue, four jump polarities, three alignment nops, the PHI-copy order of two initialisations,
and the inlined helper's line numbers. What differs in `draw_frame`: the x87 comparison form on the
idle path (`fucom`+`fstp` vs `fucompp`+`sahf`), where `p_im = 1` is materialised, register choice in
the inlined sprite bodies, the evaluation order of the stripe offset (line 2530) and the cx/cy
spill-slot order. Sections 6-13 of `build/rootcause37/RESULTS.md` (ignored, regenerable) hold the
full residue analysis; the tracked experiment log is `experiments/game-main/play.jsonl` and
`draw_frame.jsonl`, rounds 147-154.

The canonical `src/main.c` bodies of `play` and `draw_frame` carry real semantic gaps (results-loop
form, scroller update, key cascade, sprite placement) and must not be used for a faithful build.
`build/faithful/icytower-faithful.exe` (tools: `link.ordinary_link` on a scratch tree with the two
faithful bodies) is the verified runnable: it plays normally and original replays do not desync
(user test, 2026-09-28).

## Data

Every initialized `.data`/`.rdata` contribution is content-equal in all 25 units (after the
`gFLDADMutex` initializer promotion, commit 16be896). `.bss` layout is proven where BSS symbols
exist (main, custom, fld_adspot, menu, replay) and is zero-filled elsewhere. A sweep of original
DWARF globals against candidate commons found no other initializer gap.

## Vendor layer

The original was built from Allegro 4.4.1 sources with the same 114 C units we build (no assembly
units; 262 function declaration lines agree), plus logg (18 functions verified) and libogg/libvorbis
(same source versions; original objects were built by GCC 4.2.1). Allegro and Xiph function bodies
have not been compared against the original. Import differences: WS2_32 vs WSOCK32, regenerated
libpng3 import library.

## Runtime

Every rebuilt executable crashed before the main menu until the `gFLDADMutex` initializer was
restored (pthreads-win32 `pthread_mutex_unlock` on a NULL mutex). After that fix the faithful build
plays and replays correctly on the user's machine.

## Equivalence tier (gate extension, awaiting approval)

`tools/equivalence/` holds the tracked simulator (`bisim.py`), the listing/comparison helpers
(`objfun.py`) and the certifier (`certify.py`). The intended gate extension, not yet applied because
it modifies the shared verification tools:

- `verify.py`: `carry_equivalence` keeps an EQUIVALENT record across `--all --refresh` only while
  the accepted body hash is unchanged and the fresh proof is DIFFER; a fresh FUNCTION_MATCH upgrades
  it; anything else is refused. The equivalence tools join the proof context.
- `promote.py --equivalent CERT.json`: publishes a DIFFER body only when the certificate binds to
  the fresh candidate body hash, names the lockstep simulation, and reports no exact peer changed;
  all other protections (exact peers, data/BSS owners, reproducibility, ordinary link, tests) stay.
  The record is EQUIVALENT with the certificate's summary; it never grants FUNCTION_MATCH.
- Publication order: `draw_frame` first, then `play` (draw_frame's declarations shift play's
  DECL_UIDs), then re-certify play on the published TU.

Correcting later: a strict promotion of a byte-exact body for either function replaces the
EQUIVALENT record through the ordinary gate.
