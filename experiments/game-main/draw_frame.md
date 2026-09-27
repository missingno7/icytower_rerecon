# draw_frame: shared idle-speed checkpoint

Preferred unpromoted structural checkpoint: `candidates/continue23-context/checkpoint.json`, tracked as `experiments/patches/d1b1084e9d260ddda666a617b10efdd3c0427de863e8cde1773b6dec4d498f1d.json`. Frame effective identity is `db9489c5661bd19a72d703e393ccabc7ca97b60f48fb35ef936ee8fa69545666`. Accepted status remains DIFFER; normalized block equality is diagnostic only.

## Original constraint and recovered path

The original idle negative-speed path loads sx at+0x8d0, uses FUCOM plus FSTP ST(1) for the .02 comparison at+0x8e6, and reaches the common zero/sign comparison at+0x759 without reloading. The load at+0x756 belongs to another incoming path. Continue22 consumed sx with FUCOMPP at+0x892 and reloaded it at+0x8ad. Full pass tracing in continuation23 locates separate values already in optimized trees and RTL expand, before x87 stack allocation.

An idle-owned cache alone spills the value at ebp-0x198. Duplicating the whole ground reset tail retains sx without that spill, but leaves a sign flag and duplicated paths. The selected source instead enters a shared reset label with the raw speed value from the idle path, and supplies a load on the other p_im 1 incoming edge. Status 0 implies p_im 1 before the idle refinement; skipping the intervening p_im 5..7 test on that edge is valid. No calls or relevant writes intervene. Historical ABS sign selection, thresholds, signed-zero and NaN behavior are preserved.

Selected code has the same nonpopping .02 path at+0xaa6 and a jump at+0xaba to the existing sign comparison at+0x759. There is no intervening reload or spill. This resolves that specific value-lifetime mismatch. The .2 threshold lowering, following reset continuation, register choices and physical layout still differ.

## Joint evaluation and limitations

Selected frame: 210 paired/174 normalized-exact blocks, 30 register and 6 near pairs, 0 stack pairs, 2 spill issue labels; 8459 bytes versus 8518 target, 6478 raw differences, frame allocation 0x1dc. Continue22 had 209/171, 26 register/12 near, 5 spill labels; 8492 bytes, 6413 raw differences. Raw distance therefore worsens despite the recovered path and fewer local-codegen differences.

Whole-TU context matters: the initial shared-entry frame alone regresses play to 628/458 with 61 stack pairs. Combining it with the qualification predecessor correction preserves play's recovered named-local slots (3 residual spill pairs) and yields 630/532. A shared-store control preserves play's continuation22 counts by itself and gives 210/174 frame blocks, but the joint selected variant has fewer extra candidate blocks. Explicit reset-sign distribution regresses play and is rejected. Do not choose these variants from frame-only scores.

Rounds 95-101 record the cache, duplicated tail, shared entry, threshold continuation, sign-distribution and joint controls. Fresh round 102 checks both selected functions with no exact peer loss. No promotion or canonical edit. Full report and receipts: `build/continue23/ROOT_RESULTS.md`; selected original/candidate frame paths: `selected-frame-ob.json` and `selected-frame-cb.json`.
