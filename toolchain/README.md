# Locked local tools

`lock.json` selects 110 files (22,541,174 bytes) from the archive's active
TDM-GCC 4.4.1-tdm-2 SJLJ distribution. The list pins the C driver/backend,
assembler, linker, archiver, import-library tool, their recursively required DLLs,
used C/Windows headers, startup objects and link libraries. C++, TDM-1, unused
binutils, installation utilities and runtime DLLs not needed for compilation/linking
are omitted. No executable is committed here.

The original package/archive identities and provenance paths are retained in
`lock.json`. They include gcc-4.4.1-tdm-2-core, bundled binutils 2.19.1,
MinGW runtime 3.16 and w32api 3.13. `files` is the exact setup allowlist; its
`archive_path` names the file relative to the old reconstruction root.

`analysis.json` separately pins GNU objdump 2.40 and its libzstd dependency.
It is only an evidence decoder, never the compiler or linker. System Windows DLLs
are platform prerequisites. Python 3.10+ standard library runs the tools.

```powershell
python tools/bootstrap.py --archive ..\icytower_recon
# Or point directly to already extracted, hash-identical inputs:
python tools/bootstrap.py --toolchain-dir C:\archive\mingw32 --analysis-dir C:\msys64\mingw64\bin --fixture C:\fixtures\icytower15.exe
```

The bootstrap is offline and writes only `build/local/`. It refuses a wrong source
file before copying. Normal builds hash the full compiler allowlist and reject
missing, modified or extra compiler files (including injected specs). Ambient GCC
include/library/path overrides are cleared. Actual include dependencies are checked
before and after each compilation, including locked system headers.

Game flags are locked per TU in `evidence/units.json`: `-O2 -g -mfpmath=387
-DALLEGRO_STATICLINK`; `fld_adspot.c` additionally uses `-fno-toplevel-reorder`.
Historical Allegro macros and header ordering are retained. These settings reproduce
the current game baseline, not an assertion about every original third-party CU.
The original libogg compiler is still a separate unresolved historical question.
