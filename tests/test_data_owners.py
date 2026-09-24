"""Object ownership negative controls independent of target instruction operands."""
import copy
import struct
from types import SimpleNamespace
import unittest
from data_owners import independent_owners,resolve_owner


def fixtures():
    def dies(address):
        return {1:{'offset':1,'tag':'DW_TAG_compile_unit','name':'original.c','parent':None,'cu':1,'resolved':{}},
                2:{'offset':2,'tag':'DW_TAG_base_type','name':'int','parent':1,'cu':1,'type_ref':None,'resolved':{'DW_AT_byte_size':'4','DW_AT_encoding':'5 (signed)'}},
                3:{'offset':3,'tag':'DW_TAG_variable','name':'counter','parent':1,'cu':1,'type_ref':2,'address':address,'resolved':{}}}
    def binary(value,va,base,rva):
        section={'index':2,'name':'.data','raw_size':32,'virtual_size':32,'rva':rva}
        sym={'index':4,'name':'_counter','value':value,'va':va,'section':2,'storage_class':2}
        return SimpleNamespace(sections=[section],symbols=[sym],relocations=[],image_base=base,
                               section_bytes=lambda section:bytes(32),at_va=lambda address,size:bytes(size))
    return binary(8,8,0,0),binary(8,108,100,0),dies(8),dies(108)


class OwnerTests(unittest.TestCase):
    def test_unique_type_owner_and_full_initializer(self):
        args=fixtures(); result=independent_owners(*args,'original.c')
        self.assertEqual(len(result['accepted']),1)
        self.assertEqual(resolve_owner(result,2,8),108)
        self.assertEqual(resolve_owner(result,2,11),111)
        self.assertIsNone(resolve_owner(result,2,12))
        self.assertIsNone(resolve_owner(result,1,8))

    def test_initializer_corruption(self):
        args=fixtures(); args[0].section_bytes=lambda s:bytes(8)+b'bad!'+bytes(20)
        self.assertFalse(independent_owners(*args,'original.c')['accepted'])

    def test_type_mismatch(self):
        for attribute,value in [('DW_AT_byte_size','8'),('DW_AT_encoding','7 (unsigned)')]:
            args=fixtures(); args[2][2]['resolved'][attribute]=value
            self.assertFalse(independent_owners(*args,'original.c')['accepted'])

    def test_duplicate_original_identity(self):
        args=fixtures(); args[3][5]=dict(args[3][3],offset=5)
        self.assertFalse(independent_owners(*args,'original.c')['accepted'])

    def test_initializer_relocations_need_separate_proof(self):
        args=fixtures(); args[0].relocations=[{'section':2,'offset':8}]
        self.assertFalse(independent_owners(*args,'original.c')['accepted'])

    def relocated_fixture(self):
        args=fixtures(); obj,exe,_,_=args
        obj.relocations=[{'section':2,'offset':8,'type':6,'symbol_index':9}]
        obj.by_index={9:{'name':'_independent_target'}}
        obj.section_bytes=lambda s:bytes(8)+struct.pack('<I',3)+bytes(20)
        exe.at_va=lambda address,size:struct.pack('<I',403)[:size]
        return args

    def test_initializer_relocation_requires_independent_target_and_exact_bytes(self):
        args=self.relocated_fixture(); seen=[]
        def resolver(symbol,addend):
            seen.append((symbol['name'],addend))
            return 400+addend,'independent named symbol'
        result=independent_owners(*args,'original.c',resolver)
        self.assertEqual(seen,[('_independent_target',3)])
        self.assertEqual(len(result['accepted']),1)
        self.assertEqual(result['accepted'][0]['initializer_relocations'][0]['target_va'],403)
        for target in (None,404):
            self.assertFalse(independent_owners(*args,'original.c',lambda s,a:(target,'test'))['accepted'])
        args[0].section_bytes=lambda s:bytes(8)+struct.pack('<I',4)+bytes(20)
        self.assertFalse(independent_owners(*args,'original.c',resolver)['accepted'])

    def test_initializer_boundary_type_overlap_and_bss_rejected(self):
        for change in ('left','right','type','overlap','bss'):
            args=self.relocated_fixture(); obj,exe,_,_=args
            if change=='left': obj.relocations[0]['offset']=7
            elif change=='right': obj.relocations[0]['offset']=9
            elif change=='type': obj.relocations[0]['type']=20
            elif change=='overlap': obj.relocations*=2
            else:
                obj.sections[0]['name']='.bss'; exe.sections[0]['name']='.bss'
            self.assertFalse(independent_owners(*args,'original.c',lambda s,a:(403,'test'))['accepted'],change)

    def test_overlapping_owner_rejected(self):
        args=fixtures(); args[0].symbols.append(dict(args[0].symbols[0],name='_alias',index=5))
        self.assertFalse(independent_owners(*args,'original.c')['accepted'])

    def test_incomplete_section_and_wrong_cu_rejected(self):
        args=fixtures(); args[0].sections[0].update(raw_size=10,virtual_size=10)
        self.assertFalse(independent_owners(*args,'original.c')['accepted'])
        self.assertFalse(independent_owners(*fixtures(),'different.c')['accepted'])

    def test_function_static_scope_is_part_of_identity(self):
        args=fixtures()
        for graph in args[2:]:
            graph[4]={'offset':4,'tag':'DW_TAG_subprogram','name':'callback','parent':1,'cu':1,'resolved':{}}
            graph[3]['parent']=4
        args[0].symbols[0].update(name='_counter.123',storage_class=3)
        args[1].symbols[0].update(name='_counter.456',storage_class=3)
        self.assertEqual(len(independent_owners(*args,'original.c')['accepted']),1)
        args[3][4]['name']='different_callback'
        self.assertFalse(independent_owners(*args,'original.c')['accepted'])


if __name__=='__main__': unittest.main()
