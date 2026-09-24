# Required historical source inputs

Only files reached by the actual compiler dependency scans of the 25 game CUs,
136 linked upstream CUs and modified logg are present. There are no research-only
versions, examples, alternative backends, source archives or prebuilt libraries.
`lock.json` pins every used source/header and records release/archive provenance;
`build.json` is the small source-based library recipe.

- Allegro 4.4.1: required core C/assembly units and headers. Win32 platform selection
  remains in `include/allegro/platform/alplatf.h`. Source bytes are unchanged.
  The required upstream notice is in `licenses/allegro-license.txt`.
- Allegro logg: `recovered/logg.c` retains the reconstructed memory-reader extension.
  The upstream source hash and modification provenance are recorded in the lock;
  its MIT notice is in `licenses/logg-license.txt`. The test suite freshly verifies
  all 18 functions and the complete text contribution, separately from 211 game matches.
- libvorbis 1.2.0 / libogg 1.1.3: the 22 source units needed by the existing source
  link, their used headers and COPYING notices. Source hashes/archive identities
  are retained. Successful linking does not prove historical Xiph byte equality;
  original libogg objects identify GCC 4.2.1-sjlj, which remains unavailable here.
- libpng 1.2.34 / zlib 1.2.3: only required headers and notices. Historical loadpng,
  savepng and regpng sources remain in their original game TU locations under src/.
- DirectX 8 MinGW headers: the exact used header subset; no SDK libraries are copied.
  Provenance identifies the archive and observed hash without claiming proof of the
  original SDK revision. Original per-file notices remain intact.
- `libpng3-derived.def`: symbolic exports used to regenerate a candidate import
  library with the locked dlltool. It was derived from the local runtime DLL;
  the original import-library identity is unresolved. The DLL itself is not imported.

Library objects, `.a` files, maps and executables are reproducible ignored outputs
under `build/`. All imported source modifications, including the recovered logg
extension and platform selection, predate this migration; no upstream code was
rewritten while importing it.
