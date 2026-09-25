# `create_replay` header-owner investigation

Baseline (`src/replay.c` SHA-256 `ef08cac2b6cc853f139f94ec85a17c2473c1f1d13ea502e566910607b805895c`) is `CODEGEN_SIMILAR` at 254 bytes. Its two mismatched operands at function offsets 44 and 52 reference the reconstructed mutable `replay_header` in `.data`. The read-only archive card for historical `REPLAY_HEADER` records a unique global `const char[6]` owner in `.rdata`, VA `0x4d7dd0`, initialized to `ITR140`.

The body-only `literal_header.c` trial replaced the array reference with the literal `"ITR140"`; constant folding changed the function to 246 bytes with 204 differing bytes. It was not useful.

The TU patch `replay_header_owner.json` adds the historically evidenced `static const char REPLAY_HEADER[6]` while retaining the existing mutable object, then points `create_replay` at the const owner. The verifier reports `create_replay` as `FUNCTION_MATCH` and independently accepts the six-byte owner at candidate `.rdata` offset 1136 -> historical VA `0x4d7dd0`. However, the patch loses exact matches for `get_replay_property` and `load_replay`; the candidate `.rdata` contribution grows from 1152 to 1160 bytes and is not content-equal to the oracle.

`replay_header_all_references.json` also routes both header checks through the owner. `load_replay` becomes exact, but `get_replay_property` still differs because its other `.rdata` references resolve at shifted section offsets. The candidate TU still has a non-equal 1160-byte `.rdata` contribution. The strict promotion guard therefore has no proof that the initialized-data change is safe.

No canonical source or recovery state was changed. The candidate, patch, metadata, and search receipts are retained alongside this note. The source-accurate owner remains a translation-unit storage/layout task; do not promote these patches as body-only candidates.
