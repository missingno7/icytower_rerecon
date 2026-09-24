"""Multidimensional search diagnostics; normalization never grants a match."""
import difflib, re
from common import digest
from control_transfers import relative

def raw_code(function):return bytearray.fromhex(''.join(i['bytes'] for i in function['instructions']))
def effective_identity(function):
    code=raw_code(function)
    if len(code)!=function['candidate_size']:raise ValueError('Incomplete decoded candidate')
    relocations=[];transfers=[]
    for r in function['relocations']:
        offset=r['function_offset'];raw=code[offset:offset+4].hex()
        if r.get('target_va') is not None:
            target=['resolved',r['target_va']]
        else:target=['unresolved',r['symbol'],r['addend'],raw]
        relocations.append([offset,r['type'],target]);code[offset:offset+4]=b'\0'*4
    for r in function['direct_transfers']:
        p=r['function_offset'];width=r['operand_size']
        if r.get('target_function') is not None and r.get('target_va') is not None:
            target=[r['target_function'],r['target_va']]
        else:target=[r.get('candidate_target'),bytes(code[p:p+width]).hex()]
        transfers.append([p,width,r['transfer_kind'],target]);code[p:p+width]=b'\0'*width
    return digest({'code':code.hex(),'relocations':relocations,'transfers':transfers})

def islands(offsets):
    result=[]
    for i in offsets:
        if result and i==result[-1][1]:result[-1][1]+=1
        else:result.append([i,i+1])
    return result

def compiler_response(function,report,original,original_instructions,baseline_exact=()):
    if function['status']=='MISSING':return {'status':'MISSING'}
    code=raw_code(function)
    for r in function['relocations']+function['direct_transfers']:
        if r.get('resolved_value') is not None:
            p=r['function_offset'];w=r.get('operand_size',4);code[p:p+w]=r['resolved_value'].to_bytes(w,'little')
    prefix=0
    for a,b in zip(code,original):
        if a!=b:break
        prefix+=1
    suffix=0
    for a,b in zip(reversed(code),reversed(original)):
        if a!=b:break
        suffix+=1
    ins=function['instructions'];frame=None;branches=[];calls=[]
    for i in ins:
        m=re.fullmatch(r'sub\s+\$0x([0-9a-f]+),%esp',i['assembly'])
        if m and i in ins[:12]:frame=int(m[1],16)
        rel=relative(i)
        if i['mnemonic'].startswith(('j','loop')):
            branches.append([i['address']-function['candidate_offset'],i['mnemonic'],rel['target']-function['candidate_offset'] if rel else 'indirect'])
        if i['mnemonic'].startswith('call'):
            off=i['address']-function['candidate_offset']
            field=next((r for r in function['relocations'] if off<=r['function_offset']<off+len(bytes.fromhex(i['bytes']))),None)
            transfer=next((t for t in function['direct_transfers'] if t['instruction_offset']==off),None)
            calls.append(field['symbol'] if field else transfer['target_function'] if transfer else i['assembly'])
    differences=list(function['difference_offsets'])+list(range(min(len(code),len(original)),max(len(code),len(original))))
    regions=islands(differences)
    exact={r['name'] for r in report['functions'] if r['status']=='FUNCTION_MATCH'}
    old=[i['mnemonic'] for i in original_instructions];new=[i['mnemonic'] for i in ins]
    edits=[list(op) for op in difflib.SequenceMatcher(a=old,b=new,autojunk=False).get_opcodes() if op[0]!='equal']
    return {'status':function['status'],'strict_exact':function['status']=='FUNCTION_MATCH',
       'candidate_size':len(code),'original_size':len(original),'differing_bytes':len(differences),
       'exact_prefix':prefix,'exact_suffix':suffix,'first_mismatch':function['first_difference'],
       'mismatch_island_count':len(regions),'mismatch_islands':regions[:32],'frame_allocation':frame,
       'instruction_order_changes':edits[:24],'instruction_order_change_count':len(edits),
       'branch_count':len(branches),'branch_shape':branches[:64],'call_count':len(calls),'call_targets':calls,
       'relocation_count':len(function['relocations']),'unproved_relocations':sum(not r['equal'] for r in function['relocations']),
       'unproved_transfers':sum(not r['equal'] for r in function['direct_transfers']),
       'relocation_targets':[[r['symbol'],r.get('target_va'),r['equal']] for r in function['relocations']],
       'peer_gains':sorted(exact-set(baseline_exact)),'peer_losses':sorted(set(baseline_exact)-exact),
       'data_contributions':[{k:s[k] for k in ['section','logical_size','content_equal']} for s in report['initialized_data_comparison']],
       'accepted_data_owners':[[o['name'],o['size'],o['section']] for o in report['object_ownership']['accepted']],
       'effective_output_id':effective_identity(function)}

def pareto(records):
    valid=[r for r in records if 'candidate_size' in r.get('response',{})]
    def costs(r):
        s=r['response'];return (not s['strict_exact'],len(s['peer_losses']),s['unproved_relocations']+s['unproved_transfers'],abs(s['candidate_size']-s['original_size']),s['differing_bytes'],s['instruction_order_change_count'])
    return [r['variant'] for r in valid if not any(all(x<=y for x,y in zip(costs(q),costs(r))) and costs(q)!=costs(r) for q in valid)]
