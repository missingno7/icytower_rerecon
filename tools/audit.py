"""Measure canonical footprint and check migration/storage boundaries."""
import argparse, collections, json
from pathlib import Path
from common import ROOT, BUILD, identity, read_json, write_json

def canonical_files():
    ignored={'build','.git','candidates','__pycache__','.pytest_cache'}
    return [p for p in ROOT.rglob('*') if p.is_file() and not any(part in ignored for part in p.relative_to(ROOT).parts) and p.suffix!='.pyc']

def measure():
    files=canonical_files();groups={}
    for p in files:
        rel=p.relative_to(ROOT);top=rel.parts[0] if len(rel.parts)>1 else '(root)'
        g=groups.setdefault(top,{'files':0,'bytes':0});g['files']+=1;g['bytes']+=p.stat().st_size
    for p in files:
        rel=p.relative_to(ROOT).as_posix()
        if rel.startswith(('docs/current/','docs/attempts/')):raise ValueError('Archive bureaucracy imported: '+rel)
        if p.suffix.lower() in ('.exe','.dll','.o','.a','.sqlite'):raise ValueError('Generated/binary input outside ignored build: '+rel)
    if len(list((ROOT/'src').glob('*.c')))!=25:raise ValueError('Historical 25-TU topology changed')
    from verify import load_state
    state=load_state()
    counts=dict(collections.Counter(r['status'] for u in state['units'].values() for r in u['functions'].values()))
    fragments=list((ROOT/'experiments/bodies').glob('*.c')) if (ROOT/'experiments/bodies').exists() else []
    from source_scope import sanitized
    for p in fragments:
        if not sanitized(p.read_bytes().decode('cp1252')).strip().startswith('{'):raise ValueError('Full TU stored as a body fragment')
    return {'canonical_files':len(files),'canonical_bytes':sum(p.stat().st_size for p in files),'by_directory':groups,
       'largest_30':[{'path':p.relative_to(ROOT).as_posix(),'bytes':p.stat().st_size} for p in sorted(files,key=lambda p:p.stat().st_size,reverse=True)[:30]],
       'function_counts':counts,'historical_TUs':25,'recovery_authority':'recovery.json','authoritative_recovery_state_count':1,
       'full_TU_experiment_snapshots':0,'experiment_body_fragments':len(fragments),
       'generated':{'directory':'build/ (ignored)','files':sum(1 for p in BUILD.rglob('*') if p.is_file()),'bytes':sum(p.stat().st_size for p in BUILD.rglob('*') if p.is_file())}}

def archive_check(archive):
    before=read_json(BUILD/'archive-before.json')
    now={p.relative_to(archive).as_posix():[p.stat().st_size,p.stat().st_mtime_ns] for p in archive.rglob('*') if p.is_file()}
    changed=[name for name in set(before)|set(now) if before.get(name)!=now.get(name)]
    plan=read_json(BUILD/'import-plan.json');hash_drift=[]
    for row in plan['source_files']:
        if identity(archive/row['path'])!={k:row[k] for k in ['size','sha256']}:hash_drift.append(row['path'])
    return {'unchanged':not changed and not hash_drift,'changed_file_metadata':sorted(changed),'imported_input_hash_drift':hash_drift,
       'checked_file_metadata_count':len(before),'checked_source_hash_count':len(plan['source_files'])}

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--archive',type=Path);a=ap.parse_args()
    result=measure()
    if a.archive:
        result['archive']=archive_check(a.archive.resolve())
        if not result['archive']['unchanged']:raise ValueError('Archive changed during task: '+str(result['archive']))
    write_json(BUILD/'audit.json',result);print(json.dumps(result,indent=2))
if __name__=='__main__':main()
