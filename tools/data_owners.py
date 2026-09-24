"""Independent DWARF/COFF object ownership; never consumes a tested code operand."""
import re
import struct
from type_graph import TypeGraph


def scope_identity(graph,die):
    parent=graph.dies.get(die.get('parent'),{})
    if parent.get('tag')=='DW_TAG_compile_unit': return ('GLOBAL',)
    if parent.get('tag')=='DW_TAG_subprogram' and parent.get('name'):
        return ('FUNCTION_STATIC',parent['name'])
    # Lexical-block statics need an independently established block identity.
    return None


def matching_symbols(binary,die):
    pattern=re.compile(r'^_'+re.escape(die['name'])+r'(?:\.\d+)?$')
    return [s for s in binary.symbols if s['section']>0 and s['storage_class'] in (2,3)
            and pattern.fullmatch(s['name']) and s.get('va',s['value'])==die['address']]


def initializer_bytes(obj,section,start,size,resolve_initializer):
    """Resolve from independent symbols/data only; never pass reference bytes to the resolver."""
    end=start+size; raw=obj.section_bytes(section)
    if end>len(raw): return None,[],'Initializer extent exceeds available bytes'
    content=bytearray(raw[start:end]); fields=[]; covered=set()
    for relocation in obj.relocations:
        if relocation['section']!=section['index']: continue
        offset=relocation['offset']; relative=offset-start
        if offset>=end or offset+4<=start: continue
        if relative<0 or relative+4>size: return None,fields,'Initializer relocation crosses object boundary'
        if relocation.get('type')!=6: return None,fields,'Unsupported initializer relocation type'
        if covered.intersection(range(relative,relative+4)): return None,fields,'Overlapping initializer relocations'
        covered.update(range(relative,relative+4))
        if resolve_initializer is None: return None,fields,'Initializer contains relocations; independent object graph proof needed'
        symbol=obj.by_index.get(relocation['symbol_index'])
        if symbol is None: return None,fields,'Initializer relocation symbol is absent'
        addend=struct.unpack_from('<I',content,relative)[0]
        target,reason=resolve_initializer(symbol,addend)
        fields.append({'object_offset':relative,'section_offset':offset,'type':6,'symbol':symbol['name'],
                       'addend':addend,'target_va':target,'resolution':reason})
        if target is None: return None,fields,'Initializer target not independently resolved'
        struct.pack_into('<I',content,relative,target&0xffffffff)
    return bytes(content),fields,None


def independent_owners(obj,exe,candidate_dies,original_dies,cu_path,resolve_initializer=None):
    cg=TypeGraph(candidate_dies.values()); og=TypeGraph(original_dies.values())
    sections={s['index']:s for s in obj.sections}; original_sections={s['index']:s for s in exe.sections}
    original={}; accepted=[]; rejected=[]
    for d in original_dies.values():
        if d['tag']!='DW_TAG_variable' or d.get('address') is None or not d.get('name'): continue
        if original_dies.get(d['cu'],{}).get('name')!=cu_path: continue
        scope=scope_identity(og,d)
        if scope: original.setdefault((scope,d['name']),[]).append(d)
    for d in candidate_dies.values():
        if d['tag']!='DW_TAG_variable' or d.get('address') is None or not d.get('name'): continue
        scope=scope_identity(cg,d)
        if not scope: continue
        olds=original.get((scope,d['name']),[])
        syms=matching_symbols(obj,d)
        if not olds or not syms: continue
        detail={'name':d['name'],'scope':scope,'candidate_die':d['offset']}
        reason=None
        if len(olds)!=1 or len(syms)!=1: reason='Ambiguous DWARF identity or candidate COFF owner'
        else:
            old=olds[0]; sym=syms[0]; old_syms=matching_symbols(exe,old)
            size=cg.size(d.get('type_ref'))
            detail.update(candidate_offset=sym['value'],section_index=sym['section'],section=sections[sym['section']]['name'],size=size,original_va=old['address'],original_die=old['offset'],dwarf_type=og.declaration(old.get('type_ref')),candidate_type=cg.declaration(d.get('type_ref')))
            if len(old_syms)!=1: reason='Historical COFF owner is absent or ambiguous'
            elif not size or size!=og.size(old.get('type_ref')) or cg.signature(d.get('type_ref'))!=og.signature(old.get('type_ref')):
                reason='DWARF type/size/member layout differs or is incomplete'
            else:
                section=sections[sym['section']]; old_section=original_sections[old_syms[0]['section']]
                start=sym['value']; end=start+size
                detail.update(section=section['name'],section_index=sym['section'],candidate_offset=start,size=size,
                              original_va=old['address'],original_die=old['offset'],candidate_coff=sym['index'],original_coff=old_syms[0]['index'])
                if section['name'] not in ('.data','.rdata','.bss') or section['name']!=old_section['name']:
                    reason='Storage section differs or is unsupported'
                elif end>max(section['raw_size'],section['virtual_size']) or old['address']+size>exe.image_base+old_section['rva']+old_section['virtual_size']:
                    reason='Object extent exceeds its section'
                elif old_syms[0]['storage_class']!=sym['storage_class']: reason='Storage linkage differs'
                elif any(s['section']==sym['section'] and start<=s['value']<end and s['index']!=sym['index'] and not s['name'].startswith('.') for s in obj.symbols):
                    reason='Overlapping candidate COFF owner'
                elif any(s['section']==old_syms[0]['section'] and old['address']<=s.get('va',-1)<old['address']+size and s['index']!=old_syms[0]['index'] and not s['name'].startswith('.') for s in exe.symbols):
                    reason='Overlapping historical COFF owner'
                else:
                    if section['name']=='.bss':
                        if any(r['section']==sym['section'] and r['offset']<end and r['offset']+4>start for r in obj.relocations):
                            reason='BSS object unexpectedly contains a relocation'
                    else:
                        content,fields,reason=initializer_bytes(obj,section,start,size,resolve_initializer)
                        detail['initializer_relocations']=fields
                        if reason is None and content!=exe.at_va(old['address'],size): reason='Initialized object bytes differ after independent relocation resolution'
                    if reason is None:
                        detail.update(proof='Unique CU/scope/name, identical DWARF type graph, COFF owner, storage and complete independently resolved initializer',
                                      initializer='zero-initialized BSS' if section['name']=='.bss' else 'all initialized bytes equal after independent relocation resolution',initial_value=None if section['name']=='.bss' else exe.at_va(old['address'],min(size,64)).hex())
                        accepted.append(detail)
        if reason: rejected.append(dict(detail,reason=reason))
    return {'accepted':accepted,'rejected':rejected,
            'limit':'Object-relative relocation ownership only. Never whole-section, object-file or CU equality.'}


def resolve_owner(owners,section,addend):
    hits=[o for o in owners['accepted'] if o['section_index']==section and o['candidate_offset']<=addend<o['candidate_offset']+o['size']]
    if len(hits)!=1: return None
    owner=hits[0]
    return owner['original_va']+addend-owner['candidate_offset']
