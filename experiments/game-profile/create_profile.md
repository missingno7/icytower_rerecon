# Profile recovery, 2026-09-25

Accepted: create_profile (823 bytes) and profile_data_page_general (1091 bytes).
Current total: 213 FUNCTION_MATCH, 39 DIFFER, 1 CODEGEN_SIMILAR.
The profile TU now has 13 exact functions and four unresolved functions.
recovery.json remains the sole accepted-state authority.

## Accepted changes and cause

Four pluralization expressions in profile_data_page_general now use > 1 with
plural/singular arms. Their positive enclosing guards make them semantically
equivalent to the prior == 1 expressions. This restores the original cold-block
ordering and all function bytes.

In view_profile, the equivalent exit guard now checks !done before
!closeButtonClicked. Both operands are nonvolatile integers with no expression
side effects. The original DWARF confirms closeButtonClicked is int. This
one-line TU-context change makes create_profile exact without changing its body.
It does not establish original source spelling or exactness of view_profile.

The former two mismatches at create_profile offsets 69 and 72 were the EDI/ESI
encodings of the overwrite load and test, not branch opcodes. A fresh IRA dump
still compares the parameter in memory: final scratch selection occurs later.
GCC 4.4.1 peep2_find_free_register has a static search_ofs and advances it between
scratch allocations. Earlier functions can therefore affect later register
choices. Scratch controls confirmed this dependence; replacing one earlier
function could make create_profile exact while losing an exact peer. Those
controls were explicitly nonpromotable and were not published.

Upstream compiler source inspected (research only, not acceptance inputs):
https://raw.githubusercontent.com/gcc-mirror/gcc/releases/gcc-4.4.1/gcc/recog.c
SHA256 9b1d1b204caf5c49ec0075b80bec2b1ead4a5b7912d6ebeb6b5489a7dd313ea3
The locked local compiler and fresh whole-TU proofs establish the actual result;
upstream source explains the observed mechanism, not TDM patch identity.

## Validation

Both promotions passed fresh strict comparison, exact-peer/body protection,
initialized storage and BSS protection, independent clean-build reproducibility,
fresh ordinary linking of all 25 game units, and the 45-test acceptance suite.
Post-publication verify.py game-profile confirms 13 FUNCTION_MATCH / 4 DIFFER.
Audit confirms unchanged archive metadata for 47,600 files and unchanged hashes
for the 302 imported inputs. No proof tools, flags, or acceptance gates changed.
The game was not executed; no object/CU/executable/runtime equality is claimed.

Accepted search artifacts:
- bodies/78fbdbbf24db54e9d5b3c1c5f6e9b21d2d549c17c7e2d76f752ef748da04f8ce.c
  (profile_data_page_general)
- patches/bef4309361a672e777b09ca2b3f5e99e78ff9fa1a2029f7ea4070a6e23822472.json
  (create_profile via the one-line viewer guard)
Paths above are relative to experiments/. Shared environments and the per-function
JSONL records retain predictions, outcomes and exact-peer effects.

## Deferred viewer work

A broader viewer candidate corrects assembly-backed discrepancies: double
interpolation with conversion after addition, a positive tick wait, sprite
coordinates (50, 0) and call order, inline rank argument evaluation, boolean exit
normalization, and zero return. It remained DIFFER. Although create_profile was
exact in that TU, promotion rejected its changed unproved .rdata contribution.
Nothing from that broader patch was published; the gate was not weakened.
Next useful work is view_profile recovery and independent proof of its floating
literal/storage contribution, using the retained fragments as hypotheses.

## Experiment interpretation

Some early generated multi-line variants accidentally searched CRLF text in an
LF source. Specifically, create_profile round 2 chain variants and round 3 date
permutations/rewards_memset were unchanged controls, not valid eliminations of
those source hypotheses. Round 1 nested_overwrite's multi-line-free change did
apply; view_profile round 2 boolean_combine did not apply and was explicitly
corrected in round 3. Effective grouping recorded the duplicates. These search
mistakes have no role in acceptance. Future generators should assert every
replacement and use source-observed line endings.
