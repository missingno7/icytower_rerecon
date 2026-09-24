"""Small body overlays; materialize whole historical context only in scratch."""
import contextlib, re, shutil, tempfile
from pathlib import Path
from common import ROOT, BUILD, identity, sha, read_json
from build import targets
from source_scope import function_span, sanitized

def resolve_function(name,target=None):
    from evidence import Evidence
    if '::' in name:
        target,name=name.split('::',1)
    db=Evidence();configs=targets();matches=[]
    for t,c in configs.items():
        if target and t!=target:continue
        for f in db.functions(c['historical_cu']):
            if f['name']==name:matches.append((t,c,f))
    db.close()
    if len(matches)!=1:raise ValueError('Expected one historical function; specify game-target::function: '+str([t for t,_,_ in matches]))
    return matches[0]

def fragment(path,name):
    data=Path(path).read_bytes();text=data.decode('cp1252');clean=sanitized(text).strip()
    if clean.startswith('{'):
        # A body must consume the entire fragment; forbid trailing declarations.
        opening=text.index('{');depth=0;closing=None
        for i,c in enumerate(sanitized(text)[opening:],opening):
            depth+=(c=='{')-(c=='}')
            if depth==0:closing=i+1;break
        if closing is None or sanitized(text[closing:]).strip():raise ValueError('Expected exactly one braced body')
        return text[opening:closing].encode('cp1252')
    raise ValueError('Candidate must be a braced function body; the historical signature stays in its TU')

def overlay(source,name,body):
    text=source.decode('cp1252');lo,hi=function_span(text,name)
    # Byte offsets equal string offsets in cp1252; preserve all surrounding bytes.
    return source[:lo]+body+source[hi:]

def acceptance_syntax(body):
    # C removes escaped newlines before tokenization; inspect that same spelling.
    clean=sanitized(re.sub(r'\\\r?\n','',body.decode('cp1252')))
    if '#' in clean or re.search(r'\b(?:asm|__asm(?:__)?|__attribute(?:__)?|__declspec|volatile|__volatile(?:__)?|_Pragma|__pragma)\b',clean):
        raise ValueError('Promotion refuses preprocessor directives, invented asm/attributes/volatile in candidate bodies')

def patched_source(path,target,strict=False):
    """A small, exact replacement patch for one historical TU; no path execution."""
    patch=read_json(path);config=targets()[target];source=(ROOT/config['source']).read_bytes()
    if patch.get('source')!=config['source'] or patch.get('base_sha256')!=sha(source):
        raise ValueError('Patch does not identify the current owning TU')
    text=source.decode('cp1252')
    if not patch.get('edits'):raise ValueError('Empty TU patch')
    for edit in patch['edits']:
        old,new=edit['old'],edit['new']
        if not old or text.count(old)!=1:raise ValueError('Patch old text must occur exactly once')
        if strict:acceptance_syntax(new.encode('cp1252'))
        text=text.replace(old,new,1)
    return text.encode('cp1252')

@contextlib.contextmanager
def materialized(target,name=None,body=None,source_override=None):
    config=targets()[target]
    BUILD.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='scratch-',dir=BUILD) as folder:
        root=Path(folder)
        # Copy inputs; no junctions/hardlinks to canonical files. Temporary TUs are
        # disposed at exit, and never become experiment-history artifacts.
        for directory in ['src','include','third_party']:
            shutil.copytree(ROOT/directory,root/directory)
        source=root/config['source']
        if source_override is not None:source.write_bytes(source_override)
        elif body is not None:source.write_bytes(overlay(source.read_bytes(),name,body))
        yield root
