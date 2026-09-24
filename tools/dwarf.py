"""Lossless attribute graph from objdump DWARF2 text, including origin chains.

Offsets are absolute .debug_info offsets, never CU-relative aliases. Types
are intentionally not merged merely because two CUs use the same name.
"""
import re
from collections import Counter

def integer(raw):
    m = re.match(r'(0x[0-9a-fA-F]+|-?\d+)',raw or '')
    return int(m[1],0) if m else None

def reference(raw):
    m = re.search(r'<0x([0-9a-fA-F]+)>',raw or '')
    return int(m[1],16) if m else None

def name(raw):
    return re.sub(r'^\(indirect string, offset: (?:0x)?[0-9a-fA-F]+\):\s*','',raw) if raw else None

def parse(text):
    dies,stack = {},{}
    cu = current = None
    for line in text.splitlines():
        m = re.match(r' <(\d+)><([0-9a-fA-F]+)>: Abbrev Number: \d+(?: \((DW_TAG_\w+)\))?',line)
        if m:
            depth,offset,tag = int(m[1]),int(m[2],16),m[3]
            current = None
            if not tag: continue
            if tag=='DW_TAG_compile_unit':
                cu = offset
                stack = {}
            current = {'offset':offset,'tag':tag,'depth':depth,'cu':cu,
                       'parent':stack.get(depth-1),'children':[],'attrs':{}}
            dies[offset] = current
            if current['parent'] is not None:
                dies[current['parent']]['children'].append(offset)
            stack[depth] = offset
            continue
        m = re.match(r'\s+<[0-9a-fA-F]+>\s+(DW_AT_\w+)\s*:\s*(.*)$',line)
        if m and current is not None:
            current['attrs'][m[1]]=m[2].strip()
    unresolved = []
    cache = {}
    def resolve(offset,trail=()):
        if offset in cache: return cache[offset]
        if offset in trail: raise ValueError(f'DWARF reference cycle: {trail} -> {offset}')
        d = dies[offset]
        result = {}
        for key in ['DW_AT_specification','DW_AT_abstract_origin']:
            target = reference(d['attrs'].get(key))
            if target is not None:
                if target not in dies: unresolved.append({'die':offset,'attribute':key,'target':target})
                else: result.update(resolve(target,trail+(offset,)))
        result.update(d['attrs'])
        cache[offset]=result
        return result
    for offset,d in dies.items():
        a = resolve(offset)
        d['resolved'] = a
        d['name'] = name(a.get('DW_AT_name'))
        d['type_ref'] = reference(a.get('DW_AT_type'))
        d['low_pc'] = integer(a.get('DW_AT_low_pc'))
        d['high_pc'] = integer(a.get('DW_AT_high_pc'))
        loc = re.search(r'DW_OP_addr:\s*([0-9a-fA-F]+)',a.get('DW_AT_location',''))
        d['address'] = int(loc[1],16) if loc else None
    return dies, {'die_count':len(dies),'tags':dict(sorted(Counter(d['tag'] for d in dies.values()).items())),
                  'unresolved_origin_specification':unresolved}

def line_rows(text):
    result=[]
    context=None
    for line in text.splitlines():
        if line.endswith(':') and not line.startswith('Contents'):
            context=line[:-1]
        m=re.match(r'(.+?)\s+(\d+|-)\s+(0x[0-9a-fA-F]+)(.*)$',line)
        if m:
            result.append({'context':context,'file':m[1].strip(),'line':int(m[2]) if m[2]!='-' else None,
                           'address':int(m[3],16),'end_sequence':m[2]=='-','flags':m[4].strip()})
    return result

def file_tables(text):
    """DWARF2 line-program include directories and file indices."""
    result=[]
    table=None
    mode=None
    for line in text.splitlines():
        m=re.match(r'\s+Offset:\s+(0x[0-9a-fA-F]+|\d+)\s*$',line)
        if m:
            table={'offset':int(m[1],0),'directories':{},'files':{}}
            result.append(table)
            mode=None
        elif 'The Directory Table' in line: mode='directories'
        elif 'The File Name Table' in line: mode='files'
        elif 'Line Number Statements:' in line: mode=None
        elif table and mode=='directories':
            m=re.match(r'\s+(\d+)\s+(.+)$',line)
            if m: table['directories'][int(m[1])]=m[2].strip()
        elif table and mode=='files':
            m=re.match(r'\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(.+)$',line)
            if m:
                directory=table['directories'].get(int(m[2]),'')
                filename=m[5].strip()
                table['files'][int(m[1])]={'name':filename,'directory_index':int(m[2]),
                    'timestamp':int(m[3]),'size':int(m[4]),'path':directory+'/'+filename if directory else filename}
    return result

def address_lists(binary,section_name):
    """Preserve DWARF2 range/location lists and bytecode with exact offsets."""
    import struct
    section=next(s for s in binary.sections if s['name']==section_name)
    data=binary.section_bytes(section)[:section['virtual_size']]
    result=[]
    offset=0
    entries=[]
    start=0
    while offset+8<=len(data):
        begin,end=struct.unpack_from('<II',data,offset)
        row={'offset':offset,'begin':begin,'end':end}
        offset+=8
        if begin==0 and end==0:
            result.append({'offset':start,'entries':entries,'end_offset':offset})
            entries=[]
            start=offset
            continue
        if begin==0xffffffff:
            row['kind']='base_address_selection'
        else:
            row['kind']='range'
            if section_name=='.debug_loc':
                size=struct.unpack_from('<H',data,offset)[0]
                offset+=2
                row['expression_hex']=data[offset:offset+size].hex()
                offset+=size
        entries.append(row)
    if entries or offset!=len(data):
        raise ValueError(f'Incomplete {section_name} at {offset:x}')
    return result
