"""Small shared filesystem, identity and subprocess primitives."""
import contextlib, hashlib, json, os, subprocess, tempfile
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / 'build'
TC = BUILD / 'local/toolchain'
ANALYSIS = BUILD / 'local/analysis/objdump.exe'
ORACLE = BUILD / 'local/oracle.exe'

def sha(data): return hashlib.sha256(data).hexdigest()
def identity(path):
    data=Path(path).read_bytes()
    return {'size':len(data),'sha256':sha(data)}
def read_json(path): return json.loads(Path(path).read_text(encoding='utf-8'))
def json_bytes(value): return (json.dumps(value,sort_keys=True,separators=(',',':'))+'\n').encode()
def digest(value): return sha(json_bytes(value))
def atomic_bytes(path,data):
    path=Path(path);path.parent.mkdir(parents=True,exist_ok=True)
    fd,name=tempfile.mkstemp(prefix='.'+path.name+'.',dir=path.parent)
    try:
        with os.fdopen(fd,'wb') as f: f.write(data);f.flush();os.fsync(f.fileno())
        os.replace(name,path)
    finally:
        Path(name).unlink(missing_ok=True)
def write_json(path,value): atomic_bytes(path,json_bytes(value))
def environment():
    # No ambient GCC include/library/spec overrides. All non-system tools resolve locally.
    env={k:v for k,v in os.environ.items() if k.upper() not in {
      'CPATH','C_INCLUDE_PATH','CPLUS_INCLUDE_PATH','OBJC_INCLUDE_PATH','GCC_EXEC_PREFIX',
      'COMPILER_PATH','LIBRARY_PATH','GCC_SPECS','COLLECT_GCC_OPTIONS','DEPENDENCIES_OUTPUT',
      'SUNPRO_DEPENDENCIES'}}
    temp=BUILD/'tmp';temp.mkdir(parents=True,exist_ok=True)
    env.update(PATH=str(TC/'bin')+os.pathsep+str(ANALYSIS.parent)+os.pathsep+str(Path(os.environ.get('SystemRoot','C:/Windows'))/'System32'),
               TMP=str(temp),TEMP=str(temp),LC_ALL='C',LANG='C')
    return env

def process(args,cwd=ROOT):
    return subprocess.run([str(x) for x in args],cwd=cwd,env=environment(),capture_output=True)
def run(args,output=None,cwd=ROOT,toolchain=None):
    r=process(args,cwd)
    if r.returncode: raise RuntimeError(str(args[0])+': '+r.stderr.decode(errors='replace'))
    if output: atomic_bytes(output,r.stdout)
    return r.stdout.decode('utf-8',errors='replace')
def verify_manifest(base,rows,exact=False):
    if exact:
        actual={p.relative_to(base).as_posix() for p in Path(base).rglob("*") if p.is_file()}
        if actual!={r["path"] for r in rows}:raise ValueError("Locked input file set drift: "+str(base))
    for row in rows:
        if identity(Path(base)/row['path'])!={k:row[k] for k in ('size','sha256')}:
            raise ValueError('Input identity drift: '+str(Path(base)/row['path']))
@contextlib.contextmanager
def exclusive(path):
    path=Path(path);path.parent.mkdir(parents=True,exist_ok=True)
    fd=os.open(path,os.O_CREAT|os.O_EXCL|os.O_WRONLY)
    try:
        os.write(fd,str(os.getpid()).encode());os.close(fd);yield
    finally: path.unlink(missing_ok=True)
