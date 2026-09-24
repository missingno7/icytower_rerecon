"""Two-file publication with a durable rollback journal and fail-closed readers."""
import base64, ctypes, json, os
from pathlib import Path
from common import ROOT, BUILD, atomic_bytes, read_json, write_json, sha

LOCK=BUILD/'promotion.lock'
JOURNAL=BUILD/'promotion.json'

def ensure_consistent():
    if JOURNAL.exists():raise ValueError('Interrupted promotion; run python tools/promote.py --recover before reading canonical state')

def running(pid):
    if os.name=='nt':
        kernel=ctypes.WinDLL('kernel32',use_last_error=True)
        kernel.OpenProcess.restype=ctypes.c_void_p
        handle=kernel.OpenProcess(0x00100000,False,pid) # SYNCHRONIZE, never terminate
        if not handle:return ctypes.get_last_error()==5 # Access denied is conservatively live.
        kernel.WaitForSingleObject.argtypes=[ctypes.c_void_p,ctypes.c_uint32]
        kernel.CloseHandle.argtypes=[ctypes.c_void_p]
        try:return kernel.WaitForSingleObject(handle,0)!=0
        finally:kernel.CloseHandle(handle)
    try:os.kill(pid,0);return True
    except ProcessLookupError:return False
    except PermissionError:return True

def rollback(root,journal):
    root=Path(root).resolve();data=read_json(journal);changes=[]
    for row in data['files']:
        rel=Path(row['path']);path=(root/rel).resolve()
        if not path.is_relative_to(root) or (rel.as_posix()!='recovery.json' and (len(rel.parts)!=2 or rel.parts[0]!='src' or rel.suffix!='.c')):
            raise ValueError('Unsafe journal path')
        old=base64.b64decode(row['before']);new=base64.b64decode(row['after']);current=path.read_bytes()
        if current not in (old,new):raise ValueError('Concurrent edit preserved; journal recovery requires manual resolution: '+str(path))
        changes.append((path,old))
    for path,data in changes:atomic_bytes(path,data)
    Path(journal).unlink()

def publish(source,new_source,state_bytes,expected_source,expected_state,root=ROOT,journal=JOURNAL,failpoint=None):
    root=Path(root).resolve();source=Path(source).resolve();state=root/'recovery.json'
    if source.parent!=root/'src' or source.suffix!='.c':raise ValueError('Only one historical TU can be published')
    if source.read_bytes()!=expected_source or state.read_bytes()!=expected_state:raise ValueError('Canonical source/state changed before commit')
    if Path(journal).exists():raise ValueError('Pending publication journal')
    rows=[]
    for path,before,after in [(source,expected_source,new_source),(state,expected_state,state_bytes)]:
        rows.append({'path':path.relative_to(root).as_posix(),'before':base64.b64encode(before).decode(),'after':base64.b64encode(after).decode()})
    write_json(journal,{'files':rows})
    try:
        atomic_bytes(source,new_source)
        if failpoint:failpoint('source')
        atomic_bytes(state,state_bytes)
        if failpoint:failpoint('state')
        if source.read_bytes()!=new_source or state.read_bytes()!=state_bytes:raise ValueError('Published files changed before commit completed')
        Path(journal).unlink()
    except BaseException:
        rollback(root,journal)
        raise

def recover():
    if LOCK.exists():
        try:pid=int(LOCK.read_text())
        except ValueError:raise ValueError('Invalid promotion lock; inspect it before removing')
        if running(pid):raise ValueError('Promotion process is still running')
    recovery_lock=BUILD/'promotion-recovery.lock'
    from common import exclusive
    with exclusive(recovery_lock):
        if LOCK.exists():
            try:pid=int(LOCK.read_text())
            except ValueError:raise ValueError('Invalid promotion lock; inspect it before removing')
            if running(pid):raise ValueError('Promotion process is still running')
        if JOURNAL.exists():rollback(ROOT,JOURNAL)
        LOCK.unlink(missing_ok=True)
    print('Interrupted promotion rolled back; canonical state is consistent.')
