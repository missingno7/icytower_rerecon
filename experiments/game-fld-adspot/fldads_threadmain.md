# fldads_threadmain research, 2026-09-25

The 223-byte candidate and original have the same instruction layout except
for five `.rdata` literal addresses; seven bytes differ. The original 420-byte
contribution places `"Cached ads are up to date"` at offset 191, before the
download messages. The current contribution places it at offset 389, after
them. Both strings and section size are otherwise the same.

A cached-first if/else changes code length to 204 bytes while leaving the
literal order unchanged. Separate cached and download ifs preserve 223 bytes
but still leave literal order unchanged. GCC's literal emission follows its
optimized block layout in these variants, so lexical source order alone does
not prove the original contribution. No variant was promoted; see
`fldads_threadmain.jsonl`, rounds 1-2. Further work needs a source-backed way
to recover the original literal order and independently prove the whole `.rdata`
contribution without changing accepted peers.
