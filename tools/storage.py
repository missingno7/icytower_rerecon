"""Conservative storage regression proof for body promotions."""
import struct
from binary import Binary
from common import digest

def storage(obj_path,report):
    obj=Binary(obj_path);result={}
    for section in obj.sections:
        if section['name'] not in ('.data','.rdata','.bss'):continue
        secsym=next((s for s in obj.symbols if s['name']==section['name'] and s['aux_count']),None)
        logical=struct.unpack_from('<I',bytes.fromhex(secsym['aux_hex']))[0] if secsym else section['raw_size']
        code=bytearray(obj.section_bytes(section)[:logical]);relocations=[]
        for rel in obj.relocations:
            if rel['section']!=section['index']:continue
            offset=rel['offset'];sym=obj.by_index[rel['symbol_index']]
            if rel['type']!=6 or offset+4>len(code):
                relocations.append(['unsupported',rel]);continue
            addend=struct.unpack_from('<I',code,offset)[0];target=None
            if sym['name']=='.text':
                hits=[f for f in report['functions'] if f.get('candidate_offset',-1)<=addend<f.get('candidate_offset',-1)+f.get('candidate_size',0)]
                if len(hits)==1:target=['function',hits[0]['name'],addend-hits[0]['candidate_offset']]
            if target is None:target=[sym['name'],addend-(sym['value'] if sym['section']>0 else 0)]
            relocations.append([offset,rel['type'],target]);code[offset:offset+4]=b'\0'*4
        result[section['name']]={'logical_size':logical,'fingerprint':digest([logical,code.hex(),relocations])}
    return result

def protect_storage(before,after,before_obj,after_obj):
    a,b=storage(before_obj,before),storage(after_obj,after)
    for section in set(a)|set(b):
        if a.get(section)==b.get(section):continue
        if section=='.bss':
            # Only a change to a completely proven historical layout is acceptable.
            if not any(s['section']==section and s['layout_equal'] for s in after.get('bss_layout_comparison',[])):
                raise ValueError('BSS contribution changed; body-only promotion requires unchanged allocation or a newly proven complete layout')
            continue
        newly_proved=next((s for s in after['initialized_data_comparison'] if s['section']==section and s['content_equal']),None)
        if not newly_proved:raise ValueError('Unproved initialized data changed: '+section+'; independently prove the new contribution first')
