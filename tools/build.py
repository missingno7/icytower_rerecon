"""Historical whole-TU compilation. This module never reads the oracle."""
import argparse, re
from pathlib import Path
from common import ROOT, BUILD, TC, identity, read_json, write_json, run, verify_manifest, digest

def targets(): return read_json(ROOT/'evidence/units.json')
def verify_inputs():
    verify_manifest(TC,read_json(ROOT/'toolchain/lock.json')['files'],exact=True)
    path=ROOT/'third_party/lock.json'
    if path.exists(): verify_manifest(ROOT,read_json(path)['files'])
def flags_for(config):
    return [config['default'],*config.get('flags',[]),'-g','-mfpmath=387','-DALLEGRO_STATICLINK',
      '-Iinclude','-Ithird_party/allegro-4.4.1/include',*['-I'+p for p in config.get('includes',[])]]
def dependencies(text,root):
    text=text.replace('\\\n','').replace('\\\r\n','')
    body=text.split(': ',1)[1]
    result=[]
    for token in re.findall(r'(?:\\[ #\\]|[^\s])+',body):
        path=(Path(root)/re.sub(r'\\([ #\\])',r'\1',token).replace('$$','$')).resolve()
        # System headers must be from the installed locked compiler.
        if not path.is_relative_to(Path(root).resolve()) and not path.is_relative_to(TC):
            raise ValueError('External compiler include: '+str(path))
        if path not in result: result.append(path)
    if not result: raise ValueError('Empty dependency set')
    return result

def compile_target(target,dest=None,source_root=ROOT,config=None):
    from transaction import ensure_consistent
    ensure_consistent()
    verify_inputs()
    config=config or targets()[target];source_root=Path(source_root)
    out=Path(dest or BUILD/'objects'/target).resolve();out.mkdir(parents=True,exist_ok=True)
    obj=out/'unit.o';obj.unlink(missing_ok=True)
    flags=flags_for(config)
    dep=run([TC/'bin/gcc.exe',*flags,'-M','-MT','unit',config['source']],cwd=source_root)
    paths=dependencies(dep,source_root)
    allowed={r['path']:r for r in read_json(ROOT/'third_party/lock.json')['files']}
    for p in paths:
        if p.is_relative_to(source_root.resolve()/'third_party'):
            relative=p.relative_to(source_root.resolve()).as_posix()
            row=allowed.get(relative)
            if row is None or identity(p)!={k:row[k] for k in ('size','sha256')}:raise ValueError('Unpinned third-party include: '+str(p))
    before={str(p):identity(p) for p in paths}
    run([TC/'bin/gcc.exe',*flags,'-MD','-MF',out/'unit.d','-c',config['source'],'-o',obj],cwd=source_root)
    after={str(p):identity(p) for p in dependencies((out/'unit.d').read_text(),source_root)}
    if before!=after:
        obj.unlink(missing_ok=True);raise ValueError('Compiler inputs changed during build')
    verify_inputs()
    def relative(path):
        p=Path(path)
        return '@toolchain/'+p.relative_to(TC).as_posix() if p.is_relative_to(TC) else p.relative_to(source_root.resolve()).as_posix()
    inputs={relative(p):v for p,v in before.items()}
    report={'target':target,'config':config,'flags':flags,'inputs':inputs,
      'toolchain':identity(ROOT/'toolchain/lock.json'),'object':identity(obj)}
    report['context_id']=digest({k:v for k,v in report.items() if k!='object'})
    write_json(out/'compile.json',report)
    return obj,report

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('targets',nargs='+')
    a=ap.parse_args()
    for target in a.targets: print(compile_target(target)[0])
if __name__=='__main__':main()
