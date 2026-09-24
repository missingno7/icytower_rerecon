# Migration inventory (before import)

Archive: `../icytower_recon`, HEAD `a7bfbb08a031a75133a768043f5daf194b6d4f46`. Read-only throughout.
Both HEAD and working ledger record 211 FUNCTION_MATCH, 41 DIFFER, 1 CODEGEN_SIMILAR.
Working main.c has three body edits relative to HEAD; source selection requires fresh peer verification.

| Decision | Material | Why / what breaks without it | Approximate size |
|---|---|---|---|
| MUST IMPORT | 25 C sources and used recovered headers | Historical TU context and current recovery disappear | 450 KB |
| MUST IMPORT | Actual Allegro/Xiph/PNG/DirectX source and header closure | Ordinary compile and source link fail | 3.7 MB |
| MUST IMPORT | PE/COFF/DWARF parser and independent relocation/target/owner proof | Existing FUNCTION_MATCH guarantees are lost | 80 KB |
| MUST IMPORT | Toolchain, fixture, source provenance hashes | Compiler drift or wrong oracle can pass unnoticed | Small metadata |
| MUST REGENERATE | Original DIE graph, lines, ranges and file mappings | Needed on demand, reproducible from locked oracle | Ignored indexed SQLite |
| MUST REGENERATE | Fresh function proofs and ordinary link | Old receipts cannot establish new recovery state | Ignored build output |
| KEEP IN ARCHIVE | Build output and full TU experiments | Disposable experiments, not source inputs | 14.6 GB build |
| KEEP IN ARCHIVE | docs/current, docs/attempts, queues, epochs, routing | Administrative / duplicated evidence | 6.2 GB docs |
| KEEP IN ARCHIVE | TDM-1, C++, unused vendor code, research hypotheses | Not used by current production configuration | Most old dependency trees |
| INVESTIGATE | Working source changes vs HEAD | Preserve best source only with exact peers intact | 3 KB diff |
| INVESTIGATE | Ordinary link frontier and known incomplete bodies | Link success is distinct from recovery proof | Compact facts |

Selected by actual compiler dependency scans: 302 source/header files (4153603 bytes), 116 C toolchain inputs (25094723 bytes). Toolchain/fixture binaries will be local ignored setup inputs; no binary game assets are tracked.

This inventory was written before source/toolchain imports. Final measurements and validation follow after verification.

## Final migration result — 2026-09-25

Ready as the active matching recovery workspace. The original archive was not modified.
The final audit found identical paths, sizes and modification times for all **47,600** archive files, plus unchanged SHA-256 hashes for all **302** initially examined source/header inputs. No archive scripts were executed in writing mode. Git reads used per-command ownership overrides, not global configuration changes.

### Old repository footprint

This measures the complete checkout, including ignored build output and Git storage.

| Directory | Files | Bytes |
|---|---:|---:|
| (root) | 149 | 14,061,620 |
| .git | 4,941 | 412,598,248 |
| .pytest_cache | 5 | 44,199 |
| assets | 75 | 41,815,761 |
| build | 27,723 | 14,634,894,107 |
| docs | 9,270 | 6,239,971,705 |
| evidence | 41 | 93,972,513 |
| include | 49 | 49,784 |
| resources | 1 | 839 |
| src | 28 | 1,310,679 |
| third_party | 2,036 | 30,832,281 |
| toolchain | 2,776 | 171,158,656 |
| tools | 506 | 4,928,696 |
| **Total** | **47,600** | **21,645,639,088** |

### Actual imports and exclusions

- 25 original C units (496,238 bytes), unchanged from the archive working tree, and 45 actually used project headers (46,036 bytes). No game code was modernized or reformatted.
- 230 required vendor source/header files, plus notices and the symbolic PNG import definition. The final source build uses 114 Allegro units, 22 Xiph units and the separately recovered logg extension. Link-map inspection proved Allegro `inline.c` and `math3d.c` were not pulled; both were omitted from the final project. Every remaining recipe unit is pulled by the ordinary link.
- Historical dependency and toolchain identities. The local compiler setup contains only 110 C toolchain files (22,541,174 bytes), plus the separately locked objdump/libzstd analysis pair. Unused executable/DLL entries and the C++ package hypothesis were excluded.
- The original executable identity and compact PE identities, 25 historical TU build contexts, and a few freshly checked source-provenance observations. The executable itself remains an ignored local fixture.
- The independent PE/COFF reader, DWARF parser, instruction decoder, direct-transfer verification, data-owner/type proof and strict comparison core. Existing corruption controls were selected without importing their workflow dependencies.

Left in the archive: all docs/current and docs/attempts trees, giant JSON/JSONL receipts, current cards and queues, CHEAP/MEDIUM/SUPERVISOR routing, epochs, reopen semantics, task sessions, duplicated full main.c experiments, cached graphs, rejected compilers and unused upstream trees. No resources, proprietary assets, source archives or old prebuilt library outputs were copied into source control.

### Rewritten machinery

Bootstrap and compilation now verify a small exact C toolchain allowlist, actual include dependencies and pinned upstream files. Ambient compiler overrides are cleared. The original compiler never reads the oracle.
Evidence extraction builds an indexed SQLite database directly from the immutable executable. The old 59 MB DIE JSONL and its duplicated type/global/function/card views were not imported. All 130,019 DIEs, line/file mappings, lexical scopes, location/range lists and indexed COFF symbols remain queryable. The database is reproducible, hash-checked, ignored build output; no generated database is a competing authority.
The comparator retains full length/byte equality, complete instruction decoding and independent relocation/same-CU target resolution. Its old JSON-file reads were replaced by narrow database queries. Search-only scheduling projections and the old command/report pipeline were removed. No masked mismatch was promoted to FUNCTION_MATCH.
Search, context, effective-output grouping, Pareto responses, source-built library linking, strict promotion and publication recovery were written for this smaller structure. There are 23 Python tool modules and four test modules; no old orchestration dependency remains.

### Fresh recovery and source selection

| Scope | Old HEAD | New fresh proof |
|---|---:|---:|
| FUNCTION_MATCH | 211 | 211 |
| DIFFER | 41 | 41 |
| CODEGEN_SIMILAR | 1 | 1 |
| Game translation units | 25 | 25 |
| Inventoried game functions | 253 | 253 |

The sets of exact function names agree, not just the totals. HEAD is `a7bfbb08a031a75133a768043f5daf194b6d4f46`. Only main.c differs between HEAD and the imported working source: blit_to_screen, main_menu_callback and force_create_profile have prior uncommitted recovery edits. HEAD main.c was compiled separately in scratch; both versions freshly preserve the same 64 exact functions. The working source was retained. Known incomplete draw_frame, play and main_menu_callback observations were reduced to freshly checked hashes and measurements.
Modified logg also freshly verifies 18/18 functions and its complete text contribution, separately from the 211 game-function count. The ordinary diagnostic source link closes after rebuilding the selected dependencies. No binary is executed and no playable-game, object, CU, whole-executable or Xiph-byte-equality claim is made.

### Active workflow

`context.py FUNCTION` returns a compact packet (create_profile is about 1.2 KB). Explicit flags expose locals, assembly, source lines, globals, callers, raw DIEs or grouped history. COFF-only and indirect callers are not misrepresented as complete call-graph coverage.
`search.py FUNCTION CANDIDATES...` stores body fragments or small exact-replacement TU patches, one shared environment per identity and compact outcome records. A temporary complete context is removed after each compilation. There is no batch-size/model/attempt gate. Equivalent resolved compiler outputs group together; ten stagnant reasoning rounds suggest a new strategy without stopping research.
`promote.py FUNCTION CANDIDATE` freshly proves the baseline and candidate, protects exact peers and data/BSS, repeats the clean build, performs an ordinary link and runs the tests. It rechecks inputs, then publishes the source and recovery.json with a durable rollback journal. Recovery preserves unrelated edits and refuses live-process interference. Header or cross-TU changes require an explicit extension of this gate.

### Validation

- 45 tests pass: real COFF code/relocation/call/literal/initializer corruption; independent owner/type checks; short/near transfer boundaries; source races; oracle isolation; input drift; patch scoping; storage and link regression; atomic rollback/crash recovery; live-process protection.
- End-to-end smoke: 12 distinct source spellings produce one effective output; a repeated round records stagnation; a compact TU patch runs in scratch; an incorrect candidate is refused; a successful unchanged-body promotion exercises the complete gate and leaves canonical source/state byte-identical.
- All 25 game units freshly reproduce 211/41/1 after final dependency pruning, and the ordinary link closes. Final source, toolchain, fixture and archive audits pass.
- Production experiment history begins empty. Synthetic migration checks and their receipts remain under ignored build/smoke/. No full-TU experiment snapshots are tracked.

### New source-controlled footprint

**350 files; 4,470,556 bytes** (about 4.47 MB). This table excludes .git, build and candidate scratch files.

| Directory | Files | Bytes |
|---|---:|---:|
| (root) | 6 | 64,584 |
| evidence | 3 | 11,285 |
| experiments | 1 | 867 |
| include | 45 | 46,036 |
| src | 25 | 496,238 |
| tests | 4 | 33,386 |
| third_party | 240 | 3,659,352 |
| toolchain | 3 | 24,488 |
| tools | 23 | 134,320 |

Ignored local setup, evidence and validation artifacts currently occupy approximately 149.9 MB across 912 files. Those are disposable or reinstallable outputs, not imported canonical knowledge.

Exactly one machine-readable file represents accepted recovery: **recovery.json**. Other JSON files are immutable identities/build recipes, historical observations or disposable search records. They are not synchronized copies of recovery status.

### Largest 30 source-controlled files

| File | Bytes |
|---|---:|
| src/main.c | 311,299 |
| third_party/dx80_mgw/include/ddraw.h | 240,099 |
| third_party/dx80_mgw/include/dinput.h | 221,049 |
| third_party/libpng-1.2.34/png.h | 156,126 |
| third_party/libvorbis-1.2.0/lib/window.c | 130,661 |
| third_party/dx80_mgw/include/dsound.h | 109,997 |
| third_party/allegro-4.4.1/src/file.c | 76,131 |
| third_party/allegro-4.4.1/src/unicode.c | 68,718 |
| third_party/zlib-1.2.3/zlib.h | 66,188 |
| third_party/allegro-4.4.1/src/gui.c | 63,937 |
| third_party/libvorbis-1.2.0/lib/vorbisfile.c | 62,478 |
| third_party/allegro-4.4.1/include/allegro/internal/aintern.h | 62,133 |
| third_party/allegro-4.4.1/src/font.c | 61,500 |
| third_party/allegro-4.4.1/src/sound.c | 57,238 |
| third_party/allegro-4.4.1/src/poly3d.c | 55,220 |
| third_party/allegro-4.4.1/src/mixer.c | 52,503 |
| third_party/libogg-1.1.3/src/framing.c | 50,958 |
| third_party/allegro-4.4.1/src/graphics.c | 50,394 |
| third_party/allegro-4.4.1/src/guiproc.c | 50,187 |
| third_party/allegro-4.4.1/src/datafile.c | 47,347 |
| third_party/libpng-1.2.34/pngconf.h | 46,290 |
| third_party/allegro-4.4.1/src/midi.c | 43,414 |
| recovery.json | 42,195 |
| third_party/libvorbis-1.2.0/lib/masking.h | 40,215 |
| third_party/allegro-4.4.1/src/c/cspr.h | 39,868 |
| third_party/allegro-4.4.1/src/gfx.c | 37,757 |
| third_party/allegro-4.4.1/src/win/wdsound.c | 36,864 |
| src/replay.c | 36,668 |
| third_party/lock.json | 35,527 |
| third_party/allegro-4.4.1/src/config.c | 35,101 |

The largest file is the unchanged historical main.c. The next large files are required DirectX/libpng/zlib headers and upstream Allegro/Vorbis implementation tables, all reached by the compile/link closure. Splitting, rewriting or stripping them would discard useful historical source or change its context. The 55 MB generated SQLite index is deliberately absent from this table and from Git.

The genuinely indispensable old machinery was the historical compiler identity and source/header context, the original executable/DWARF, and independent relocation/target/data-owner verification. The old administrative representations were not indispensable.
