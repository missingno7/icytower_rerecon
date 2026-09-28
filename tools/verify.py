"""Fresh whole-TU proof, compact state, and exact-peer protection."""
import argparse, collections, json
from pathlib import Path
from common import ROOT, BUILD, ORACLE, ANALYSIS, identity, read_json, write_json, digest, sha
from build import targets, compile_target
from compare import compare
from source_scope import body_hash

STATE = ROOT / 'recovery.json'
def canonical_inputs(root=ROOT):
    # One snapshot of the active C/headers; generated objects never establish source identity.
    return {p.relative_to(root).as_posix():identity(p) for folder in ['src','include']
            for p in sorted((Path(root)/folder).rglob('*')) if p.is_file()}
def proof_context(overrides=None):
    # overrides: {relative path: bytes} for a configuration file about to be published atomically.
    proof_modules='common build binary dwarf compare data_owners type_graph control_transfers instructions evidence verify link source_scope scratch promote transaction storage test equivalence/objfun equivalence/bisim equivalence/certify'.split()
    files=[*sorted((ROOT/'tests').glob('*.py')),*[ROOT/'tools'/(name+'.py') for name in proof_modules],
           *sorted((ROOT/'evidence').glob('*.json')),*sorted((ROOT/'toolchain').glob('*.json')),*sorted((ROOT/'third_party').glob('*.json'))]
    context={p.relative_to(ROOT).as_posix():identity(p) for p in files}
    for rel,data in (overrides or {}).items():
        if rel not in context:raise ValueError('Unknown proof configuration file: '+rel)
        context[rel]={'size':len(data),'sha256':sha(data)}
    return digest(context)
def fresh(target,dest=None,source_root=ROOT,config=None):
    config=config or targets()[target]
    obj,compilation=compile_target(target,dest,source_root,config=config)
    result=compare(obj,config['historical_cu'],ORACLE,ANALYSIS)
    if identity(obj)!=compilation['object']:raise ValueError('Object changed during verification')
    result['build']=compilation
    text=(Path(source_root)/config['source']).read_bytes().decode('cp1252')
    for f in result['functions']:
        if f['status']!='MISSING':
            if f['name']=='WinMain' and target=='game-main' and 'END_OF_MAIN()' in text:
                f['body_sha256']=None  # Historical Allegro macro; source/header inputs are pinned as a whole.
            else: f['body_sha256']=body_hash(text,f['name'])
    write_json(obj.parent/'proof.json',result)
    return result

def exact_set(report):return {f['name'] for f in report['functions'] if f['status']=='FUNCTION_MATCH'}
def newly_proved_data(before,after):
    """Initialized-data contributions unproved before and content-equal after."""
    was={s['section'] for s in before['initialized_data_comparison'] if s['content_equal']}
    return {s['section'] for s in after['initialized_data_comparison'] if s['content_equal'] and s['section'] not in was}
def proved_bss(report):
    """Zero-initialized contributions whose complete historical layout is proven."""
    return {s['section'] for s in report.get('bss_layout_comparison',[]) if s['layout_equal']}
def newly_proved_bss(before,after):return proved_bss(after)-proved_bss(before)
def commons_moved_to_proven_owners(before,after):
    # A TU storage promotion may turn common (tentative) definitions into definitions, but
    # only when every removed common now has an independently accepted global owner inside
    # a content-equal initialized contribution or a completely proven BSS layout. Nothing
    # may be added to, or resized in, the common set.
    def common(r):return {(s['name'],s['value'],s['storage_class']) for s in r['common_allocations']}
    a,b=common(before),common(after)
    if not b<=a:return False
    proved={s['section'] for s in after['initialized_data_comparison'] if s['content_equal']}|proved_bss(after)
    owners={o['name']:o for o in after['object_ownership']['accepted'] if tuple(o['scope'])==('GLOBAL',)}
    for name,_,_ in a-b:
        owner=owners.get(name[1:] if name.startswith('_') else name)
        if not owner or owner['section'] not in proved:return False
    return True
def protect(before,after):
    old={f['name']:f for f in before['functions']};new={f['name']:f for f in after['functions']}
    if set(old)!=set(new):raise ValueError('Historical function inventory changed')
    lost=exact_set(before)-exact_set(after)
    if lost:raise ValueError('Exact peers regressed: '+', '.join(sorted(lost)))
    # Exact peer source is frozen, except in a TU storage promotion that newly proves a
    # complete initialized-data contribution: literal pools follow each function's expansion
    # order, so that proof may require an exact peer to keep its code while changing source.
    # The peer must still be FUNCTION_MATCH (checked above).
    data_promotion=bool(newly_proved_data(before,after))
    # Likewise a promotion that newly proves the TU's complete text contribution (function
    # order and padding included) may need exact peers to move or be re-spelled; each must
    # still be FUNCTION_MATCH.
    text_promotion=not before.get('whole_text_contribution_equal') and bool(after.get('whole_text_contribution_equal'))
    # A newly proven BSS layout never needs an exact peer to change source.
    storage_promotion=data_promotion or bool(newly_proved_bss(before,after))
    for name in exact_set(before):
        if old[name].get('body_sha256')!=new[name].get('body_sha256') and not (data_promotion or text_promotion):
            raise ValueError('Protected exact peer body changed: '+name)
    # Protect each complete previously established data/BSS owner and contribution.
    def owners(r):
        return {(tuple(o['scope']),o['name']):(o['size'],o['section'],o['original_va'],o['dwarf_type'])
                for o in r['object_ownership']['accepted']}
    a,b=owners(before),owners(after)
    for key,value in a.items():
        if b.get(key)!=value:raise ValueError('Proven data/BSS owner regressed: '+str(key))
    for section in before['initialized_data_comparison']:
        if section['content_equal']:
            current=next((s for s in after['initialized_data_comparison'] if s['section']==section['section']),None)
            if not current or not current['content_equal'] or current['logical_size']!=section['logical_size']:
                raise ValueError('Proven data contribution regressed: '+section['section'])
    for section in before.get('bss_layout_comparison',[]):
        if section['layout_equal']:
            current=next((s for s in after.get('bss_layout_comparison',[]) if s['section']==section['section']),None)
            if not current or not current['layout_equal'] or current['logical_size']!=section['logical_size']:
                raise ValueError('Proven BSS layout regressed: '+section['section'])
    # BSS/common ownership and size cannot change in a function-body promotion.
    def common(r):return sorted((s['name'],s['value'],s['storage_class']) for s in r['common_allocations'])
    if common(before)!=common(after) and not (storage_promotion and commons_moved_to_proven_owners(before,after)):
        raise ValueError('Common/BSS allocations changed')

def carry_equivalence(prior_units,units):
    """Equivalence-tier records survive a refresh only while the accepted body is unchanged and the
    fresh proof still says DIFFER; a fresh FUNCTION_MATCH upgrades the record, anything else is refused.
    EQUIVALENT is never granted here, only by promote.py --equivalent with a bound certificate."""
    for target,unit in units.items():
        old=(prior_units or {}).get(target,{}).get('functions',{})
        for name,rec in unit['functions'].items():
            was=old.get(name)
            if not was or was.get('status')!='EQUIVALENT':continue
            if rec['status']=='FUNCTION_MATCH':continue
            if rec['status']!='DIFFER' or rec.get('body_sha256')!=was.get('body_sha256'):
                raise ValueError('Equivalent function changed outside promotion: '+target+'::'+name)
            unit['functions'][name]=dict(was)
    return units
def state_from(reports,link,source_root=ROOT,prior=None):
    units={target:{'functions':{f['name']:{'status':f['status'],'body_sha256':f.get('body_sha256')} for f in r['functions']},
                   'whole_text_equal':r['whole_text_contribution_equal'],'object_match':False,'cu_match':False}
           for target,r in reports.items()}
    return {'schema':1,'inputs':canonical_inputs(source_root),'proof_context':proof_context(),
      'units':carry_equivalence(prior['units'] if prior else None,units),'link':link}
def load_state(check=True):
    from transaction import ensure_consistent
    ensure_consistent()
    s=read_json(STATE)
    if check and s['inputs']!=canonical_inputs():raise ValueError('Canonical sources differ from accepted state; use scratch overlays')
    if check and s['proof_context']!=proof_context():raise ValueError('Proof tools/configuration changed; run verify.py --all --refresh')
    return s

def summary(reports):return dict(collections.Counter(f['status'] for r in reports.values() for f in r['functions']))
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('targets',nargs='*');ap.add_argument('--all',action='store_true')
    ap.add_argument('--initialize',action='store_true',help='Fresh migration proof against build/migration-expected.json; refuses an existing state')
    ap.add_argument('--refresh',action='store_true',help='Re-certify tool changes, preserving all accepted exact functions')
    a=ap.parse_args();chosen=list(targets()) if a.all else a.targets
    if not chosen:ap.error('Choose targets or --all')
    if (a.initialize or a.refresh) and not a.all:ap.error('State publication requires --all')
    if a.initialize and STATE.exists():raise ValueError('Recovery state already exists')
    old=read_json(STATE) if STATE.exists() else None
    if a.refresh and not old:raise ValueError('No accepted state to refresh')
    if old and old['inputs']!=canonical_inputs():raise ValueError('Canonical source changed outside promotion')
    before=canonical_inputs();context=proof_context();reports={}
    for target in chosen:
        report=fresh(target,BUILD/'verify'/target);reports[target]=report
        print(target,dict(collections.Counter(f['status'] for f in report['functions'])),flush=True)
        if old:
            proven={n for n,f in old['units'][target]['functions'].items() if f['status']=='FUNCTION_MATCH'}
            if proven-exact_set(report):raise ValueError('Accepted exact functions regressed: '+str(proven-exact_set(report)))
    print('Fresh totals:',summary(reports),flush=True)
    if a.initialize:
        expected=read_json(BUILD/'migration-expected.json')
        for target,r in reports.items():
            prior=expected[targets()[target]['source']];current={f['name']:f['status'] for f in r['functions']}
            if set(prior)!=set(current):raise ValueError('Historical function topology differs: '+target)
            missing={n for n,v in prior.items() if v=='FUNCTION_MATCH'}-exact_set(r)
            if missing:raise ValueError('Old exact proof not reproduced: '+target+' '+str(missing))
    if a.initialize or a.refresh:
        from link import ordinary_link
        link=ordinary_link(BUILD/'verify/link',objects={t:BUILD/'verify'/t/'unit.o' for t in chosen})
        if old:
            from link import protect_link
            protect_link(old['link'],link)
        if before!=canonical_inputs() or context!=proof_context():raise ValueError('Inputs changed during verification')
        write_json(STATE,state_from(reports,link,prior=old))
        print('Published freshly verified recovery.json')
    write_json(BUILD/'verification-summary.json',{'counts':summary(reports),'targets':chosen})
if __name__=='__main__':main()
