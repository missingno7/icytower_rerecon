"""Rebuild historical dependency archives and perform an ordinary source link."""
import argparse, concurrent.futures, re
from pathlib import Path
from common import ROOT, BUILD, TC, identity, read_json, write_json, digest, run, process, exclusive
from build import verify_inputs, compile_target

def libraries():
    verify_inputs();plan=read_json(ROOT/'third_party/build.json')
    inputs={'vendor':read_json(ROOT/'third_party/lock.json'),'compiler':read_json(ROOT/'toolchain/lock.json'),
            'plan':plan,'driver':identity(Path(__file__)),'common':identity(ROOT/'tools/common.py'),
            'headers':{p.relative_to(ROOT).as_posix():identity(p) for p in (ROOT/'include').rglob('*') if p.is_file()}}
    key=digest(inputs);out=BUILD/'libraries'/key;receipt=out/'libraries.json'
    if receipt.exists():
        data=read_json(receipt)
        if all(identity(out/name)==v for name,v in data.items()):return out
        raise ValueError('Cached dependency archive changed; remove its build directory and rebuild')
    out.mkdir(parents=True,exist_ok=True)
    with exclusive(out/'build.lock'):
        units=[]
        for family in ['allegro','xiph']:
            group=plan[family]
            includes=['-Iinclude','-Ithird_party/allegro-4.4.1/include','-Ithird_party/dx80_mgw/include'] if family=='allegro' else ['-Ithird_party/libvorbis-1.2.0/include','-Ithird_party/libogg-1.1.3/include']
            for row in group['units']:units.append((family,row,[*group['configuration'],*includes]))
        def compile_unit(task):
            family,row,flags=task;obj=out/(family+'_'+row['object']);obj.unlink(missing_ok=True)
            run([TC/'bin/gcc.exe',*flags,'-c',row['source'],'-o',obj])
            return row.get('archive','liballeg.a'),obj
        with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:built=list(pool.map(compile_unit,units))
        for archive in sorted({name for name,_ in built}):
            dest=out/archive;dest.unlink(missing_ok=True)
            run([TC/'bin/ar.exe','rcs',dest,*[p for n,p in built if n==archive]])
        run([TC/'bin/dlltool.exe','-d','third_party/libpng-1.2.34/libpng3-derived.def','-l',out/'libpng3.a'])
        compile_target('allegro-logg',out/'logg',config=plan['logg'])
        verify_inputs()
        if inputs['headers']!={p.relative_to(ROOT).as_posix():identity(p) for p in (ROOT/'include').rglob('*') if p.is_file()}:
            raise ValueError('Platform headers changed while building libraries')
        write_json(receipt,{p.relative_to(out).as_posix():identity(p) for p in [*out.glob('*.a'),out/'logg/unit.o']})
        print('Rebuilt',len(units),'historical vendor units and modified logg',flush=True)
    return out

def protect_link(before,after):
    if before['linked'] and not after['linked']:raise ValueError('Ordinary link stopped closing')
    if not set(after['unresolved_symbols']).issubset(before['unresolved_symbols']):
        raise ValueError('New ordinary link unresolved symbols: '+str(sorted(set(after['unresolved_symbols'])-set(before['unresolved_symbols']))))

def ordinary_link(dest=None,objects=None,source_root=ROOT):
    dest=Path(dest or BUILD/'link');dest.mkdir(parents=True,exist_ok=True)
    lib=libraries();plan=read_json(ROOT/'third_party/build.json')
    if objects is None:objects={t:compile_target(t,dest/t,source_root)[0] for t in plan['game_order']}
    if set(objects)!=set(plan['game_order']):raise ValueError('Ordinary link requires all 25 historical game TUs')
    inputs=[*[Path(objects[t]) for t in plan['game_order']],lib/'logg/unit.o',lib/'libvorbisfile.a',lib/'libvorbis.a',lib/'libogg.a',lib/'liballeg.a',lib/'libpng3.a']
    before={str(p):identity(p) for p in inputs}
    exe=dest/'diagnostic-game.exe';exe.unlink(missing_ok=True)
    args=[TC/'bin/gcc.exe','-O2','-g','-mfpmath=387','-DALLEGRO_STATICLINK','-mwindows',
        *[objects[t] for t in plan['game_order']],lib/'logg/unit.o',lib/'libvorbisfile.a',lib/'libvorbis.a',lib/'libogg.a',lib/'liballeg.a',
        '-lkernel32','-luser32','-lgdi32','-lcomdlg32','-lole32','-ldinput','-lddraw','-ldxguid','-lwinmm','-ldsound','-lws2_32','-lpthread',
        lib/'libpng3.a','-lm','-Wl,-Map,'+str((dest/'game.map').resolve()),'-Wl,--cref','-o',exe]
    result=process(args);stderr=result.stderr.decode(errors='replace')
    verify_inputs()
    if before!={str(p):identity(p) for p in inputs}:raise ValueError('Ordinary link input changed')
    unresolved=sorted(set(re.findall(r"undefined reference to [`']([^`']+)[`']",stderr)))
    record={'linked':result.returncode==0,'unresolved_symbols':unresolved,
            'scope':'Ordinary diagnostic source link; unresolved gameplay and vendor recovery remain; never executed'}
    write_json(dest/'link.json',dict(record,returncode=result.returncode,stderr=stderr,stdout=result.stdout.decode(errors='replace'),
      inputs=before,command=[str(a) for a in args],executable=identity(exe) if exe.exists() else None))
    if result.returncode and (not unresolved or re.search(r'cannot find|multiple definition|unrecognized|file format not recognized|cannot open|Permission denied',stderr,re.I)):
        raise RuntimeError('Ordinary link infrastructure failure: '+stderr)
    print('Ordinary link:', 'closed (diagnostic)' if record['linked'] else 'unresolved '+', '.join(unresolved),flush=True)
    return record

if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--libraries-only',action='store_true');a=ap.parse_args()
    print(libraries()) if a.libraries_only else ordinary_link()
