# Viewer recovery, 2026-09-25

Accepted: view_profile, 2249 bytes, FUNCTION_MATCH.
Current game total: 214 FUNCTION_MATCH, 38 DIFFER, 1 CODEGEN_SIMILAR.
The profile TU has 14 exact functions and three DIFFER functions.
recovery.json remains the sole accepted-state authority.

## Source and storage findings

DWARF line order places the first animation interpolation before blit_to_screen.
The optimized assembly rotates it into the loop header; following assembly order
literally in C had prevented recovery. Both animations use double 0.2 and convert
the complete updated position to int. Other recovered details include BITMAP
locals, inline rank evaluation, sprite coordinates (50, 0) and call order,
unconditional control/key polling with boolean normalization, positive tick
waits, and a zero return. The original close-button-first guard is restored;
create_profile remains exact with this recovered surrounding code.

Resolving the shared double required independently proving the complete original
initialized contributions: .data (392 bytes) and .rdata (2032 bytes). Remove the
unsupported trailing period from the selector format and restore its shared
double arithmetic. Selector evidence also establishes 16-row paging, independent
control checks, repeat-delay reset, delete key 77, activation keys 67/75, direct
profile handle expressions and call argument order, animation source order, and
local buffer scopes. These corrections reproduce the original switch table.
No proof code, compiler flags, or acceptance gates changed.

select_profile remains DIFFER (3051 versus 3070 bytes): the existing-profile
load failure path still merges its success tail differently and two stack slots
are exchanged. draw_buffer and draw_profile_selector also remain DIFFER.
Equivalent failure-condition and lexical-scope variants did not resolve this.
An unconditional exit variant was diagnostic only and was not accepted because
it would also exit after a failed load.

## Validation and identity

Promotion passed exact-peer/body protection, initialized storage and BSS checks,
independent clean-build reproducibility, ordinary linking of all 25 game units,
and the 45-test acceptance suite. Fresh post-publication verification confirms
14 exact profile functions. Archive audit confirms unchanged metadata for 47,600
files and unchanged hashes for 302 imported inputs. No original game was run;
object, CU, executable and runtime equality remain unclaimed.

Accepted patch (relative to experiments/):
patches/bc6940f5d86ad851ddada87fc1227a316953489993e263430a2f7114c733b3f5.json
Search record: game-profile/view_profile.jsonl, round 14, recover_viewer.
Accepted src/profile.c SHA256:
441b3fa70574fcfa116e7f6e97e95c2a8f3775272de90170db96dffef0ea4319

Next evidence-led candidates: draw_buffer's original index loop (DWARF has pos,
tempBuf and tempPos, without the current char temporary); selector failure-path
tail merging; draw_profile_selector code shape. Run every hypothesis in scratch
and protect the newly accepted viewer along with all prior exact functions.
