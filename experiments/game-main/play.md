# play: residual mechanism notes

Accepted state remains DIFFER. Preferred unpromoted structural checkpoint is `candidates/continue23-context/checkpoint.json` (play effective `30d882d9ff5b18b7`). Continue22 remains the comparison control with more normalized-exact play blocks. Normalized block equality is diagnostic only.

## Continuations 18-19: distinct ordering mechanisms

Changing only the draw_frame timer structure perturbs unchanged play. The controlled pair remains consistent under identifier renaming and parallel-PHI permutation through the final pre-elimination tree dump. Conversion out of SSA then emits seven constant-copy pairs in different order. This explains initialization-order drift; it does not establish a historical compiler-state fingerprint. Full comparison, falsifiers and scope: `build/unlock18/UNBLOCK_PLAN.md`.

The seven-local stack rotation has a separately measured cause in the candidate: all seven locals start as pseudo-registers, and IRA/reload sorts spill sets by allocation frequency. Both frame contexts put next_speed in slot18 (cost9), timeTimeStart/qpc_start/clockTimeStart/endTime in slots19-22 (cost8), next_floor in slot23 (cost7), and shake in slot24 (cost7). The target places shake before next_speed. Original allocation frequencies are unknown.

A diagnostic-only stock-compiler branch bias on the update guard raises shake's cost and moves its spill slot; a bias on the drawing guard leaves it unchanged. Neither is an admissible source recovery. Do not introduce builtins, attributes or artificial compiler-state padding based on this diagnostic.

Round85's separate historical skipDrawing assignments change costs but worsen block correspondence. Round86's inverted/conditional/switch update guards either reproduce the parent or regress. No new checkpoint or exact function was selected. These families are recorded in play.jsonl; generated evidence and validation receipts are in `build/continue19/RESULTS.md`.

Further source hypotheses should distinguish frequency/coalescing changes from out-of-SSA copy order, use relocation-aware branch correspondence, and preserve every exact peer. Initializer placement alone has already failed to fix the spill rotation.

## Continuation 20: corrected branch anchors

Supersedes the branch labels in continuation17-19 scratch reports: original play+0x260f reads **debug**, not recording (COFF address0x4dd160; recording is0x4f8e28). The name-entry close check at+0x2f59 already corresponds to candidate+0x2f8a: both branch to the immediate zero return at+0xb20 and fall through to update_frame. The actual inverted close check is before qualification, original+0x2460 versus candidate+0x23e8. Repeated-global tests must be paired by successors and neighboring operations, not global identity alone.

Round87's duplicated cleanup/early-cleanup variants retain peers but leave extra blocks and reduce play to518 normalized-exact blocks; reject. Frame round91's reset-before-airborne-refinement variants lose frame correspondence and exact peers; reject. The preferred joint checkpoint is unchanged. Details and anchored disassembly evidence: `build/continue20/RESULTS.md` and `anchored-branches.json`. Whole-TU STC replay matches all188 recorded events; this is a diagnostic validation, not a historical-layout claim.

## Continuation 21: layout-pass divergence localized

Stable instruction lineage shows that qualification-close UID3256 and final-menu-debug UID4760 have the target jump direction before RTL block reordering and reverse inside bbro. Trace IDs differ from pre/post-layout IDs because of compaction; use `build/continue21/layout-lineage.json`, not a numeric-ID guess. The qualification close start ties with cleanup at key-1000204, cleanup is visited first, and the qualification successor is deferred to the next round. The menu path carries a29% call prediction.

Narrow diagnostic-only frequency hints recover the intended directions. Menu-only reaches631 paired/524 normalized-exact blocks; close-only and joint probes regress overall. These probes are not candidates. Actual builtin expectation is100% in this locked compiler's dump. Heap replay validates all188 events; changing a replay key only changes recorded-segment visitation, not verified emitted code.

Round88's explicit three-path profile-save source histories worsen correspondence (two-path form also loses a peer). Round89's early cleanup return on the debug/menu-skip path leaves the29% menu prediction unchanged and extra blocks survive. Reject both families. Preferred checkpoint remains unchanged. Full scope, reproduction inputs and receipts: `build/continue21/RESULTS.md`.

## Continuation 22: admissible menu and allocation witnesses

Ordinary, contextually dead C guards are allowed by README's compiler-steering policy; uncertain historical spelling alone is not a reason to exclude them. A negative return guarded by !debug inside the debug arm changes the final menu estimate from29% to90.7%, then disappears. Original menu JNE/fallthrough is recovered without builtins or compiler changes. Zero-return and opposite-arm controls reproduce the parent. Returns inside the gameplay loop alter loop-exit predictions and regress; reject that scope.

The shake update guard adds a redundant byte/short/full-width conjunction under its existing nonzero disjunction. All additional tests disappear. Predicted retained update frequency29+71/8=37.875% gives measured block frequency76, raising shake's spill cost7to9. The seven locals now occupy shake18, next_speed19, timeTimeStart20, qpc_start21, clockTimeStart22, endTime23, next_floor24. The emitted shake store is the original ebp-0x96c. A two-test control gives frequency94/cost11; explicit final-tick paths first recovered cost9 but retained extra branches.

The selected joint checkpoint reaches630 paired/536 normalized-exact play blocks (previous630/523), reducing stack-slot pairs21to3; frame remains209/171. Play has17443 bytes and13275 raw differences, so this is a structural/allocation gain with layout tradeoffs, not an exact function or universal score improvement. Every exact peer is preserved. Fresh play round98 and frame round94 reproduce effective identities; no promotion.

Qualification still has the wrong branch direction. A modest zero implication improves later layout but does not resolve that guard. Relocation-aware comparison also identifies a reversed operand/relation pair at next_aight+0x995; it is equivalent behavior, not a missing computation. The frame .02-to-.2 x87 value lifetime remains real when the correct incoming path is followed. Full predictions, rejected controls, branch anchors and receipts: `build/continue22/RESULTS.md`. Rounds90-97 contain38 unique source candidates across both residuals; generated evidence stays ignored.

## Continuation 23: qualification direction and joint frame lifetime

Widening the qualification predecessor to `!quit || (quit && closeButtonClicked)`, while retaining the existing inner close guard, admits only an extra path that immediately fails that guard. The added tests disappear from the optimized CFG. The measured close-to-qualification probability changes from 50% to 62.5%; emitted play+0x23e8 now has JNE-to-cleanup and qualification fallthrough, matching original+0x2460. The recovered final-menu JNE direction also remains at+0x2593, followed by do_replay_menu. The initial continuation23 attribution of STC BB435/634 to close/cleanup was unsupported and is retracted: numeric IDs were carried across passes without a stable bridge. The measured profile and relocation-resolved emitted branches establish this result; the precise trace-queue cause remains unresolved.

The selected joint patch is `experiments/patches/d1b1084e9d260ddda666a617b10efdd3c0427de863e8cde1773b6dec4d498f1d.json`. It also preserves raw sx from the idle .02 comparison into the frame .2 reset path. Play has 630 paired/532 normalized-exact blocks, 82 register, 3 stack-slot and 13 near pairs; 17419 bytes versus 17420 target and 13285 raw differences. Continue22 had 536 normalized-exact blocks and 13275 raw differences. Thus this checkpoint resolves two concrete branch/lifetime mechanisms with layout tradeoffs; it is not a monotonic score improvement or an exact function.

Independent DWARF/instruction audit identifies the three remaining stack pairs as spills: live scroll_acc/tot_scroll across sound calls, and BITMAP height/width argument temporaries across new_rand before blit. No missing named-local identity was found. Do not add locals merely to force those spill addresses.

Fresh round 102 searches reproduce both effective identities with no exact peer losses. Verify remains 251 FUNCTION_MATCH / 2 DIFFER; no canonical source/header, proof tool or recovery state changed. The remaining next_aight operand/relation reversal and name-entry pairing need separate analysis; an unpaired label alone does not prove missing or duplicated code. Full results: `build/continue23/ROOT_RESULTS.md`; branch anchors: `selected-branch-anchors.json`; audits: `PLAY_PREDECESSOR_RESULT.md` and `PLAY_STACK_AUDIT.md`.
