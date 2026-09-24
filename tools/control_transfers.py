"""Decoded same-CU relative transfers and narrowly proved terminal-jump relaxation."""
import struct
import re
from collections import defaultdict
from common import sha


def relative(instruction):
    raw=bytes.fromhex(instruction['bytes']); n=len(raw)
    if n==5 and raw[0] in (0xe8,0xe9): offset,width,kind=1,4,'call' if raw[0]==0xe8 else 'jmp'
    elif n==2 and raw[0]==0xeb: offset,width,kind=1,1,'jmp'
    elif n==2 and (0x70<=raw[0]<=0x7f or 0xe0<=raw[0]<=0xe3): offset,width,kind=1,1,'conditional'
    elif n==6 and raw[0]==0x0f and 0x80<=raw[1]<=0x8f: offset,width,kind=2,4,'conditional'
    else: return None
    displacement=int.from_bytes(raw[offset:offset+width],'little',signed=True)
    return {'operand_offset':offset,'operand_size':width,'kind':kind,'length':n,'displacement':displacement,
            'target':(instruction['address']+n+displacement)&0xffffffff}


def groups(functions,key):
    result=defaultdict(list)
    for f in functions: result[f[key]].append(f)
    return result


def resolve(code,reference,low,high,original_va,instructions,candidates,originals,relocations):
    entries=groups(candidates,'low_pc'); old_names=groups(originals,'name'); records=[]
    for instruction in instructions:
        op=instruction['address']-low
        if not 0<=op<len(code): continue
        branch=relative(instruction)
        if branch is None:
            assembly=re.sub(r'^(?:(?:data16|addr16|repz|bnd|cs|ds|es|fs|gs|ss)\s+)+','',instruction['assembly'])
            mnemonic=assembly.split()[0] if assembly else ''
            if '*' not in assembly and mnemonic.startswith(('j','call','loop')):
                records.append({'instruction_offset':op,'function_offset':op,'operand_size':0,'opcode':bytes.fromhex(instruction['bytes'])[0],
                                'transfer_kind':'unsupported','target_function':None,'target_va':None,'candidate_target':None,'candidate_displacement':None,
                                'layout_operand_equal':False,'resolved_value':None,'original_value':None,'equal':False,'resolution':'Unsupported decoded direct-transfer encoding'})
            continue
        p=op+branch['operand_offset']; width=branch['operand_size']
        if p+width>len(code):
            records.append({'instruction_offset':op,'function_offset':p,'operand_size':width,'opcode':bytes.fromhex(instruction['bytes'])[0],
                            'transfer_kind':branch['kind'],'target_function':None,'target_va':None,'candidate_target':branch['target'],'candidate_displacement':branch['displacement'],
                            'layout_operand_equal':False,'resolved_value':None,'original_value':None,'equal':False,'resolution':'Direct transfer crosses the DWARF function extent'})
            continue
        overlaps=[r for r in relocations if p<r['function_offset']+4 and r['function_offset']<p+width]
        if overlaps and len(overlaps)==1 and overlaps[0]['function_offset']==p and width==4: continue
        hits=entries.get(branch['target'],[])
        # Internal basic-block edges retain exact raw displacement checking.
        if low<branch['target']<high and not hits and not overlaps: continue
        expected=None; target=None; name=None; reason='Missing or ambiguous same-CU function entry'
        if len(hits)==1 and len(old_names.get(hits[0]['name'],[]))==1 and not overlaps:
            name=hits[0]['name']; target=old_names[name][0]['va']
            delta=target-(original_va+op+branch['length'])
            if width==4 or -128<=delta<=127:
                expected=delta&((1<<(8*width))-1); reason='Unique candidate and historical same-CU function entry'
            else: reason='Historical target is outside the candidate short-branch range'
        elif overlaps: reason='Branch operand overlaps an unsupported relocation field'
        actual=int.from_bytes(reference[p:p+width],'little') if p+width<=len(reference) else None
        if expected is not None: code[p:p+width]=expected.to_bytes(width,'little')
        records.append({'instruction_offset':op,'function_offset':p,'opcode':bytes.fromhex(instruction['bytes'])[0],
                        'operand_size':width,'transfer_kind':branch['kind'],'target_function':name,'target_va':target,
                        'candidate_target':branch['target'],'candidate_displacement':branch['displacement'],
                        'layout_operand_equal':(branch['displacement']&((1<<(8*width))-1))==actual,
                        'resolved_value':expected,'original_value':actual,'equal':expected is not None and expected==actual,'resolution':reason})
    return records


def complete_stream(instructions,base,size):
    result=bytearray(); end=base
    for i in instructions:
        raw=bytes.fromhex(i['bytes'])
        if i['address']!=end or not raw: return None
        result.extend(raw); end+=len(raw)
    return bytes(result) if len(result)==size else None


def tail_layout(row,original_instructions,candidates,originals,reference=None):
    """Prove exact prefix plus the same final JMP when short range forces a width change.

    This never yields FUNCTION_MATCH. No general opcode normalization, padding,
    register permutation, relaxed relocation or source equivalence is admitted.
    """
    current=row.get('instructions',[]); size=row.get('candidate_size')
    if not current or not original_instructions or not row.get('instruction_boundaries_verified') or abs((size or 0)-row['original_size'])!=3: return None
    old=complete_stream(original_instructions,row['va'],row['original_size'])
    raw=complete_stream(current,row['candidate_offset'],size)
    if old is None or raw is None or (reference is not None and old!=reference): return None
    a=relative(original_instructions[-1]); b=relative(current[-1])
    if not a or not b or a['kind']!='jmp' or b['kind']!='jmp' or {a['length'],b['length']}!={2,5}: return None
    prefix=len(old)-a['length']
    if prefix!=len(raw)-b['length']: return None
    before=groups(originals,'va').get(a['target'],[]); after=groups(candidates,'low_pc').get(b['target'],[])
    if len(before)!=1 or len(after)!=1 or before[0]['name']!=after[0]['name']: return None
    # Test the hypothetical SHORT encoding at the long branch site. Using its
    # current rel32 displacement here would falsely prove near-range self loops.
    long_branch=a if a['length']==5 else b
    long_site=original_instructions[-1]['address'] if a['length']==5 else current[-1]['address']
    short_delta=long_branch['target']-(long_site+2)
    if -128<=short_delta<=127: return None
    code=bytearray(raw); occupied=set()
    for field in row.get('relocations',[])+row.get('direct_transfers',[]):
        offset=field['function_offset']; width=field.get('operand_size',4)
        if offset>=prefix:
            if field in row.get('relocations',[]): return None
            if field.get('instruction_offset')!=prefix: return None
            continue
        if width not in (1,4) or offset<0 or offset+width>prefix or not field.get('equal') or field.get('resolved_value') is None: return None
        positions=set(range(offset,offset+width))
        if occupied&positions: return None
        occupied.update(positions); code[offset:offset+width]=field['resolved_value'].to_bytes(width,'little')
    if bytes(code[:prefix])!=old[:prefix]: return None
    return {'schema':1,'proof':'EXACT_PREFIX_AND_IDENTICAL_TERMINAL_TRANSFER','prefix_size':prefix,
            'prefix_sha256':sha(old[:prefix]),'target_function':before[0]['name'],'historical_target_va':a['target'],
            'candidate_target_offset':b['target'],'original_instruction':original_instructions[-1],'candidate_instruction':current[-1],
            'original_encoding_bytes':a['length'],'candidate_encoding_bytes':b['length'],'long_site_short_displacement':short_delta,
            'limit':'Only terminal same-CU JMP width differs because short range is insufficient in one layout. Raw function bytes/sizes differ; no FUNCTION_MATCH, OBJECT_MATCH or CU_MATCH.'}
