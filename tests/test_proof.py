import struct, tempfile, unittest
from pathlib import Path
from common import ROOT, ORACLE, ANALYSIS
from binary import Binary
from build import compile_target
from compare import compare
from dwarf import parse
OBJDUMP=ANALYSIS
CU='F:\\projects\\icytower\\trunk\\source\\timer.c'

class PipelineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        for target in ['game-timer','game-main','game-profile']:
            compile_target(target,ROOT/'build/test-objects'/target)
        cls.obj=ROOT/'build/test-objects/game-timer/unit.o'


    def check_object(self, path):
        return compare(path, CU, ORACLE, OBJDUMP)

    def mutate(self, editor):
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as folder:
            path = Path(folder) / 'unit.o'
            contents = bytearray(self.obj.read_bytes())
            editor(contents, Binary(self.obj))
            path.write_bytes(contents)
            return self.check_object(path)

    def test_complete_timer_text(self):
        r = self.check_object(self.obj)
        self.assertEqual(r['functions_total'], 3)
        self.assertEqual(r['function_matches'], 3)
        self.assertTrue(r['whole_text_contribution_equal'])
        self.assertEqual(r['candidate_text_logical_size'], 152)
        self.assertEqual(len(r['common_allocations']), 5)
        self.assertFalse(r['object_match'])

    def test_repeated_literal_neighbourhood_rejects_wrong_target(self):
        obj = ROOT / 'build/test-objects/game-main/unit.o'
        cu = 'F:\\projects\\icytower\\trunk\\source\\main.c'
        exact = compare(obj, cu, ORACLE, OBJDUMP)
        line = next((f for f in exact['functions'] if f['name'] == 'line_intersect'))
        relocation = line['relocations'][0]
        b = Binary(obj)
        section = next((s for s in b.sections if s['index'] == relocation['section']))
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as folder:
            altered = Path(folder) / 'unit.o'
            contents = bytearray(obj.read_bytes())
            struct.pack_into('<I', contents, section['raw_pointer'] + relocation['offset'], relocation['addend'] - 4)
            altered.write_bytes(contents)
            wrong = compare(altered, cu, ORACLE, OBJDUMP)
        line = next((f for f in wrong['functions'] if f['name'] == 'line_intersect'))
        self.assertTrue(line['masked_equal'])
        self.assertFalse(line['relocation_resolved_equal'])
        self.assertNotEqual(line['status'], 'FUNCTION_MATCH')

    def test_trailing_literal_neighbourhood_rejects_wrong_target(self):
        obj = ROOT / 'build/test-objects/game-main/unit.o'
        cu = 'F:\\projects\\icytower\\trunk\\source\\main.c'
        exact = compare(obj, cu, ORACLE, OBJDUMP)
        jump = next((f for f in exact['functions'] if f['name'] == 'play_jump_sound'))
        relocation = jump['relocations'][0]
        self.assertTrue(relocation['equal'])
        b = Binary(obj)
        section = next((s for s in b.sections if s['index'] == relocation['section']))
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as folder:
            altered = Path(folder) / 'unit.o'
            contents = bytearray(obj.read_bytes())
            struct.pack_into('<I', contents, section['raw_pointer'] + relocation['offset'], relocation['addend'] + 4)
            altered.write_bytes(contents)
            wrong = compare(altered, cu, ORACLE, OBJDUMP)
        jump = next((f for f in wrong['functions'] if f['name'] == 'play_jump_sound'))
        self.assertFalse(jump['relocations'][0]['equal'])
        self.assertNotEqual(jump['status'], 'FUNCTION_MATCH')

    def test_wrong_direct_same_unit_target_is_rejected(self):
        obj = ROOT / 'build/test-objects/game-main/unit.o'
        cu = 'F:\\projects\\icytower\\trunk\\source\\main.c'
        exact = compare(obj, cu, ORACLE, OBJDUMP)
        save = next((f for f in exact['functions'] if f['name'] == 'save_config'))
        end = next((f for f in exact['functions'] if f['name'] == 'end_game'))
        transfer = save['direct_transfers'][0]
        b = Binary(obj)
        text = next((s for s in b.sections if s['name'] == '.text'))
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as folder:
            altered = Path(folder) / 'unit.o'
            contents = bytearray(obj.read_bytes())
            displacement = end['candidate_offset'] - (save['candidate_offset'] + transfer['function_offset'] + 4)
            struct.pack_into('<i', contents, text['raw_pointer'] + save['candidate_offset'] + transfer['function_offset'], displacement)
            altered.write_bytes(contents)
            wrong = compare(altered, cu, ORACLE, OBJDUMP)
        save = next((f for f in wrong['functions'] if f['name'] == 'save_config'))
        self.assertEqual(save['direct_transfers'][0]['target_function'], 'end_game')
        self.assertFalse(save['direct_transfers'][0]['equal'])
        self.assertNotEqual(save['status'], 'FUNCTION_MATCH')

    def test_wrong_relocation_target_is_rejected_despite_masked_equality(self):

        def mutate(data, b):
            sec = b.sections[0]
            wrong = next((s['index'] for s in b.symbols if s['name'] == '_fps'))
            struct.pack_into('<I', data, sec['relocation_pointer'] + 4, wrong)
        r = self.mutate(mutate)
        fps = next((f for f in r['functions'] if f['name'] == 'fps_counter'))
        self.assertTrue(fps['masked_equal'])
        self.assertFalse(fps['relocation_resolved_equal'])
        self.assertFalse(r['whole_text_contribution_equal'])

    def test_code_corruption_is_rejected(self):
        r = self.mutate(lambda data, b: data.__setitem__(b.sections[0]['raw_pointer'], 144))
        self.assertFalse(r['functions'][0]['masked_equal'])
        self.assertFalse(r['whole_text_contribution_equal'])

    def test_padding_corruption_is_rejected(self):
        r = self.mutate(lambda data, b: data.__setitem__(b.sections[0]['raw_pointer'] + 45, 144))
        self.assertEqual(r['function_matches'], 3)
        self.assertFalse(r['whole_text_contribution_equal'])

    def test_unknown_relocation_type_is_rejected(self):

        def mutate(data, b):
            struct.pack_into('<H', data, b.sections[0]['relocation_pointer'] + 8, 4660)
        r = self.mutate(mutate)
        self.assertFalse(r['whole_text_contribution_equal'])
        self.assertFalse(r['functions'][0]['relocation_resolved_equal'])

    def test_origin_chain_is_followed(self):
        text = ' <0><b>: Abbrev Number: 1 (DW_TAG_compile_unit)\n    <c> DW_AT_name : test.c\n <1><20>: Abbrev Number: 2 (DW_TAG_subprogram)\n    <21> DW_AT_name : original_name\n    <22> DW_AT_type : <0x50>\n <1><30>: Abbrev Number: 3 (DW_TAG_subprogram)\n    <31> DW_AT_specification : <0x20>\n <1><40>: Abbrev Number: 4 (DW_TAG_subprogram)\n    <41> DW_AT_abstract_origin : <0x30>\n    <42> DW_AT_low_pc : 0x1234\n    <43> DW_AT_high_pc : 0x1240\n <1><50>: Abbrev Number: 5 (DW_TAG_base_type)\n    <51> DW_AT_name : int\n'
        (dies, stats) = parse(text)
        self.assertEqual(dies[64]['name'], 'original_name')
        self.assertEqual(dies[64]['type_ref'], 80)
        self.assertEqual(stats['unresolved_origin_specification'], [])

    def test_main_named_data_owner_is_independent_and_strict(self):
        path = ROOT / 'build/test-objects/game-main/unit.o'
        cu = 'F:\\projects\\icytower\\trunk\\source\\main.c'
        base = compare(path, cu, ORACLE, OBJDUMP)
        row = next((r for r in base['functions'] if r['name'] == 'startGameMusic'))
        self.assertEqual(row['status'], 'FUNCTION_MATCH')
        callback = next((r for r in base['functions'] if r['name'] == 'datafile_callback_slow'))
        self.assertEqual(callback['status'], 'FUNCTION_MATCH')
        relocated = next((r for r in row['relocations'] if 'object owner' in r['resolution']))
        obj = Binary(path)
        text = next((s for s in obj.sections if s['name'] == '.text'))
        for change in ('operand', 'initializer'):
            payload = bytearray(path.read_bytes())
            if change == 'operand':
                struct.pack_into('<I', payload, text['raw_pointer'] + relocated['offset'], relocated['addend'] + 1)
            else:
                symbol = next((s for s in obj.symbols if s['name'] == '_gameMusicVoiceID'))
                section = next((s for s in obj.sections if s['index'] == symbol['section']))
                payload[section['raw_pointer'] + symbol['value']] ^= 1
            with tempfile.TemporaryDirectory(dir=ROOT / 'build') as folder:
                candidate = Path(folder) / 'unit.o'
                candidate.write_bytes(payload)
                report = compare(candidate, cu, ORACLE, OBJDUMP)
                changed = next((r for r in report['functions'] if r['name'] == 'startGameMusic'))
                self.assertNotEqual(changed['status'], 'FUNCTION_MATCH', change)

    def test_pointer_initializer_owner_is_independent_and_strict(self):
        path = ROOT / 'build/test-objects/game-profile/unit.o'
        cu = 'F:\\projects\\icytower\\trunk\\source\\profile.c'
        base = compare(path, cu, ORACLE, OBJDUMP)
        owner = next((o for o in base['object_ownership']['accepted'] if o['name'] == 'jcLabels'))
        self.assertEqual(len(owner['initializer_relocations']), 5)
        obj = Binary(path)
        section = next((s for s in obj.sections if s['index'] == owner['section_index']))
        first = owner['initializer_relocations'][0]
        for mutation in ('addend', 'literal', 'relocation_type'):
            payload = bytearray(path.read_bytes())
            if mutation == 'addend':
                struct.pack_into('<I', payload, section['raw_pointer'] + first['section_offset'], first['addend'] + 1)
            elif mutation == 'literal':
                rdata = next((s for s in obj.sections if s['name'] == '.rdata'))
                payload[rdata['raw_pointer'] + first['addend']] ^= 1
            else:
                index = next((i for (i, r) in enumerate((r for r in obj.relocations if r['section'] == section['index'])) if r['offset'] == first['section_offset']))
                struct.pack_into('<H', payload, section['relocation_pointer'] + index * 10 + 8, 4660)
            with tempfile.TemporaryDirectory(dir=ROOT / 'build') as folder:
                candidate = Path(folder) / 'unit.o'
                candidate.write_bytes(payload)
                report = compare(candidate, cu, ORACLE, OBJDUMP)
                if mutation == 'literal':
                    # The independently proved .rdata base can preserve pointer
                    # ownership when its pointee changes. Full storage must reject it.
                    from storage import protect_storage
                    self.assertFalse(next(s for s in report['initialized_data_comparison']
                                          if s['section'] == '.rdata')['content_equal'])
                    with self.assertRaises(ValueError):
                        protect_storage(base, report, path, candidate)
                else:
                    self.assertNotIn('jcLabels', [o['name'] for o in report['object_ownership']['accepted']], mutation)

    def test_changed_unproved_storage_is_refused(self):
        from storage import protect_storage
        path=ROOT/'build/test-objects/game-main/unit.o'
        report=compare(path,'F:\\projects\\icytower\\trunk\\source\\main.c',ORACLE,ANALYSIS)
        section=next(s for s in Binary(path).sections if s['name']=='.rdata')
        with tempfile.TemporaryDirectory(dir=ROOT/'build') as folder:
            changed=Path(folder)/'unit.o';data=bytearray(path.read_bytes());data[section['raw_pointer']]^=1;changed.write_bytes(data)
            with self.assertRaises(ValueError):protect_storage(report,report,path,changed)

    def test_locked_modified_logg_still_matches(self):
        from common import read_json
        config=read_json(ROOT/'third_party/build.json')['logg']
        path,_=compile_target('allegro-logg',ROOT/'build/test-objects/allegro-logg',config=config)
        report=compare(path,config['historical_cu'],ORACLE,ANALYSIS)
        self.assertEqual(report['function_matches'],18)
        self.assertTrue(report['whole_text_contribution_equal'])
