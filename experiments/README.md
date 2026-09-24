# New experiment history

Production history starts empty. Search writes one compact JSONL log per
`game-target/function`, plus content-addressed body fragments or TU patches.
Shared environment identities are stored once under `environments/`.

Records include function, round, variant, parent, hypothesis family, description,
prediction/falsifier, candidate hash, environment reference, effective output ID,
compiler response and outcome. They are observations, not recovery authority.
Only `../recovery.json` represents accepted recovery.

Compiler objects, complete temporary translation units and disassemblies are not
retained here. Migration smoke-test history lives under ignored build/smoke/.
Historical research remains in ../../icytower_recon/docs/attempts/ and should be
queried selectively, not imported as logs, queues or full-TU experiment snapshots.
