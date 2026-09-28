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

## Independent investigation 25: earlier context effects

Fresh normal searches reproduce both checkpoints. Removing15 redundant forward-declaration groups changes no function body, preserves80 exact peers, leaves frame effective code identical, but regresses play to628 paired/455 normalized-exact blocks and60 stack-slot pairs. Restoring source-owned unspecified-parameter declarations also leaves frame identical. Neither is a shared fix; no prelude/header change was promoted.

The deduplication control exposes an earlier context mechanism than the previous out-of-SSA pair. Executable play trees agree under consistent identifier mapping/parallel-PHI permutation until071t.dom1. Checkpoint `vgp_306 < lastMusicPos_943` becomes `lastMusicPos_85 > vgp_306`, following GCC's SSA-version operand ordering rule. The reversal survives at emitted play+0x302: original/checkpoint CMP-stack-to-EAX/JGE becomes CMP-EAX-to-stack/JLE. Thus removing obvious declaration repetition can damage an already matching code region. Later copy/PRE/allocation changes are separate observations, not attributed entirely to this comparison.

The frame-only idle-pose guard independently changes unchanged play at074t.reassoc1: gotHigh addition operands reverse after its incoming PHI changes from version2820 to893. Consequently the earlier seven-copy SSA-elimination result must stay scoped to its measured source pair. The historical UID/PHI state is still unknown. Raw stage comparisons include mutation controls for inconsistent identifiers, constants and edges, and use the final function rendering only.

Relocation-resolved reinspection confirms next_aight's unpaired block is reversed CMP/Jcc with corresponding successors; the name-entry close check is a direct memory comparison in the original versus load/TEST in continue23, with the same resolved global and return/update successors. Neither establishes missing computation. The retained frame coldness witness gives play630 paired/533 normalized-exact blocks,3 stack pairs,17419 bytes/13267 differences, still strict DIFFER. Other continuation controls regress; the common-store variant loses do_replay_menu and is rejected. Both earlier checkpoints remain alternatives.

Full report, nine source controls, stage anchors, storage protection and validation receipts: `build/rootcause25/RESULTS.md`. Fresh verify251/2,55 tests, audit and ordinary source link pass. No canonical source/header, proof tool, configuration or recovery state changed; no promotion or original-executable execution.

## Continuation 26: exact-peer copy ordering isolated

Twelve additional frame/context patches were searched jointly in rounds110-118; every patch leaves the checkpoint23 play body byte-identical. Direct short-circuit reset changes can regress play to628 paired/455 normalized-exact blocks and61 stack pairs. Its first executable divergence is again071t.dom1's vgp/lastMusicPos operand/relation reversal; later allocation changes remain separate observations. Virtual memory dump annotations are excluded by actual VUSE/VDEF identity, with an identical-source control agreeing at all76 stages for both residuals and identifier/constant/edge mutation checks passing.

The status/direct-load frame control loses exact do_replay_menu through two constant-copy ordering swaps, first visible after SSA elimination at123t.optimized. Its executable trees agree through122t.uncprop; final differing instruction spans are offsets71-85 and253-264. Combining the previously audited source-owned unspecified-parameter context restores every exact peer and all76 do_replay_menu stages, while leaving frame's effective code unchanged. No peer body was edited and no declaration padding was used.

Retained structural alternative `experiments/patches/36795d211160d81b97f88f57c5a219aa50f1052c1219ebf793218b2ce2687b4a.json` preserves80 peers and protected storage and recovers frame's raw-speed/coldness/staggered-clone constraints, but play remains17321 bytes/13279 raw differences/455 normalized-exact blocks/61 stack pairs. This is a tradeoff, not a new preferred checkpoint or FUNCTION_MATCH. Full evidence: `build/rootcause26/RESULTS.md`. Fresh canonical verify251/2 and audit pass; prior gate tests/link apply to unchanged inputs. No canonical/proof/configuration edit, promotion or original-executable execution.

## Continuation 27: direct-callee interface control

Checkpoint23's decoded play inventory has272 direct/18 indirect static call sites, unchanged target multiplicities and the same151 resolved absolute data-address set as the original. This does not prove dynamic call order, arguments, pointer accesses, or runtime equivalence. All called recovered game functions are exact except draw_frame; library/import/indirect behavior remains outside that assertion. Canonical frame still has a one-update stripe loop where the original catches up, potentially shifting shared custom-RNG consumption and downstream play effects. The checkpoint fixes that caller behavior; changing the exact RNG body is unsupported.

The type/call intersection identifies six source-owned nullary interface discrepancies: new_rand, do_replay_menu, save_config, startGameMusic, stopGameMusic, update_frame. Round119 restores only their seven audited spelling edits on checkpoint23, preserving every body and header. Frame effective output is identical to checkpoint23, while unchanged play becomes17321 bytes/13279 differences/455 normalized-exact blocks/61 stack pairs, effective b9a01d1474ad6bb3… . This is compiler-context sensitivity, not evidence of a zero-argument runtime ABI correction. Tracked control: `experiments/patches/a0ab57cd1bbee072637cb06b0a40e053166b5713c3303b30e394375a8068759c.json`.

Round120's post-join frame-zero route leaves play in the same effective group. Both new patches pass80 exact peers, protected bodies/storage, and unchanged checkpoint23 play-body identity. Neither is an improvement to the joint checkpoint. Frame's bounded timer/predicate checks do not certify play's remaining near/spill regions. Full evidence and explicit canonical-versus-checkpoint behavioral distinction: `build/rootcause27/RESULTS.md`. Accepted state remains251/253; no promotion or canonical/proof/configuration edit.

## User-directed search priority: targets first, repair peers afterward

The user authorized temporary regressions elsewhere while pursuing strict play/draw_frame equality in scratch. Treat peer losses as recorded repair obligations, not an exploration veto; preserve the full canonical promotion gate. Once a target is exact, preserve its reproducible patch/receipt and check it on every repair. Both targets must eventually match in the same TU configuration before any joint success claim. Historical falsifiers mentioning peer loss remain unchanged records; future target and repair criteria should be separated.

Read-only review of335 play/327 frame records finds no strict target match, including all21/44 peer-regressing records. Thus no exact solution was hidden solely by the peer gate. Target-only diagnostic metrics retain round88 save_recording_paths as one peer-regressing play frontier observation, eligible for renewed evidence review, not selected on byte distance alone. Investigation26 already demonstrates restoring do_replay_menu without changing frame's effective identity, although neither target was exact. Strategy and full review: `build/rootcause28/STRATEGY.md`, `target-first-frontier.json`. No gate/tool or canonical changes.

## Continuation29: target-first save-path and metadata controls

Rounds121–123 add seven jointly evaluated scratch candidates, retaining collateral losses as repair obligations. Porting round88's two save paths onto current guards gives534 normalized-exact blocks and3 stack pairs but only628 paired blocks and one emitted save_profile site versus the original three; do_replay_menu regresses and is recorded for deferred repair. Removing only the three later guard witnesses gives524 normalized-exact/20 stack pairs. Three explicit save paths with current guards restore three calls and reach17415 bytes, but626 paired/525 normalized-exact blocks are worse than checkpoint23's630/532. No target milestone was preserved or discarded merely because of peer loss.

The original-like common source save becomes three calls at060t.vrp1. The two-source-call control retains two through final trees/IRA/dse2;181r.csa cross-jumps11 common instructions from block431 to432 and deletes save-call UID3249, leaving3272. A same-output diagnostic trace pins this to late CFG cleanup rather than tree merging. Preserve the single-common-save/VRP1 model for further work; do not pursue static call duplication based only on raw scores. Full call topology and event: `build/rootcause29/RESULTS.md`, `save-crossjump-event.txt`.

Three independent original-DWARF context controls restore last_log[256], show_name(const char*,int,void*), or both without editing any body/header. Array extent leaves both targets' effective outputs unchanged. Callback signature leaves frame unchanged but regresses play to17321 bytes/13297 raw differences/455 normalized-exact/60 stack pairs; the combination is identical at the target level. Fresh DWARF confirms the intended metadata. All80 exact peers survive these metadata controls; both array variants incur a Common/BSS allocation gate obligation while preserving established protected storage. They are exploratory evidence, not accepted storage fixes.

All81 ordinary recoverable main.c definitions already match original DWARF declaration-line order; emitted address order is not source order. No reordering experiment is warranted. Across seven controls four pass the general function/allocation guard, one loses do_replay_menu, two alter common allocation, and all pass established storage protection. Frame output stays checkpoint23 throughout. No strict match or promotion; canonical/proof/configuration inputs remain unchanged. Full metrics/patch identities and validation: `build/rootcause29/experiment-summary.json` and `RESULTS.md`.


## Continuation30: lexical scope audit and causal controls

The prior type audit did not establish scope/declaration fidelity. Original play has20 non-inline scopes versus checkpoint23's18, including a separate addTime at4375 and a results block with15 ordered locals. Eight joint controls in rounds124–126 restore specific original scopes/declarations while leaving canonical inputs unchanged. Optimized DWARF is a constraint, not unique source proof; locationless missing computations were not invented.

Statistics braces/k/XML scope changes alone leave both targets' effective output unchanged. Removing the synthetic flags cache alone reproduces the combined statistics regression to17321 bytes/456 normalized-exact/60 stack pairs. Thus the regression is caused by expression/CFG restructuring, not these braces. Results-scope restoration changes only five initializer instructions in play+0x2d90..0x2dad; all other emitted instructions equal checkpoint23, retaining630 paired/532 normalized-exact/3 stack pairs. The original initialization order is still not fully reproduced.

The original post-game timer local addTime is independently evidenced by DWARF block133223/location0x7541; checkpoint23 reuses outer diff. Restoring it alone regresses play and untouched do_replay_menu. The peer agrees through122t.uncprop, then two constant assignment pairs reverse at123t.optimized after SSA elimination. Combining the original timer local with results-scope restoration repairs the peer and all76 compared stages, without editing its body. Retained alternative experiments/patches/cd9dbc91b575f4dc146eb003aa75b294e9fb91111923e9094507c2de8c38d10d.json has17419 bytes/13284 raw differences/630 paired/532 normalized-exact/3 stack pairs and80 exact peers; neither target is exact. The isolated regression remains recorded as a repair obligation, not discarded merely for peer loss.

All eight frame outputs stay checkpoint23. Seven general function/allocation guards pass; isolated addTime loses do_replay_menu. All eight established-storage checks pass and diagnostic target outputs reproduce ordinary-search effective identities. These scope discrepancies are real but do not explain both residuals. Full scoped evidence, metrics, patches, pass diffs and unchanged input receipts: build/rootcause30/RESULTS.md and experiment-summary.json. Canonical251/2, source/header/proof/configuration unchanged; no promotion or original-executable execution.

## Continuation31: frame value-use control repairs unchanged play allocation

Six joint frame controls in rounds127-129 keep checkpoint23's play body byte-identical. Removing fallback sx work recovers frame's five reset copies but distinguishes cached-speed spilling from direct-field reloading. A dominating definition then preserves both the original-like reset clone timing and raw x87 lifetime. Its ordinary spelling leaves play17321/13279/455 normalized-exact/61 stack pairs; using the same definition in airborne refinement leaves frame effective code identical and restores play17419/13267/532 normalized-exact/3 stack pairs. This is a source-context effect, not a play body edit or FUNCTION_MATCH.

Retained mechanism patch experiments/patches/e40d78bf875a069c088c10d6826b605f96b00d3e9ae599df1c3356e2f5079dea.json has frame8539 bytes/6483 raw differences/212 paired/170 normalized-exact and80 exact peers. All six general/body/common-allocation and established-storage guards pass; no regression was used as an exploration veto. Bounded pose/reset behavior checks and fresh output/input receipts are in build/rootcause31/RESULTS.md and experiment-summary.json. Neither target is exact; canonical/proof/configuration unchanged, no promotion or original-executable execution. Next frame discriminator is excess speed lifetime on nonwalking pose paths, preserving the newly recovered five-copy/cold-compare/live-x87 conjunction.

## Continuation32: frame load-placement control preserves effective play

Seven joint frame controls in rounds130-132 leave checkpoint23 play source byte-identical. The early-load controls both reproduce the investigation31 parent's effective play output:17419 bytes/13267 raw differences/630 paired/532 normalized-exact/3 stack pairs. Frame's before-pose-chain control removes measured cache-load duplication while retaining five resets, staggered cloning and live x87 speed; it remains DIFFER at8508/6019/211 paired/172 normalized-exact. Tracked mechanism control experiments/patches/08e89a9cf9ab3ec78c5509c163ec67f1c5e0791a17679dc8351ffedb48e1aca5.json preserves80 exact peers and all body/allocation/storage guards. Its speed-load placement still disagrees with original disassembly, so no promotion or recovered-function claim follows from the ten-byte length gap.

Refinement partitions and fallback gates give distinct unchanged-play allocation outcomes, all retained with multi-metric observations. Fresh parent reproduction, candidate diagnostic/search identity checks, bounded source model and canonical-input receipts are in build/rootcause32/RESULTS.md and experiment-summary.json. The frame load-duplication cause starts at VRP1 rather than late sinking. No canonical/proof/configuration change; accepted251/2 unchanged.

## Continuation 33: branch-owned zero path and speed liveness

Rounds 133–137 test seven compact TU patches jointly; neither target is exact. The interval fallback's missing idle-zero reset is traced to serial pose PHIs around an intervening load. Owning the fallback in the nonidle branch restores the literal-zero reset input, 2/3/5 cloning, cold frequency-8 thresholds and live raw x87 SX, but loads the nonidle speed too early.

Lazy branch ownership preserves reset cloning but spills SX. Both forms expand speed as a register; IRA gives the eager form one root allocation assigned x87 reg8, whereas the lazy form carries speed through five unrelated loop regions and spills at memory80 versus register26544. An ordinary zero initializer removes those five regions and restores reg8 without losing the five resets or raw SX. This is a definedness/liveness mechanism, not evidence of an original speed local. The initialized control still has an extra cmp4 pose decision and wrong fallback placement; the initialized walking-predicate variant loses the zero clone again.

The retained definedness witness is candidates/rootcause33/defined/lazy_cache_initialized.json: frame8558 /6134 raw differences /209 paired /169 normalized-exact; play17419 /532 normalized-exact /3 stack pairs. Neither function matches. The eager branch-owned frame witnesses are8530 /211 paired /170 exact, but their unchanged play body falls to17321 /455 exact /61 stack pairs. Combining the separately evidenced historical results/addTime scopes leaves frame unchanged and does not repair this play allocation profile. No general-transfer claim for that context repair.

All seven preserve80 exact peers, source bodies, common allocations and established storage; diagnostics reproduce ordinary search effective identities. Six distinct frame models pass20,412 cases each (122,472 comparisons), and the eager/initialized idle routes pass verified-edge symbolic x87 replay. All direct-call inventories match; walking-predicate control has40 versus43 original indirect sites, so static coverage is not universally unchanged. Canonical hashes unchanged; fresh audit251 FUNCTION_MATCH /2 DIFFER,25 TUs. No original executable ran and no promotion occurred. Full pass evidence, allocator decisions, metrics and content-addressed patch IDs: build/rootcause33/RESULTS.md and experiment-summary.json.

Next discriminator: retain the direct zero input and eliminate default-definition live ranges while explaining the original range-then-walking fallback branch. Do not repeat broad guard/load-placement sweeps or assume scope repairs transfer across TU contexts.

## Continuation 34: range ownership removes diagnostic initialization

Rounds138–141 test five TU controls jointly. Using the complement of the existing air-range predicate to own the nonidle fallback removes the extra cmp4 interval decision, keeps literal0 at the reset PHI, preserves2/3/5 clone history and frequency8 signed thresholds, and retains one unspilled x87 SX through both idle comparisons. Removing the diagnostic zero initializer produces identical effective outputs for both functions: both forms have one root speed IRA allocation assigned reg8, with no five unrelated-loop allocations. The new graph itself eliminates that default-definition liveness.

Retained initializer-free witness: candidates/rootcause34/default_control/range_owned_without_default.json; frame8520 /6456 raw differences /211 paired /172 normalized-exact; unchanged-body play17419 /532 normalized-exact /3 stack pairs. Original frame8518 does NOT imply only2 wrong bytes. This is a causal witness, not a replacement for all checkpoint metrics or a strict match.

The remaining emitted ordering mismatch is explicit: original compares pose1 at+0x74d then loads SX+0x756; witness loads SX+0x74d before pose comparison+0x750 and needs a discard+0x759. Registers, pose materialization and layout also differ. An explicit range-else status gate gets its fifth reset only at DOM2, not VRP2. Selecting fallback inside ABS repeats status selection and yields four frequency4 thresholds; selecting once restores two frequency8 thresholds but leaves the status branch/default-zero path and wrong2/2/4 clone history. These are separate falsified mechanisms.

All five preserve80 exact peers, bodies/common allocations/storage, reproduce search outputs in diagnostic builds, and retain original direct-call inventory plus43 indirect sites. Five stable-field models pass20,412 cases each (102,060 comparisons); retained idle routes pass verified-edge symbolic x87 replay. No whole-runtime or exception/concurrency equivalence claim. Canonical hashes unchanged; fresh audit251 FUNCTION_MATCH /2 DIFFER,25 TUs; no original executable run or promotion. Full evidence and durable patch IDs: build/rootcause34/RESULTS.md and experiment-summary.json.

Next discriminator is fallback-load ownership after walking-pose validation while retaining a single dominating pose fact for the shared signed comparisons. Do not reintroduce a status selector or mixed-pose PHI without a new causal reason; do not treat the diagnostic speed local as historical source.

## Continuation 35: direct-field PRE separates equality from availability

Fresh direct-field reproduction matches investigation31. PRE recognizes idle/air/reset sx loads as the same value219 and sign value220 under the same pointer/memory version. At mixed-pose guard77, however, SX is absent from both all-path availability and anticipation; it is available on idle predecessor222 and anticipated only in walking successor78. This control's missed reuse is not differing alias/memory identity.

Rounds142–143 test three source controls. Explicit p_im=1 on idle .02-false bypasses the walking guard and lets PRE insert fallback SX after validation, merging it with idle SX. Original compare+0x74d /load+0x756 placement is recovered without a named cache; both idle routes pass one-load/no-spill symbolic x87 replay. PRE also merges the sign Boolean, so DL is reused instead of the original repeated sign comparison (three floating comparisons versus four). A mixed post-guard pose PHI blocks the DOM1 reset copy:2/2/4 stores instead of2/3/5. Control frame8483 /6402 raw differences /211 paired /174 normalized-exact; play17321 /455 exact /60 stack pairs. Deferring the earlier walking assignment gives8476 /171 exact and remains four stores.

Extending only the reset sign comparison to long double folds away before the earliest captured tree: all19 renderings and both target outputs identical. A separate diagnostic -fgcse-after-reload intervention leaves frame and19 tree renderings unchanged, changes play to453 exact and loses draw_reward/main_menu_callback; no target mechanism improvement, no canonical flags changed. Attempted -ftree-partial-pre is unsupported by locked cc1 and produced no object; no conclusion from that failed probe.

All three ordinary source controls preserve80 exact peers, bodies/common allocations/storage and unchanged checkpoint23 play body; diagnostic builds reproduce search outputs. Direct-call inventories and43 indirect sites retained. Each source form passes20,412 bounded cases (61,236 total), with exception/concurrency/full-runtime limits retained. Canonical hashes unchanged; fresh audit251 FUNCTION_MATCH /2 DIFFER,25 TUs; no original executable execution or promotion. Evidence and durable patch IDs: build/rootcause35/RESULTS.md and experiment-summary.json; compiler intervention is separately recorded in late-load-probe-result.json.

Next discriminator must separate field reuse from sign reuse while retaining usable pose information after walking validation. Source-cache range ownership remains an alternative witness with five resets but early fallback load. Neither witness dominates or qualifies as recovered source. Do not repeat alias or generic late-load optimization hypotheses without new evidence.

## Continuation 36: field/sign reuse separated; reset timing still unresolved

Rounds144-146 test nine ordinary source controls. Changing only idle or reset ABS sign to strict-positive/negative-first preserves late fallback cmp+0x74d/load+0x756 and raw SX PRE while eliminating the sign-Boolean PHI. Eight signed x87 path replays verify one SX load, no spill/reload and four comparisons. One sign predicate still differs from historical code; this is a mechanism witness, not recovered source. All four sign controls retain2/2/4 reset stores. idle_positive_sign reaches175 normalized-exact frame blocks,8483-parent to8475 bytes, but not a strict match.

Whole-RHS walking assignments restore2/3/4 history, but create a shared threshold Boolean PHI and one signed-result reset clone. Two path-specific facts avoid that Boolean join; DOM1 nevertheless learns both signed poses as1 and merges them in block198. They also remain2/3/4, with negative-only fact changing threshold frequency8/8 to8/6. No five-store result.

All nine preserve80 exact peers and unchanged checkpoint23 play body. Negative-first sign forms restore the already-known play17419/532 normalized-exact/3 stack-pair outcome, but three fail data protection: idle-negative swaps +.02/-.02; reset-negative with/without whole-RHS fact swaps +.2/-.2. Exactly two nonrelocation sign bytes change per affected .rdata. The other six pass guards/storage. These concrete collateral obligations are recorded, not hidden by peer counts. Direct-call inventory and43 indirect sites agree for all controls. All diagnostic builds reproduce search effective outputs.183,708 bounded stable-field decisions and eight x87 replays pass within documented limits. Canonical identities unchanged, audit251/2 across25 TUs; no promotion, original execution or proof changes.

Evidence, full metrics and durable patch IDs: build/rootcause36/RESULTS.md and experiment-summary.json. Next source hypothesis must preserve original sign/data ordering and asymmetric DOM1/VRP2 reset history; generic sign rewrites or idempotent pose facts are now discriminated failures. Investigation34 range-owned cache retains that history but still reads fallback SX too early.

## Independent investigation 37: declaration-count sweep

The "context sensitivity" recorded above (532/3 versus 455/60 blocks and stack pairs after frame, prototype, scope or forward-declaration edits) is GCC 4.4.1's `referenced_vars` hash order: PHI insertion, SSA version numbers, operand canonicalization, out-of-SSA copy order and IRA tie-breaks follow DECL_UID modulo the table size, and every declaration, parameter and gimplification temporary before play consumes a UID. The canonical prelude repeats its forward-declaration block sixteen times (lines 491..1752); that repetition acts as fitted UID padding.

A sweep of 256 padding declarations inserted before play on checkpoint23 produces 57 distinct play outputs, all with a masked byte distance between 10544 and 10583 of 17420 and the same first divergence at byte 63, a jump displacement placing the cold `itrcheck` else-block (+0x27f7 originally). Padding at the end of the TU changes nothing. Hence no declaration count makes play exact: the residual is block layout plus a small allocation family, and the hand-explored contexts are members of that family. do_replay_menu changes at 12 of 256 residues, so the fitted count is only right for it modulo collisions. Full tables and receipts: `build/rootcause37/RESULTS.md`, `build/rootcause37/sweeps/`. Recommended next step is the line-table-faithful rebuild that recovered 136 of 152 draw_frame lines, applied to play's 1500 lines, before further allocation reasoning. Canonical 251/2 unchanged; no promotion.

## Independent investigation 37 (play): line-faithful frame

Applying the line-table method to play (`candidates/rootcause37/play_linefaithful_v16.c`, round 147):
every statement placed on its oracle line (delta 768), the DWARF declaration set and block nesting
reproduced exactly (`build/rootcause37/linefaithful_play/declcheck.py`), and inline call-site lines
used as anchors. 582 of 710 attributed lines are instruction-identical, 3686 versus 3685 instructions,
17428 versus 17420 bytes, all 80 exact main.c peers kept. The frame corrects checkpoint23 in ways the
line table proves: three `do rest(2); while (cycle_count == 0);` waits (4363, 4734, 4932) where the
canonical `while` re-tests first; `scrollerY` updated only inside `if (summary_scroller_message[0])`;
a `while (hy > hyTarget)` results loop with `float hyTarget = 140.0f`; the hint-string branches with
two crossjumped `strcpy` copies and three unused locals; `int scrollerTargetY = 0` / `rankTargetY = 320`;
three separate `if`s in the key cascade (reproducing the original's byte move); an else-if duplicate
`add_floor` body at 3783/3787; block-local `addTime` at 4066/4159/4222/4375 and `fc`/`ca` on their own
lines; no `initials`, `typed`, `flags` locals. Micro-tests on the locked compiler show `&&`/`||` second
operands and do-while tests take the statement line, so tags on later lines mean separate statements.

What remains (128 lines) is not hash order: a 96-step DECL_UID sweep on the frame moves only eight
copy-order lines. The main-loop register family (player_id/tot_scroll in edx/ecx, about 55 lines) is
constant across the sweep and flips to the original's choice under the diagnostic
`-fno-guess-branch-probability`, so it is decided by estimated block frequencies. The post-loop layout
inversions sit on 50/50 guards (GCC 4.4 does not predict comparisons with zero), and the original
aligns the two rest loops at 4357 and 4932 that the frame leaves below the alignment threshold: the
original's estimated profile is systematically hotter after the main loop. The source construct that
changes the predictions is the open question. Full tables, tooling and receipts:
`build/rootcause37/RESULTS.md` section 6 and `build/rootcause37/linefaithful_play/`. Canonical 251/2
unchanged; no promotion.

Addendum (same day): a 70-build `__builtin_expect` probe (diagnostics only, never candidates) shows
the 3719 register family flips only under large profile shifts at the loop core, and the sync loop at
4357 aligns almost for free only if `if (!debug)` at 4356 were predicted likely, which no natural
construct in GCC 4.4 produces (zero comparisons get no opcode prediction; the call heuristic already
fires identically in both, checked at 4249 and 4337). The x87 compare idioms agree at all 14 sites,
so the results region is cold in both and the 4932 alignment is a trace-start effect. The polarity-swap
spelling is excluded by line order at 4519 and 4883. Details: build/rootcause37/RESULTS.md section 6.2.
IRA identifies the register family precisely: the tie is between the top-region caps of `player_id`
and the ladder counter `scroll_acc` (tot_scroll shares its register in both binaries); the frame
colours scroll_acc first, the original player_id first. Loop-form probes confirm `while (playing)`
(a do-while changes 30 instructions) and a `while (done)` name-entry loop (its test is tagged on
4792); the results loop's form is indistinguishable. No header attributes and no bypass of the
particle-loop guard exist, so the profile difference must come from an unidentified construct.

Semantic verdict (build/rootcause37/linefaithful_play/semdiff.py): the canonical play body has 47
lines the checker cannot explain and 100 differing abstracted instructions against the oracle (real
behaviour gaps); frame v16 has one unexplained line (a move2add 16-bit immediate form) and a 2/4
instruction residue (operand order with inverted jump, duplicated latch jumps), everything else being
register, slot, copy-order or layout differences. The proven redundant-guard idiom
(`if (debug) if (debug && key[KEY_F2])` in handle_player_collision_vector_2) applied to play's guards
lowers the masked distance to 9778 (draw-guard split + doubled name-entry wait) but never reproduces
the original's player_id/scroll_acc colouring order and trades instruction fidelity for layout in a
greedy pass; that family is closed. Data map: every initialized section in all 25 units is
content-equal; BSS is behaviour-neutral.

Correction and simulation result: the three key-wait loops at 4363, 4734 and 4932 are one-line
`while (!cycle_count) rest(2);` loops (entry jumps target the tests at +0x1910 and +0x32ec, back edges the
bodies), not do-while as stated above; frame v17 (round 149) restores them. A lockstep simulation modulo
register allocation, spill slots and layout (build/rootcause37/linefaithful_play/bisim.py) walks v17 against
the oracle over 616 states and 5118 instruction pairs with zero divergences and zero unverified instructions
(13 independent reorders and one narrowed immediate tolerated). The canonical play body is not equivalent
(47 unexplained lines); v17 is the body to build from for behavioural fidelity.

Full-match programs (same day): a witness-guided search over constructs that vanish after branch
prediction (131 builds, doubled/split guards at 34 sites, simulation-filtered) finds nothing that flips
the 3719 register or moves the first divergence; closed. The original prelude order recovered from
DWARF declaration lines (111 globals, 81 functions) is a real lever: fixing the globals' UID order with a
one-line extern block gives a bimodal outcome (588 or 522 identical lines) in the results-region
allocation, but 220 single-global moves all give the 588 outcome and never touch the main-loop register
family. Both programs are exhausted for play; v17 with the extern-block prelude (588/710) is the closest
point recorded (build/rootcause37/RESULTS.md 6.6).
