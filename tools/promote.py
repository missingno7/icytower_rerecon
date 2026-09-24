"""Fresh strict acceptance and atomic publication of a body-only candidate."""
import argparse, copy, json, sys
from pathlib import Path
from common import ROOT, BUILD, identity, read_json, json_bytes, digest, exclusive, run
from scratch import resolve_function, fragment, materialized, acceptance_syntax, patched_source
from verify import load_state, fresh, protect, canonical_inputs, proof_context, STATE
from build import compile_target, targets
from link import ordinary_link, protect_link
from storage import protect_storage
from transaction import LOCK, JOURNAL, publish, recover, ensure_consistent

def emitted_identity(report):
    # Ignore debug path metadata; require identical fresh code, data, BSS, symbols
    # and all non-debug relocation fields across two independent source builds.
    sections={s['index']:s['name'] for s in report['object_sections']}
    runtime={i for i,n in sections.items() if n in ('.text','.data','.rdata','.bss')}
    return digest({'sections':[{k:s[k] for k in ['name','raw_size','virtual_size','sha256']} for s in report['object_sections'] if s['index'] in runtime],
      'symbols':[(s['name'],s['value'],sections.get(s['section']),s['storage_class'],s['type']) for s in report['object_symbols'] if s['section'] in runtime or s['section']==0],
      'relocations':[r for r in report['object_relocations'] if r['section'] in runtime],
      'functions':[(r['name'],r['status']) for r in report['functions']]})

def promote(name,path,target=None,verify_only=False,patch=False):
    ensure_consistent()
    if (BUILD/'promotion-recovery.lock').exists():raise ValueError('Promotion recovery in progress')
    with exclusive(LOCK):
        if (BUILD/'promotion-recovery.lock').exists():raise ValueError('Promotion recovery in progress')
        state=load_state();old_state=STATE.read_bytes();inputs=canonical_inputs();context=proof_context()
        target,config,function=resolve_function(name,target);name=function['name']
        original=(ROOT/config['source']).read_bytes()
        body=None if patch else fragment(path,name)
        complete=patched_source(path,target,strict=True) if patch else None
        if body is not None:acceptance_syntax(body)
        baseline=fresh(target,BUILD/'acceptance/baseline'/target)
        prior_exact={n for n,f in state['units'][target]['functions'].items() if f['status']=='FUNCTION_MATCH'}
        if prior_exact-{r['name'] for r in baseline['functions'] if r['status']=='FUNCTION_MATCH'}:raise ValueError('Baseline exact set regressed')
        with materialized(target,name,body,source_override=complete) as shadow:
            candidate_source=(shadow/config['source']).read_bytes()
            candidate=fresh(target,shadow/'verified',shadow)
            row=next(f for f in candidate['functions'] if f['name']==name)
            if row['status']!='FUNCTION_MATCH':raise ValueError('Target is '+row['status']+'; first mismatch '+str(row.get('first_difference')))
            protect(baseline,candidate)
            protect_storage(baseline,candidate,BUILD/"acceptance/baseline"/target/"unit.o",shadow/"verified/unit.o")
            with materialized(target,name,body,source_override=complete) as second:
                repeated=fresh(target,second/'verified',second)
                if emitted_identity(candidate)!=emitted_identity(repeated):raise ValueError('Candidate is not reproducible from clean inputs')
                protect(baseline,repeated)
            # All ordinary-link objects are newly built, with the exact verified
            # candidate object reused for its owning TU.
            obj=shadow/'verified/unit.o'
            if identity(obj)!=candidate['build']['object']:raise ValueError('Verified object changed')
            objects={t:(obj if t==target else compile_target(t,shadow/'link-objects'/t,shadow)[0]) for t in targets()}
            link=ordinary_link(shadow/'link',objects,shadow);protect_link(state['link'],link)
            print(run([sys.executable,ROOT/'tools/test.py']),end='')
            if canonical_inputs()!=inputs or proof_context()!=context or STATE.read_bytes()!=old_state:raise ValueError('Canonical inputs changed during acceptance')
            if identity(obj)!=candidate['build']['object']:raise ValueError('Candidate object changed during acceptance')
            updated=copy.deepcopy(state)
            updated['inputs'][config['source']]=identity(shadow/config['source'])
            updated['units'][target]={'functions':{f['name']:{'status':f['status'],'body_sha256':f.get('body_sha256')} for f in candidate['functions']},
              'whole_text_equal':candidate['whole_text_contribution_equal'],'object_match':False,'cu_match':False}
            updated['link']=link
            if verify_only:
                print('Strict acceptance passed; --verify-only leaves canonical state unchanged.')
                return updated
            publish(ROOT/config['source'],candidate_source,json_bytes(updated),original,old_state)
            print('PROMOTED',target+'::'+name,'FUNCTION_MATCH; exact peers, data/BSS, fresh-build reproducibility and ordinary link protected.')
            return updated

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('function',nargs='?');ap.add_argument('candidate',nargs='?',type=Path)
    ap.add_argument('--target');ap.add_argument('--patch',action='store_true');ap.add_argument('--verify-only',action='store_true');ap.add_argument('--recover',action='store_true');a=ap.parse_args()
    if a.recover:
        if a.function or a.candidate:ap.error('--recover takes no candidate')
        recover();return
    if not a.function or not a.candidate:ap.error('Supply function and braced candidate body')
    promote(a.function,a.candidate,a.target,a.verify_only,a.patch)
if __name__=='__main__':main()
