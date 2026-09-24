"""Lossless DWARF type traversal. No game layouts are encoded here."""
import json
import re
from collections import defaultdict
from functools import lru_cache
from common import ROOT


def number(value):
    if value is None:
        return None
    match = re.match(r'(-?0x[0-9a-fA-F]+|-?\d+)', str(value))
    return int(match[0], 0) if match else None


class TypeGraph:
    def __init__(self, rows=None):
        if rows is None:
            raise ValueError('Provide an explicit CU graph from Evidence.dies; no global graph reads')
        self.dies = {r['offset']: r for r in rows}
        self.children = defaultdict(list)
        for row in self.dies.values():
            self.children[row.get('parent')].append(row)
        self.game_types = defaultdict(list)
        for row in self.dies.values():
            if row['tag'] == 'DW_TAG_typedef' and '/icytower/trunk/source/' in (row.get('decl_file_path') or ''):
                self.game_types[row['name']].append(row)

    def descendants(self, offset):
        for child in self.children[offset]:
            yield child
            yield from self.descendants(child['offset'])

    def attrs(self, offset):
        return self.dies[offset].get('resolved', {})

    def size(self, offset):
        if offset is None:
            return None
        d = self.dies[offset]
        size = number(self.attrs(offset).get('DW_AT_byte_size'))
        if size is not None:
            return size
        if d['tag'] == 'DW_TAG_array_type':
            size = self.size(d['type_ref'])
            for count in self.dimensions(offset):
                if count is None or size is None:
                    return None
                size *= count
            return size
        if d['tag'] in ('DW_TAG_typedef', 'DW_TAG_const_type', 'DW_TAG_volatile_type'):
            return self.size(d['type_ref'])
        return None

    def dimensions(self, offset):
        result = []
        for r in self.children[offset]:
            if r['tag'] != 'DW_TAG_subrange_type':
                continue
            a = r['resolved']
            count = number(a.get('DW_AT_count'))
            upper = number(a.get('DW_AT_upper_bound'))
            lower = number(a.get('DW_AT_lower_bound')) or 0
            result.append(count if count is not None else upper - lower + 1 if upper is not None else None)
        return result

    def member_offset(self, row):
        loc = row['resolved'].get('DW_AT_data_member_location')
        if loc is None:
            parent = self.dies[row['parent']]
            return 0 if parent['tag'] == 'DW_TAG_union_type' else None
        match = re.search(r'DW_OP_plus_uconst: (\d+)', loc)
        return int(match[1]) if match else number(loc) if 'DW_OP_' not in loc and 'block' not in loc else None

    def signature(self, offset, seen=()):
        if offset is None:
            return ('void',)
        d = self.dies[offset]
        tag = d['tag'].removeprefix('DW_TAG_')
        if offset in seen:
            return ('recursive', tag, d.get('name'))
        seen = (*seen, offset)
        children = []
        for c in self.children[offset]:
            if c['tag'] in ('DW_TAG_member', 'DW_TAG_formal_parameter', 'DW_TAG_enumerator', 'DW_TAG_unspecified_parameters'):
                children.append((c['tag'], c.get('name') if c['tag'] != 'DW_TAG_formal_parameter' else None,
                                 self.member_offset(c) if c['tag'] == 'DW_TAG_member' else None,
                                 c.get('resolved', {}).get('DW_AT_const_value'),
                                 c.get('resolved', {}).get('DW_AT_bit_size'),
                                 c.get('resolved', {}).get('DW_AT_bit_offset'),
                                 self.signature(c.get('type_ref'), seen)))
        return (tag, d.get('name'), number(d['resolved'].get('DW_AT_byte_size')),
                d['resolved'].get('DW_AT_encoding'), tuple(self.dimensions(offset)),
                self.signature(d['type_ref'], seen) if d.get('type_ref') else None, tuple(children))

    def declaration(self, offset, name='', expand=False):
        if offset is None:
            return ('void ' + name).strip()
        d = self.dies[offset]
        tag = d['tag'].removeprefix('DW_TAG_')
        ref = d.get('type_ref')
        if tag == 'typedef' and not expand:
            return (d['name'] + ' ' + name).strip()
        if tag == 'typedef':
            return self.declaration(ref, name)
        if tag in ('const_type', 'volatile_type'):
            qualifier = tag.split('_')[0]
            if ref and self.dies[ref]['tag'] == 'DW_TAG_pointer_type':
                return self.declaration(ref, qualifier + ' ' + name)
            return qualifier + ' ' + self.declaration(ref, name)
        if tag == 'pointer_type':
            inner = '*' + name
            if ref and self.dies[ref]['tag'] in ('DW_TAG_array_type', 'DW_TAG_subroutine_type'):
                inner = '(' + inner + ')'
            return self.declaration(ref, inner)
        if tag == 'array_type':
            dims = ''.join('[%s]' % (n if n is not None else '') for n in self.dimensions(offset))
            return self.declaration(ref, name + dims)
        if tag == 'subroutine_type':
            args = [self.declaration(c.get('type_ref')) for c in self.children[offset] if c['tag'] == 'DW_TAG_formal_parameter']
            if any(c['tag'] == 'DW_TAG_unspecified_parameters' for c in self.children[offset]):
                args.append('...')
            return self.declaration(ref, name + '(' + ', '.join(args or ['void']) + ')')
        if tag in ('structure_type', 'union_type', 'enumeration_type'):
            kind = {'structure_type': 'struct', 'union_type': 'union', 'enumeration_type': 'enum'}[tag]
            if d.get('name'):
                return kind + ' ' + d['name'] + (' ' + name if name else '')
            if tag == 'enumeration_type':
                items = [c['name'] + ' = ' + c['resolved']['DW_AT_const_value'] for c in self.children[offset] if c['tag'] == 'DW_TAG_enumerator']
                return 'enum { ' + ', '.join(items) + ' } ' + name
            members = []
            for c in self.children[offset]:
                if c['tag'] != 'DW_TAG_member':
                    continue
                if 'DW_AT_bit_size' in c['resolved']:
                    raise ValueError('Bitfield emission requires allocation proof: DIE %s' % c['offset'])
                members.append(self.declaration(c['type_ref'], c['name']) + ';')
            return kind + ' {\n    ' + '\n    '.join(members) + '\n} ' + name
        if tag == 'base_type':
            return (d['name'] + ' ' + name).strip()
        raise ValueError('Unsupported type DIE %s: %s' % (offset, tag))

    def variable(self, d):
        ref=d.get('type_ref'); visited=set()
        while ref and ref not in visited and self.dies[ref]['tag'] in ('DW_TAG_typedef','DW_TAG_const_type','DW_TAG_volatile_type','DW_TAG_pointer_type','DW_TAG_array_type'):
            visited.add(ref); ref=self.dies[ref].get('type_ref')
        base=self.dies.get(ref,{})
        return {'die': d['offset'], 'name': d.get('name'), 'type_die': d.get('type_ref'),
                'type': self.declaration(d.get('type_ref')), 'scope': d.get('parent'),
                'base_type':base.get('name'),'base_encoding':base.get('resolved',{}).get('DW_AT_encoding'),
                'location': d['resolved'].get('DW_AT_location'), 'address': d.get('address')}


@lru_cache(maxsize=1)
def graph():
    return TypeGraph()
