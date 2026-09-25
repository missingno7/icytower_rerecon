# draw_buffer recovery, 2026-09-25

Accepted: draw_buffer, 185 bytes, FUNCTION_MATCH. The game now has 216 exact
functions, 36 DIFFER and one CODEGEN_SIMILAR; the profile TU has 15 exact and
two DIFFER. recovery.json is the accepted-state authority.

The historical DWARF has `pos`, `tempBuf`, and `tempPos`, without the former
`char c` temporary. Using `pos` to index the input and `y` to carry the rendered
position reproduces all original bytes. The compiler strength-reduces the index
to pointer movement, explaining why assembly appeared to use a pointer loop.
An indexed while loop and increment-expression variant also emitted exact code;
a direct pointer while loop did not. The accepted candidate is
`experiments/bodies/4f629102a5738fc880064fa62c81253a7754367965b2f5e6f9911da113650d2b.c`.
Search details are in `draw_buffer.jsonl`, round 1.

The first promotion attempt stopped at a stale corruption-test assertion: after
the previously accepted viewer proved the entire .rdata contribution, changing
one `jcLabels` pointee literal left the pointer's address independently proved.
The changed `.rdata` contribution and `protect_storage` both rejected the
mutation. Addend and relocation-type mutations still revoke ownership.
With user authorization, `tests/test_proof.py` now checks the complete storage
rejection for the literal case and retains the two owner checks. All 45 tests
pass. The repository-required `verify.py --all --refresh` passed before
promotion and again after CRLF normalization of that test file.

Promotion passed fresh exact comparison, exact peer/body protection, storage and
BSS checks, repeated clean emission, ordinary linking of all 25 game units,
and the 45-test suite. A fresh profile verification confirms 15 exact functions.
No original game was executed; object, CU, executable and runtime equality are
not claimed. Accepted `src/profile.c` SHA256:
`ea4faf70aa65139453e96c1b58ab277c60d73f4710ad2b926cd2b9b4418e6dc1`.

Remaining profile functions: `draw_profile_selector` and `select_profile`.
A bounded selector drawing search corrected source-backed call order and font
arguments but did not produce exact bytes; see `draw_profile_selector.jsonl`.
