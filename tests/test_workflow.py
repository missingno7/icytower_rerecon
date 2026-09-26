import base64, copy, json, tempfile, unittest
from pathlib import Path
from unittest.mock import patch
from common import ROOT, BUILD, ORACLE, identity, read_json, write_json, verify_manifest
from scratch import fragment, materialized, acceptance_syntax
from source_scope import function_span
from transaction import publish, rollback
from verify import protect, canonical_inputs
from metrics import effective_identity

class TransactionTests(unittest.TestCase):
    def test_live_process_is_not_stale(self):
        import os
        from transaction import running
        self.assertTrue(running(os.getpid()))

    def prepare(self,root):
        source=root/'src/timer.c';source.parent.mkdir();source.write_bytes(b'old source')
        (root/'recovery.json').write_bytes(b'old state');return source,root/'journal.json'
    def test_success_publishes_both(self):
        with tempfile.TemporaryDirectory(dir=BUILD) as folder:
            root=Path(folder);source,journal=self.prepare(root)
            publish(source,b'new source',b'new state',b'old source',b'old state',root,journal)
            self.assertEqual(source.read_bytes(),b'new source');self.assertEqual((root/'recovery.json').read_bytes(),b'new state');self.assertFalse(journal.exists())
    def test_failure_at_either_write_rolls_back(self):
        for boundary in ['source','state']:
            with self.subTest(boundary=boundary),tempfile.TemporaryDirectory(dir=BUILD) as folder:
                root=Path(folder);source,journal=self.prepare(root)
                def fail(point):
                    if point==boundary:raise RuntimeError('injected failure')
                with self.assertRaises(RuntimeError):publish(source,b'new source',b'new state',b'old source',b'old state',root,journal,fail)
                self.assertEqual(source.read_bytes(),b'old source');self.assertEqual((root/'recovery.json').read_bytes(),b'old state');self.assertFalse(journal.exists())
    def interrupted(self,root,source,journal):
        enc=lambda b:base64.b64encode(b).decode()
        write_json(journal,{'files':[{'path':'src/timer.c','before':enc(b'old source'),'after':enc(b'new source')},
          {'path':'recovery.json','before':enc(b'old state'),'after':enc(b'new state')}]})
        source.write_bytes(b'new source')
    def test_crash_recovery_and_foreign_edit_protection(self):
        with tempfile.TemporaryDirectory(dir=BUILD) as folder:
            root=Path(folder);source,journal=self.prepare(root);self.interrupted(root,source,journal)
            source.write_bytes(b'foreign edit')
            with self.assertRaises(ValueError):rollback(root,journal)
            self.assertEqual(source.read_bytes(),b'foreign edit');self.assertTrue(journal.exists())
            source.write_bytes(b'new source');rollback(root,journal)
            self.assertEqual(source.read_bytes(),b'old source');self.assertEqual((root/'recovery.json').read_bytes(),b'old state')
    def test_journal_path_escape_rejected(self):
        with tempfile.TemporaryDirectory(dir=BUILD) as folder:
            root=Path(folder);_,journal=self.prepare(root)
            write_json(journal,{'files':[{'path':'../outside.c','before':'','after':''}]})
            with self.assertRaises(ValueError):rollback(root,journal)

class IsolationTests(unittest.TestCase):
    def test_overlay_preserves_source_and_discards_full_tu(self):
        before=canonical_inputs()
        with materialized('game-timer','cycle_counter',b'{ cycle_count += 2; }') as shadow:
            path=shadow
            self.assertIn(b'cycle_count += 2;', (shadow/'src/timer.c').read_bytes())
            (shadow/'include/timer.h').write_bytes(b'changed in scratch')
        self.assertFalse(path.exists());self.assertEqual(before,canonical_inputs())
    def test_compact_tu_patch_checks_base_and_unique_context(self):
        from scratch import patched_source
        from common import sha
        original=(ROOT/'src/timer.c').read_bytes()
        with tempfile.TemporaryDirectory(dir=BUILD) as folder:
            path=Path(folder)/'variant.json'
            data={'source':'src/timer.c','base_sha256':sha(original),'edits':[{'old':'cycle_count++;','new':'cycle_count += 2;'}]}
            write_json(path,data)
            patched=patched_source(path,'game-timer')
            self.assertIn(b'cycle_count += 2;',patched);self.assertEqual((ROOT/'src/timer.c').read_bytes(),original)
            data['base_sha256']='wrong';write_json(path,data)
            with self.assertRaises(ValueError):patched_source(path,'game-timer')
            data['base_sha256']=sha(original);data['edits'][0]['old']='int';write_json(path,data)
            with self.assertRaises(ValueError):patched_source(path,'game-timer')

    def test_fragment_boundaries_and_codegen_hacks(self):
        with tempfile.TemporaryDirectory(dir=BUILD) as folder:
            p=Path(folder)/'body.c'
            for bad in [b'{} int other;',b'void f() {}',b'{']:
                p.write_bytes(bad)
                with self.assertRaises(ValueError):fragment(p,'f')
            p.write_bytes(b'{ const char *p="}"; /* } */ if (1) {} }')
            self.assertEqual(fragment(p,'f'),p.read_bytes())
        for bad in [b'{ __asm("nop"); }',b'{ _Pragma("GCC optimize") }',b'{ __as'+bytes([92,10])+b'm__("nop"); }',b'{ __asm__("nop"); }',b'{ volatile int x; }',b'{\n#include "other.h"\n}']:
            with self.assertRaises(ValueError):acceptance_syntax(bad)
    def test_input_race_deletes_object(self):
        import build
        before=canonical_inputs()
        with materialized('game-timer') as root:
            actual=build.run
            def racing(args,**kw):
                result=actual(args,**kw)
                if '-c' in args:
                    p=root/'src/timer.c';p.write_bytes(p.read_bytes()+b'\n/* raced */\n')
                return result
            with patch.object(build,'run',side_effect=racing):
                with self.assertRaises(ValueError):build.compile_target('game-timer',root/'out',root)
            self.assertFalse((root/'out/unit.o').exists())
        self.assertEqual(before,canonical_inputs())
    def test_ordinary_compiler_does_not_read_oracle(self):
        import build
        actual=Path.read_bytes
        def guarded(path,*args,**kwargs):
            if path==ORACLE or path.name=='evidence.sqlite':raise AssertionError('Compiler read oracle')
            return actual(path,*args,**kwargs)
        with patch.object(Path,'read_bytes',guarded):build.compile_target('game-timer',BUILD/'no-oracle-test')
    def test_wrong_fixture_and_toolchain_drift(self):
        from evidence import verify_oracle
        with tempfile.TemporaryDirectory(dir=BUILD) as folder:
            root=Path(folder);p=root/'input';p.write_bytes(b'input');row={'path':'input',**identity(p)}
            verify_manifest(root,[row],exact=True)
            (root/'extra-specs').write_bytes(b'extra')
            with self.assertRaises(ValueError):verify_manifest(root,[row],exact=True)
            p.write_bytes(b'wrong')
            with self.assertRaises(ValueError):verify_manifest(root,[row])
            with self.assertRaises(ValueError):verify_oracle(p)

class GateTests(unittest.TestCase):
    def report(self):
        return {'functions':[{'name':'peer','status':'FUNCTION_MATCH','body_sha256':'same'}],
          'object_ownership':{'accepted':[{'scope':['GLOBAL'],'name':'x','size':4,'section':'.bss','original_va':100,'dwarf_type':'int'}]},
          'initialized_data_comparison':[{'section':'.rdata','logical_size':8,'content_equal':True}],
          'common_allocations':[{'name':'y','value':4,'storage_class':2}]}
    def test_exact_peers_data_bss_and_bodies_are_protected(self):
        before=self.report()
        for field in ['status','body','owner','data','common']:
            after=copy.deepcopy(before)
            if field=='status':after['functions'][0]['status']='DIFFER'
            if field=='body':after['functions'][0]['body_sha256']='changed'
            if field=='owner':after['object_ownership']['accepted']=[]
            if field=='data':after['initialized_data_comparison'][0]['content_equal']=False
            if field=='common':after['common_allocations'][0]['value']=8
            with self.subTest(field=field),self.assertRaises(ValueError):protect(before,after)
    def storage_reports(self):
        before={'functions':[{'name':'peer','status':'FUNCTION_MATCH','body_sha256':'same'}],
          'object_ownership':{'accepted':[]},
          'initialized_data_comparison':[{'section':'.data','logical_size':8,'content_equal':False}],
          'common_allocations':[{'name':'_g','value':4,'storage_class':2},{'name':'_h','value':4,'storage_class':2}]}
        after=copy.deepcopy(before)
        after['functions'][0]['body_sha256']='changed'
        after['initialized_data_comparison'][0]['content_equal']=True
        after['object_ownership']['accepted']=[{'scope':['GLOBAL'],'name':'g','size':4,'section':'.data','original_va':100,'dwarf_type':'int'}]
        after['common_allocations']=[{'name':'_h','value':4,'storage_class':2}]
        return before,after
    def test_storage_promotion_allows_only_proven_moves(self):
        before,after=self.storage_reports()
        protect(before,after)  # newly proved .data: exact peer keeps code, _g becomes a proven owner
        cases={'no_new_proof':lambda a:a['initialized_data_comparison'][0].update(content_equal=False),
               'peer_lost':lambda a:a['functions'][0].update(status='DIFFER'),
               'owner_missing':lambda a:a['object_ownership'].update(accepted=[]),
               'owner_unproved_section':lambda a:a['object_ownership']['accepted'][0].update(section='.rdata'),
               'common_added':lambda a:a['common_allocations'].append({'name':'_k','value':4,'storage_class':2}),
               'common_resized':lambda a:a['common_allocations'][0].update(value=8)}
        for name,mutate in cases.items():
            changed=copy.deepcopy(after);mutate(changed)
            with self.subTest(case=name),self.assertRaises(ValueError):protect(before,changed)
    def test_effective_output_ignores_only_resolved_layout(self):
        row={'candidate_size':5,'instructions':[{'bytes':'e801000000'}],
          'relocations':[{'function_offset':1,'type':20,'symbol':'_f','addend':1,'target_va':4096}], 'direct_transfers':[]}
        other=copy.deepcopy(row);other['instructions'][0]['bytes']='e845000000';other['relocations'][0]['addend']=69
        self.assertEqual(effective_identity(row),effective_identity(other))
        other['relocations'][0]['target_va']=8192
        self.assertNotEqual(effective_identity(row),effective_identity(other))
        row['relocations'][0]['target_va']=None;other['relocations'][0]['target_va']=None
        self.assertNotEqual(effective_identity(row),effective_identity(other))
    def test_link_regression_refused(self):
        from link import protect_link
        with self.assertRaises(ValueError):protect_link({'linked':True,'unresolved_symbols':[]},{'linked':False,'unresolved_symbols':['new']})
        with self.assertRaises(ValueError):protect_link({'linked':False,'unresolved_symbols':['old']},{'linked':False,'unresolved_symbols':['other']})
