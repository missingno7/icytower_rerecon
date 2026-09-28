"""Object-function extraction, line-tagged listings and per-source-line comparison of a candidate
object against the oracle's exported assembly and line table (research tooling promoted to the
equivalence tier; verifier-only use of oracle bytes, which context.py --asm/--lines export)."""
import re, json, subprocess, collections, sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from common import ROOT, BUILD
from binary import Binary
OBJDUMP = str(ROOT/'build/local/analysis/objdump.exe')
ORACLE_DIR = BUILD/'equivalence/oracle'
DELTAS = {'play': 768, 'draw_frame': 584}   # canonical line - original line for the two frames
def oracle_files(fn):
    """Oracle asm/line-table exports for FN, regenerated with tools/context.py when missing."""
    ORACLE_DIR.mkdir(parents=True, exist_ok=True)
    out = {}
    for kind in ('asm', 'lines'):
        p = ORACLE_DIR/('%s_%s.json' % (fn, kind))
        if not p.exists():
            r = subprocess.run([sys.executable, str(ROOT/'tools/context.py'), fn, '--'+kind], capture_output=True, text=True, check=True)
            p.write_text(r.stdout, encoding='utf-8')
        out[kind] = json.load(open(p, encoding='utf-8'))
    return out

def functions(obj_path):
    obj=Binary(obj_path)
    text=next(s for s in obj.sections if s['name']=='.text')
    raw=obj.section_bytes(text)
    syms=sorted([s for s in obj.symbols if s['section']==text['index'] and s['type']==0x20 and not s['name'].startswith('.')],key=lambda s:s['value'])
    relocs=sorted(r['offset'] for r in obj.relocations if r['section']==text['index'])
    out={}
    for i,s in enumerate(syms):
        lo=s['value'];hi=syms[i+1]['value'] if i+1<len(syms) else len(raw)
        out[s['name'].lstrip('_')]=(lo,hi,raw,relocs)
    return out

def call_targets(fns_map,name):
    """Set of (offset, target_name) for e8/e9 rel32 whose target is a function start."""
    lo,hi,raw,relocs=fns_map[name];starts={}
    for n,(l,h,_,_) in fns_map.items(): starts[l]=n
    rel=set(relocs);out=[]
    for p in range(lo,hi-4):
        if raw[p] in (0xe8,0xe9) and (p+1) not in rel:
            disp=int.from_bytes(raw[p+1:p+5],'little',signed=True);t=p+5+disp
            if t in starts: out.append((p-lo,starts[t]))
    return out

def normalized(fns_map,name):
    """Function bytes with relocation fields and resolved same-section call fields zeroed,
    plus the list of (offset,target) call identities."""
    lo,hi,raw,relocs=fns_map[name];code=bytearray(raw[lo:hi])
    for r in relocs:
        p=r-lo
        if 0<=p<len(code): code[p:p+4]=b'\0'*4
    calls=call_targets(fns_map,name)
    for p,_ in calls: code[p+1:p+5]=b'\0'*4
    return bytes(code),calls

def peers_changed(obj_path,ref_obj_path,skip=('play','draw_frame')):
    a=functions(obj_path);b=functions(ref_obj_path);changed=[]
    for fn in a:
        if fn in skip or fn not in b: continue
        if normalized(a,fn)!=normalized(b,fn): changed.append(fn)
    return changed

def lines_for(obj):
    r=subprocess.run([OBJDUMP,'--dwarf=decodedline',str(obj)],capture_output=True,text=True)
    rows=[]
    for l in r.stdout.split('\n'):
        m=re.match(r'\s*(\S+)\s+(\d+)\s+(0x[0-9a-f]+|\d+)\s',l)
        if m: rows.append((int(m.group(3),16) if m.group(3).startswith('0x') else int(m.group(3)),m.group(1),int(m.group(2))))
    return rows

def norm_ops(mn,ops,target_line,callname):
    if mn.startswith('j') or mn=='call':
        if mn=='call': return mn+' '+(callname or 'IND')
        return mn+' ->'+(str(target_line) if target_line is not None else '?')
    ops=re.sub(r'0x[0-9a-f]{6,8}\b','ABS',ops)   # absolute addresses (original) 
    ops=re.sub(r'\$0x[0-9a-f]{6,8}\b','$ABS',ops)
    return (mn+' '+ops).strip()

def original_listing(fn):
    f=oracle_files(fn);a=f['asm'];l=f['lines'];va=int(a['va'],16)
    tags={}
    for e in l['lines']: tags.setdefault(e['address']-va,[]).append((e['file'],e['line']))
    ins=[];cur=None
    for i in a['assembly']:
        off=i['address']-va
        if off in tags: cur=tags[off][-1]
        ins.append({'off':off,'line':cur,'mn':i['mnemonic'],'text':i['assembly'],'bytes':i['bytes']})
    return ins

def candidate_listing(obj,fn,delta):
    fns=functions(obj);lo,hi,raw,rel=fns[fn]
    r=subprocess.run([OBJDUMP,'-d','-r','--start-address=%d'%lo,'--stop-address=%d'%hi,str(obj)],capture_output=True,text=True)
    tags={}
    for a,f,ln in lines_for(obj):
        if lo<=a<hi: tags.setdefault(a-lo,[]).append((f,ln-delta if f=='main.c' else ln))
    ins=[];cur=None;pending_reloc=None
    for l in r.stdout.split('\n'):
        m=re.match(r'\s*([0-9a-f]+):\s*((?:[0-9a-f]{2} )+)\s*(\S+)\s*(.*)',l)
        if m and re.fullmatch(r'[0-9a-f]{2}',m.group(3)) and not m.group(4).strip():
            # continuation line of a long instruction: only bytes, no mnemonic
            if ins: ins[-1]['bytes']+=m.group(2).replace(' ','')+m.group(3)
            continue
        if m:
            off=int(m.group(1),16)-lo
            if off in tags: cur=tags[off][-1]
            ins.append({'off':off,'line':cur,'mn':m.group(3),'text':(m.group(3)+' '+m.group(4)).strip(),'bytes':m.group(2).replace(' ',''),'reloc':None})
        else:
            mr=re.match(r'\s*([0-9a-f]+):\s+(dir32|DISP32|R_386_\w+|IMAGE_REL_\w+)\s+(\S+)',l)
            if mr and ins: ins[-1]['reloc']=mr.group(3); ins[-1]['reloc_field']=int(mr.group(1),16)-lo-ins[-1]['off']
    return ins

def annotate(ins,is_cand):
    """Add normalized text with jump targets mapped to lines and call names."""
    byoff={i['off']:i for i in ins}
    for i in ins:
        mn=i['mn'];text=i['text'];tl=None;cn=None
        m=re.search(r'<_?([A-Za-z_][\w@]*)(?:\+0x([0-9a-f]+))?>',text)
        if mn.startswith('j') or mn=='call':
            if m:
                name=m.group(1);addend=int(m.group(2),16) if m.group(2) else 0
                if mn=='call': cn=name
                else:
                    t=byoff.get(addend)
                    tl=t['line'][1] if t and t['line'] else None
            if mn=='call' and is_cand and i.get('reloc'): cn=i['reloc'].lstrip('_')
        ops=text.split(None,1)[1] if ' ' in text else ''
        ops=re.sub(r'\s*<.*>','',ops)
        if is_cand and i.get('reloc') and mn!='call':
            fld=i.get('reloc_field'); b=bytes.fromhex(i['bytes']); done=False
            if fld is not None and 0<=fld and fld+4<=len(b):
                val=int.from_bytes(b[fld:fld+4],'little')
                toks=list(re.finditer(r'\$?0x[0-9a-f]+',ops))
                match=[t for t in toks if int(t.group(0).lstrip('$'),16)==val]
                if len(match)>1:
                    pref=[t for t in match if (t.group(0).startswith('$'))==(i['reloc'].startswith('.rdata'))]
                    match=pref or match
                if match:
                    t=match[0]; ops=ops[:t.start()]+('$ABS' if t.group(0).startswith('$') else 'ABS')+ops[t.end():]; done=True
            if not done: ops=re.sub(r'\$?0x[0-9a-f]+',lambda m:'$ABS' if m.group(0).startswith('$') else 'ABS',ops,count=1)
        i['norm']=norm_ops(mn,ops,tl,cn)
    return ins

def compare(fn,obj,delta,show_same=False,detail_lines=None):
    o=annotate(original_listing(fn),False);c=annotate(candidate_listing(obj,fn,delta),True)
    def group(ins):
        g=collections.OrderedDict()
        for i in ins:
            key=i['line'][1] if i['line'] else -1
            g.setdefault(key,[]).append(i)
        return g
    go,gc=group(o),group(c)
    lines=sorted(set(go)|set(gc))
    same=diff=0;report=[]
    for ln in lines:
        a=[i['norm'] for i in go.get(ln,[])];b=[i['norm'] for i in gc.get(ln,[])]
        if a==b: same+=1; status='='
        elif collections.Counter(a)==collections.Counter(b): status='~order'; diff+=1
        else: status='X'; diff+=1
        if status!='=' or show_same or (detail_lines and ln in detail_lines):
            report.append((ln,status,len(a),len(b),a,b,[i['off'] for i in go.get(ln,[])],[i['off'] for i in gc.get(ln,[])]))
    return same,diff,report,o,c

ORIG_BRACE = {'play': 3407, 'draw_frame': 2491}   # original line the frame's opening-brace line maps to
def delta_for(obj, fn):
    """Empirical line delta: candidate prologue line minus the frame's brace line (independent of how
    the body was materialised into the TU)."""
    ins = candidate_listing(obj, fn, 0)
    first = next((i['line'][1] for i in ins if i['line'] and i['line'][0] == 'main.c'), None)
    if first is None: return DELTAS.get(fn, 0)
    return first - ORIG_BRACE[fn]
