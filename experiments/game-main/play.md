# play: residual mechanism notes

Accepted state remains DIFFER. Preferred unpromoted joint checkpoint is `candidates/continue16-context/checkpoint.json` (play effective `1304c73109b13c4f`). Normalized block equality is diagnostic only.

## Continuations 18-19: distinct ordering mechanisms

Changing only the draw_frame timer structure perturbs unchanged play. The controlled pair remains consistent under identifier renaming and parallel-PHI permutation through the final pre-elimination tree dump. Conversion out of SSA then emits seven constant-copy pairs in different order. This explains initialization-order drift; it does not establish a historical compiler-state fingerprint. Full comparison, falsifiers and scope: `build/unlock18/UNBLOCK_PLAN.md`.

The seven-local stack rotation has a separately measured cause in the candidate: all seven locals start as pseudo-registers, and IRA/reload sorts spill sets by allocation frequency. Both frame contexts put next_speed in slot18 (cost9), timeTimeStart/qpc_start/clockTimeStart/endTime in slots19-22 (cost8), next_floor in slot23 (cost7), and shake in slot24 (cost7). The target places shake before next_speed. Original allocation frequencies are unknown.

A diagnostic-only stock-compiler branch bias on the update guard raises shake's cost and moves its spill slot; a bias on the drawing guard leaves it unchanged. Neither is an admissible source recovery. Do not introduce builtins, attributes or artificial compiler-state padding based on this diagnostic.

Round85's separate historical skipDrawing assignments change costs but worsen block correspondence. Round86's inverted/conditional/switch update guards either reproduce the parent or regress. No new checkpoint or exact function was selected. These families are recorded in play.jsonl; generated evidence and validation receipts are in `build/continue19/RESULTS.md`.

Further source hypotheses should distinguish frequency/coalescing changes from out-of-SSA copy order, use relocation-aware branch correspondence, and preserve every exact peer. Initializer placement alone has already failed to fix the spill rotation.

## Continuation 20: corrected branch anchors

Supersedes the branch labels in continuation17-19 scratch reports: original play+0x260f reads **debug**, not recording (COFF address0x4dd160; recording is0x4f8e28). The name-entry close check at+0x2f59 already corresponds to candidate+0x2f8a: both branch to the immediate zero return at+0xb20 and fall through to update_frame. The actual inverted close check is before qualification, original+0x2460 versus candidate+0x23e8. Repeated-global tests must be paired by successors and neighboring operations, not global identity alone.

Round87's duplicated cleanup/early-cleanup variants retain peers but leave extra blocks and reduce play to518 normalized-exact blocks; reject. Frame round91's reset-before-airborne-refinement variants lose frame correspondence and exact peers; reject. The preferred joint checkpoint is unchanged. Details and anchored disassembly evidence: `build/continue20/RESULTS.md` and `anchored-branches.json`. Whole-TU STC replay matches all188 recorded events; this is a diagnostic validation, not a historical-layout claim.
