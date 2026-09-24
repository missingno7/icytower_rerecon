# Icy Tower 1.5.1 — minimal matching reconstruction

25 historical game translation units. **Migration baseline: 211 FUNCTION_MATCH,
41 DIFFER, 1 CODEGEN_SIMILAR.** The ordinary source link closes, but this remains
an incomplete reconstruction. No playable-game, OBJECT_MATCH, CU_MATCH or
whole-executable claim is made. Current accepted state lives in `recovery.json`.

The workflow is **evidence → scratch search → strict acceptance**.
`../icytower_recon` is read-only cold storage. Normal work does not depend on it.

## Setup and reproduce

Use Windows and Python 3.10+ with its standard library, from this directory:

```powershell
python tools/bootstrap.py --archive ..\icytower_recon
python tools/verify.py --all
python tools/test.py
python tools/audit.py
```

Setup copies only hash-locked C tools, the analysis tool and the user-supplied
oracle into ignored `build/local/`. Alternatively, supply `--toolchain-dir PATH
--analysis-dir PATH --fixture PATH` without the archive. See
[toolchain/README.md](toolchain/README.md) for identities and required files.
No executable, runtime DLL, game asset, generated object or library is tracked.

```powershell
python tools/build.py game-timer       # compilation never reads the oracle
python tools/link.py                  # ordinary diagnostic source link
python tools/evidence.py --rebuild    # regenerate indexed oracle facts
```

The diagnostic game is never automatically executed. Xiph and original
import-library identities have remaining limitations described in
[third_party/README.md](third_party/README.md).

## Work on a function

```powershell
python tools/context.py create_profile
python tools/context.py create_profile --locals
python tools/context.py create_profile --asm
python tools/context.py create_profile --callers
python tools/context.py create_profile --history
python tools/search.py create_profile candidates\a.c candidates\b.c
python tools/promote.py create_profile candidates\winner.c
```

Candidate `.c` files contain **one braced function body**, without a declaration
or signature. Use ASCII or the original CP1252 encoding. The complete historical
TU is materialized temporarily; surrounding source is preserved byte-for-byte.
Use `--target game-profile` or `game-profile::create_profile` for ambiguous names.
Never edit canonical source merely to experiment. Scratch copies are independent
files and are removed after compilation, including on failure.

A search call is a reasoning round with as many variants as useful. Optional
`--meta round.json` records `round`, `parent`, `family`, `description`, `prediction`
and `falsifier`. `--insight "..."` records new knowledge when output repeats.

```json
{"family":"local-declaration-order","prediction":"One fewer stack spill",
 "falsifier":"Frame and instruction order remain unchanged"}
```

Responses include strict status, sizes, differing bytes, prefix/suffix, mismatch
islands, frame allocation, instruction order, branches, symbolic calls,
relocations, data ownership and exact-peer gains/losses. Pareto summaries expose
tradeoffs. Byte distance is neither a proof nor a universal search objective.

Effective identities normalize only independently resolved relocation/same-CU
layout operands. Unknown targets retain their raw identity. **Effective equality
never grants FUNCTION_MATCH.** There is no batch size, compile quota, model tier,
task opening or supervisor permission. After about ten nonproductive reasoning
rounds, consider a new hypothesis family or meta-analysis; the tool does not stop
research. Return to cheap experiments when the strategy improves.

## Structural TU hypotheses

For declaration order, globals or peer context, use a small JSON replacement patch.
It identifies one current TU by hash; each `old` fragment must occur exactly once:

```json
{"source":"src/profile.c","base_sha256":"<from context/source identity>",
 "edits":[{"old":"exact existing text","new":"candidate text"}]}
```

```powershell
python tools/search.py create_profile candidates\context.json --patch
python tools/promote.py create_profile candidates\context.json --patch
```

History stores replacements, not another complete main.c. Exact peer bodies remain
protected. Header or cross-TU changes require a reviewed extension of the gate.

## Strict acceptance

Promotion freshly compiles the baseline and candidate in whole-TU context. It
requires exact target bytes/length, complete instruction decoding, every relocation
and independently identified direct target. Exact peers, their bodies, proved
data/BSS ownership and common allocations cannot regress. Previously unproved
storage must stay unchanged or become independently proved; BSS allocation must
stay unchanged for this one-TU operation.

A second clean build must reproduce code, storage, symbols and relocations. All
25 game units participate in an ordinary source link. The corruption, isolation
and transaction tests must pass. Inputs are checked again before publishing.
No oracle bytes feed compilation/linking. No binary patches, raw-code fallbacks,
guest-address dispatch, placement tricks or invented asm/attributes/volatile hacks.

```powershell
python tools/promote.py cycle_counter candidates\body.c --verify-only
python tools/promote.py --recover     # after an interrupted promotion
```

Publication uses an exclusive lock and a durable rollback journal. Readers fail
closed while a journal is pending. Failure restores both files; recovery refuses
to overwrite unrelated edits or interfere with a live promotion process.
`--verify-only` exercises the complete gate without publishing.

## Information map

- `src/`, `include/`: historical sources and actually used recovered headers.
- `third_party/`: required historical source/header closure, notices and hashes.
- `toolchain/`: locked setup metadata; tool binaries stay local and ignored.
- `evidence/`: oracle identity, 25 TU contexts and a few checked historical facts.
- `recovery.json`: **the single canonical recovery state**; never edit it by hand.
- `experiments/`: body/patch blobs, shared environments and compact observations.
- `build/`: SQLite evidence, objects, detailed proofs, libraries and diagnostic links.

The immutable executable is the raw evidence authority. A regenerable SQLite index
holds all 130,019 DIEs, file/line mappings, lexical scopes, location/range lists and
PE/COFF facts. Queries load one CU or record, not a 59 MB JSON document. Use
`context.py --raw/--lines/--globals` for more detail, or query individual records:

```powershell
python tools/evidence.py --die 0x1234
python tools/evidence.py --symbol _cycle_count
python tools/evidence.py --location 0x100
python tools/evidence.py --range 0x100
```

After intentional proof-code/configuration changes, run
`python tools/verify.py --all --refresh`. It preserves every accepted exact function
and link closure, and refuses unpromoted source changes. Context/search presentation
changes do not require recovery-state synchronization. Git preserves exact file
bytes; no line-ending normalization changes pinned inputs on checkout.

[AGENTS.md](AGENTS.md) is the short working contract.
[MIGRATION.md](MIGRATION.md) contains the inventory, provenance and final audit.
Old path references in unchanged C comments refer to the cold-storage archive.
