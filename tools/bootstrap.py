"""Install only locked local inputs into build/local; never write to the source.

By default the inputs come from this workspace's ignored copies:
toolchain/tdm-gcc-4.4.1-tdm-2/ (paths as in toolchain/lock.json archive_path)
and assets/icytower15.exe (evidence/oracle.json)."""
import argparse, shutil
from pathlib import Path
from common import ROOT, TC, ANALYSIS, ORACLE, identity, read_json, verify_manifest

def copy_checked(source,dest,expected):
    if identity(source)!={k:expected[k] for k in ('size','sha256')}: raise ValueError('Wrong setup input: '+str(source))
    dest.parent.mkdir(parents=True,exist_ok=True)
    if dest.exists() and identity(dest)==identity(source): return
    shutil.copyfile(source,dest)

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--source','--archive',dest='archive',type=Path,default=ROOT,
                    help='Root holding the locked inputs at their archive_path (default: this workspace)')
    ap.add_argument('--analysis-dir',type=Path)
    ap.add_argument('--toolchain-dir',type=Path,help='Alternative extracted TDM-2 mingw32 directory')
    ap.add_argument('--fixture',type=Path)
    a=ap.parse_args()
    archive=a.archive.resolve()
    for row in read_json(ROOT/'toolchain/lock.json')['files']:
        copy_checked((a.toolchain_dir/row['path']) if a.toolchain_dir else archive/row['archive_path'],TC/row['path'],row)
    analysis=read_json(ROOT/'toolchain/analysis.json')
    for row in analysis['files']:
        copy_checked((a.analysis_dir or Path(analysis['directory_hint']))/row['path'],ANALYSIS.parent/row['path'],row)
    oracle=read_json(ROOT/'evidence/oracle.json')
    copy_checked(a.fixture or archive/oracle['archive_path'],ORACLE,oracle['fixture'])
    print('Locked C toolchain, analysis tool and oracle installed under ignored build/local.')
if __name__=='__main__': main()
