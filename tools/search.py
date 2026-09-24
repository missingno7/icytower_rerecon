"""Unlimited isolated body experiments with compact content-addressed history."""
import argparse, json
from pathlib import Path
from common import ROOT, BUILD, ORACLE, ANALYSIS, identity, read_json, write_json, digest, sha, atomic_bytes, exclusive
from scratch import resolve_function, fragment, materialized, patched_source
from verify import fresh, load_state, canonical_inputs, proof_context
from binary import Binary
from instructions import decode
from metrics import compiler_response, pareto

HISTORY=ROOT/'experiments'
def history_path(target,name):return HISTORY/target/(name+'.jsonl')
def history(target,name):
    p=history_path(target,name)
    return [json.loads(line) for line in p.read_text().splitlines()] if p.exists() else []
def put(directory,data,suffix):
    key=sha(data);path=HISTORY/directory/(key+suffix)
    if path.exists() and path.read_bytes()!=data:raise ValueError('Content-address collision')
    if not path.exists():atomic_bytes(path,data)
    return key

def search(name,paths,target=None,metadata=None,insight=None,patch=False):
    state=load_state();target,config,function=resolve_function(name,target);name=function['name']
    metadata=metadata or {};prior=history(target,name)
    round_id=metadata.get('round',max([r.get('round',0) for r in prior]+[0])+1)
    if not isinstance(round_id,int) or round_id<1:raise ValueError('Round must be a positive integer')
    environment={'target':target,'config':config,'baseline_source':state['inputs'][config['source']],
      'headers':{p:v for p,v in state['inputs'].items() if p.startswith('include/')},
      'compiler':identity(ROOT/'toolchain/lock.json'),'upstream':identity(ROOT/'third_party/lock.json'),'proof_context':proof_context(),'search_tools':{n:identity(ROOT/'tools'/n) for n in ['search.py','metrics.py']}}
    env_id=put('environments',(json.dumps(environment,sort_keys=True,separators=(',',':'))+'\n').encode(),'.json')
    observed={r.get('effective_output_id') for r in prior};observed.discard(None)
    original=Binary(ORACLE).at_va(function['va'],function['size']);original_ins=decode(ORACLE,ANALYSIS,function['va'],function['va']+function['size'])
    baseline={n for n,r in state['units'][target]['functions'].items() if r['status']=='FUNCTION_MATCH'}
    inputs=canonical_inputs();context=proof_context();records=[]
    # One file per function; concurrent searches on other functions are independent.
    log=history_path(target,name);log.parent.mkdir(parents=True,exist_ok=True)
    with exclusive(BUILD/('search-'+target+'-'+name+'.lock')):
        # Refuse a racing append observed after our initial read.
        if history(target,name)!=prior:raise ValueError('History changed; retry the round')
        for path in paths:
            body=None if patch else fragment(path,name)
            complete=patched_source(path,target) if patch else None
            candidate_id=put('patches' if patch else 'bodies',Path(path).read_bytes() if patch else body,'.json' if patch else '.c')
            record={'function':name,'target':target,'round':round_id,'variant':Path(path).stem,
              'parent':metadata.get('parent'),'family':metadata.get('family','unspecified'),
              'description':metadata.get('description',Path(path).name),'prediction':metadata.get('prediction'),
              'falsifier':metadata.get('falsifier'),'candidate_kind':'tu_patch' if patch else 'body','candidate_sha256':candidate_id,'environment':env_id}
            try:
                with materialized(target,name,body,source_override=complete) as root:
                    report=fresh(target,root/'output',root)
                    row=next(r for r in report['functions'] if r['name']==name)
                    response=compiler_response(row,report,original,original_ins,baseline)
                    record.update(response=response,effective_output_id=response.get('effective_output_id'),outcome='observed')
                    record['new_effective_output']=record['effective_output_id'] not in observed
                    observed.add(record['effective_output_id'])
            except RuntimeError as error:
                record.update(outcome='compile_error',compiler_error=str(error)[-2000:])
            if canonical_inputs()!=inputs or proof_context()!=context:raise ValueError('Canonical inputs changed during scratch search')
            records.append(record)
            response=record.get('response',{})
            print(record['variant'],response.get('status',record['outcome']),response.get('effective_output_id','')[:16],
              'size',response.get('candidate_size'),'diff',response.get('differing_bytes'),'peer losses',response.get('peer_losses',[]),flush=True)
        productive=bool(insight) or any(r.get('new_effective_output') or r.get('response',{}).get('peer_gains') for r in records)
        # Stagnation is a reasoning-round observation, never a compile limit.
        last_rounds={r['round']:r.get('productive_round',False) for r in prior}
        stagnant=0
        for _,value in sorted(last_rounds.items(),reverse=True):
            if value:break
            stagnant+=1
        stagnant=0 if productive else stagnant+1
        for r in records:r.update(productive_round=productive,insight=insight,stagnant_rounds=stagnant)
        previous=log.read_bytes() if log.exists() else b''
        atomic_bytes(log,previous+b''.join((json.dumps(r,separators=(',',':'))+'\n').encode() for r in records))
    result={'round':round_id,'variants':len(records),'effective_groups':len({r.get('effective_output_id') for r in records if r.get('effective_output_id')}),
       'pareto_variants':pareto(records),'productive':productive,'stagnant_rounds':stagnant,
       'guidance':'Consider a new hypothesis family or meta-analysis after demonstrated stagnation; no enforced stop.' if stagnant>=10 else 'Continue while experiments yield useful information.'}
    print(json.dumps(result,indent=2));return records,result

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('function');ap.add_argument('candidates',nargs='+',type=Path)
    ap.add_argument('--target');ap.add_argument('--patch',action='store_true',help='Exact replacement JSON patches for structural TU hypotheses');ap.add_argument('--meta',type=Path,help='Shared round,parent,family,description,prediction,falsifier JSON')
    ap.add_argument('--insight',help='New reasoning evidence even if compiler outputs repeat');a=ap.parse_args()
    paths=[]
    for p in a.candidates:paths.extend(sorted(p.glob('*.json' if a.patch else '*.c')) if p.is_dir() else [p])
    if not paths:ap.error('No candidate bodies found')
    search(a.function,paths,a.target,read_json(a.meta) if a.meta else None,a.insight,a.patch)
if __name__=='__main__':main()
