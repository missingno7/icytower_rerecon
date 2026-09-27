# draw_frame: residual value-lifetime constraint

The preferred joint checkpoint is `candidates/continue22-context/checkpoint.json`; its frame body is unchanged from continue16, effective `fe80d43c7909f8df`, with209 paired/171 normalized-exact blocks. Accepted status remains DIFFER; normalized equality is diagnostic only.

On the original idle negative-speed path, o98@+0x8d0 loads sx and o99@+0x8e6 uses FUCOM plus FSTP ST(1) for the .02 comparison, retaining raw sx. The branch through o100 reaches o74@+0x759 without reloading and then the .2 compare at o75@+0x76c. Candidate c96@+0x892 uses FUCOMPP, consumes sx, and c98@+0x8ad reloads. Original o73@+0x756 is a load on a different incoming path. Comparing only o73+o74 to c98 incorrectly hides this lifetime difference. The saved continue15 rows and fresh continue22 baseline confirm the complete paths.

Continuation22's reset/idle/range implications and opposite idle-frequency controls do not improve the joint result. A local frame gain can perturb unchanged play through the known whole-TU context mechanism. Do not choose by a frame-only block score. Continue investigating the lifetime/merge history, preserving all exact peers. Full scope and receipts: `build/continue22/RESULTS.md`.
