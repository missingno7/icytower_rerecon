This is the active historical matching reconstruction. Read README.md first.
The old ../icytower_recon repository is READ-ONLY cold storage. Never run its
writing tools there, update its reports, clean it, or import its queues/log trees.

Preserve the 25 original source units and historical code generation. No SDL,
modernization, stylistic source edits, binary patching, copied machine-code
fallbacks, guest-address dispatch or per-function link placement. Do not invent
asm, attributes or volatile merely to force bytes. The oracle is verifier-only.

Normal work:
1. python tools/context.py FUNCTION (request --locals/--asm/--history as needed).
2. Place braced candidate bodies in ignored candidates/, or use compact one-TU
   JSON replacement patches for structural hypotheses.
3. python tools/search.py FUNCTION CANDIDATES... (add --patch for JSON patches).
4. Reason over multiple metrics and grouped effective outcomes; keep predictions
   and falsifiers in --meta. No compile quota or fixed batch size. Ten genuinely
   stagnant rounds suggest meta-analysis or a new family, not a permission gate.
5. python tools/promote.py FUNCTION WINNER [--patch]. Never hand-edit recovery.json.

Scratch candidates may be wrong. Only fresh strict acceptance changes canonical
source. Exact peers and established data/BSS ownership cannot regress. The gate
also requires repeatable clean emission, ordinary source link closure, tests and
stable inputs. Masked/effective equality never grants FUNCTION_MATCH. All current
claims are function-level; object/CU/executable/runtime recovery remains incomplete.

Do not mutate canonical src/ or include/ to experiment. Header or cross-TU changes
need a reviewed gate extension. Source/header identities are pinned in recovery.json;
proof-code/configuration changes require verify.py --all --refresh, which may not
lose any existing exact function. Run tools/test.py and tools/audit.py for gate work.
Generated evidence and full receipts belong under ignored build/. Keep experiments
as fragments/patches with shared environment references, never full TU copies.

If a promotion is interrupted, use tools/promote.py --recover; do not delete its
journal or overwrite concurrent edits. No original executable is ever executed.
See MIGRATION.md before selectively consulting historical archive experiments.
