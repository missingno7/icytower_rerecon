"""Narrow function context; request large evidence explicitly."""
import argparse, bisect, json
from common import ROOT, BUILD, ORACLE, ANALYSIS, read_json, write_json, identity, digest
from evidence import Evidence
from scratch import resolve_function
from source_scope import function_span
from type_graph import TypeGraph
from instructions import decode
from control_transfers import relative

def packet(name,target=None):
    from transaction import ensure_consistent
    ensure_consistent()
    target,config,f=resolve_function(name,target);db=Evidence();dies=db.dies(config['historical_cu']);g=TypeGraph(dies.values())
    d=dies[f['die']];state=read_json(ROOT/'recovery.json');accepted=state['units'][target]['functions'][f['name']]
    source=ROOT/config['source'];text=source.read_bytes().decode('cp1252')
    try:
        lo,hi=function_span(text,f['name']);scope={'first_line':text[:lo].count('\n')+1,'last_line':text[:hi].count('\n')+1}
    except ValueError:scope={'macro_defined':True}
    from verify import canonical_inputs,proof_context
    result={'function':f['name'],'target':target,'source':config['source'],'source_identity':identity(source),'source_scope':scope,
       'historical_cu':config['historical_cu'],'va':hex(f['va']),'size':f['size'],'die':f['die'],
       'accepted_status':accepted['status'],'accepted_state_current':state['inputs']==canonical_inputs() and state['proof_context']==proof_context(),
       'declaration':g.declaration(d.get('type_ref'),f['name'])+'('+', '.join(g.declaration(dies[p]['type_ref'],dies[p]['name'] or '') for p in f['parameters'])+')',
       'compiler':'TDM-GCC 4.4.1-tdm-2','flags':[config['default'],*config.get('flags',[]),'-g','-mfpmath=387','-DALLEGRO_STATICLINK'],
       'local_count':sum(x['tag']=='DW_TAG_variable' for x in g.descendants(f['die'])),
       'provenance_observations':[o for o in read_json(ROOT/'evidence/observations.json')['observations'] if o['source']==config['source'] and o['function']==f['name']]}
    # A cached mismatch is displayed only when its source and compiler inputs agree.
    report_path=BUILD/'verify'/target/'proof.json'
    if report_path.exists():
        report=read_json(report_path)
        if report['build']['inputs'].get(config['source'])==identity(source):
            row=next(x for x in report['functions'] if x['name']==f['name'])
            result['last_verification']={k:row.get(k) for k in ['status','candidate_size','first_difference']}
    db.close();return result

def callers(db,function):
    path=BUILD/'callers.json';key=digest({'oracle':identity(ORACLE),'analysis':identity(ANALYSIS),'code':identity(ROOT/'tools/context.py')})
    if path.exists() and read_json(path).get('key')==key:return read_json(path)['calls'].get(str(function['va']),[])
    functions=db.functions();vas=[f['va'] for f in functions];calls={}
    for ins in decode(ORACLE,ANALYSIS):
        edge=relative(ins)
        if not edge or edge['kind']!='call':continue
        i=bisect.bisect_right(vas,ins['address'])-1
        if i<0:continue
        owner=functions[i]
        if ins['address']>=owner['va']+owner['size']:continue
        calls.setdefault(str(edge['target']),[]).append({'caller':owner['name'],'cu':owner['compile_unit'],'call_va':hex(ins['address'])})
    write_json(path,{'key':key,'calls':calls});return calls.get(str(function['va']),[])

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('function');ap.add_argument('--target')
    for flag in ['locals','asm','callers','history','lines','body','globals','raw']:ap.add_argument('--'+flag,action='store_true')
    a=ap.parse_args();target,c,f=resolve_function(a.function,a.target);result=packet(f['name'],target);db=Evidence()
    if a.locals or a.raw:
        dies=db.dies(c['historical_cu']);g=TypeGraph(dies.values());desc=list(g.descendants(f['die']))
        if a.raw:result['dwarf']=[dies[f['die']],*desc]
        if a.locals:
            variables=[]
            for d in desc:
                if d['tag'] not in ['DW_TAG_variable','DW_TAG_formal_parameter']:continue
                v=g.variable(d)
                # Original location bytecode/ranges remain available with their exact offsets.
                loc=d['resolved'].get('DW_AT_location','')
                if 'location list' in loc:
                    from dwarf import integer
                    v['location_list']=db.address_list('locations',integer(loc))
                variables.append(v)
            result['locals']=variables
    if a.globals:result['globals']=db.globals(c['historical_cu'])
    if a.lines:result['lines']=db.lines(f['va'],f['va']+f['size'])
    if a.callers:
        result['callers']=callers(db,f)
        result['callers_scope']='Decoded direct calls in DWARF function extents; indirect and COFF-only scopes are not assigned.'
    if a.asm:result['assembly']=decode(ORACLE,ANALYSIS,f['va'],f['va']+f['size'])
    if a.body:
        text=(ROOT/c['source']).read_bytes().decode('cp1252');lo,hi=function_span(text,f['name']);result['body']=text[lo:hi]
    if a.history:
        from search import history
        rows=history(target,f['name']);groups={}
        for r in rows:
            key=r.get('effective_output_id',r.get('outcome'));group=groups.setdefault(key,{'variants':[],'rounds':[]})
            group['variants'].append(r['variant']);group['rounds'].append(r['round']);group['latest_response']=r.get('response')
        result['history']={'experiments':len(rows),'groups':groups,'archive_hint':'Older experiments remain in ../icytower_recon; query selectively.'}
    print(json.dumps(result,indent=2));db.close()
if __name__=='__main__':main()
