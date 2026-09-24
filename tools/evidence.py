"""Regenerable, indexed oracle facts. One raw authority: the hash-locked PE."""
import argparse, json, sqlite3, zlib
from functools import lru_cache
from pathlib import Path
from common import ROOT, BUILD, ORACLE, ANALYSIS, identity, read_json, write_json, digest, run, verify_manifest, exclusive
from binary import Binary
from dwarf import parse, integer, file_tables, line_rows, address_lists

DB = BUILD / 'evidence.sqlite'
RECEIPT = BUILD / 'evidence-cache.json'
def verify_oracle(path=ORACLE):
    expected=read_json(ROOT/'evidence/oracle.json')
    if identity(path)!=expected['fixture']: raise ValueError('Wrong oracle fixture')
    if 'pe' in expected:
        b=Binary(path)
        if {'header':b.header,'optional':b.optional,'sections':b.sections}!=expected['pe']:
            raise ValueError('Oracle PE identities differ')
def verify_analysis(): verify_manifest(ANALYSIS.parent,read_json(ROOT/'toolchain/analysis.json')['files'],exact=True)
def extraction_id():
    return digest({'fixture':read_json(ROOT/'evidence/oracle.json')['fixture'],
       'analysis':read_json(ROOT/'toolchain/analysis.json'),
       'parsers':{n:identity(ROOT/'tools'/n) for n in ['evidence.py','dwarf.py','binary.py','common.py']}})
def pack(value): return zlib.compress(json.dumps(value,separators=(',',':')).encode(),6)
def unpack(value): return json.loads(zlib.decompress(value))

def regenerate():
    verify_oracle();verify_analysis()
    with exclusive(BUILD/'evidence.lock'):
        signature=extraction_id()
        binary=Binary(ORACLE)
        dies,summary=parse(run([ANALYSIS,'--dwarf=info',ORACLE]))
        tables=file_tables(run([ANALYSIS,'--dwarf=rawline',ORACLE]))
        ti={t['offset']:t for t in tables}
        for d in dies.values():
            stmt=integer(dies[d['cu']]['attrs'].get('DW_AT_stmt_list'))
            fileno=integer(d['resolved'].get('DW_AT_decl_file'))
            d['decl_file_path']=ti.get(stmt,{}).get('files',{}).get(fileno,{}).get('path')
        pending=DB.with_suffix('.pending');pending.unlink(missing_ok=True)
        con=sqlite3.connect(pending)
        try:
            con.executescript('''
              CREATE TABLE meta(key TEXT PRIMARY KEY,value BLOB NOT NULL);
              CREATE TABLE dies(id INTEGER PRIMARY KEY,cu INTEGER,parent INTEGER,tag TEXT,name TEXT,low INTEGER,high INTEGER,data BLOB NOT NULL);
              CREATE INDEX die_cu ON dies(cu); CREATE INDEX die_parent ON dies(parent); CREATE INDEX die_name ON dies(name);
              CREATE TABLE units(id INTEGER PRIMARY KEY,path TEXT,data BLOB NOT NULL);
              CREATE TABLE functions(die INTEGER PRIMARY KEY,cu TEXT,name TEXT,va INTEGER,size INTEGER,data BLOB NOT NULL);
              CREATE INDEX fn_cu ON functions(cu); CREATE INDEX fn_name ON functions(name); CREATE INDEX fn_va ON functions(va);
              CREATE TABLE lines(address INTEGER,file TEXT,line INTEGER,data BLOB NOT NULL);
              CREATE INDEX line_address ON lines(address);
              CREATE TABLE lists(kind TEXT,offset INTEGER,data BLOB NOT NULL,PRIMARY KEY(kind,offset));
              CREATE TABLE files(offset INTEGER PRIMARY KEY,data BLOB NOT NULL);
              CREATE TABLE symbols(id INTEGER PRIMARY KEY,name TEXT,va INTEGER,data BLOB NOT NULL);
              CREATE INDEX symbol_name ON symbols(name); CREATE INDEX symbol_va ON symbols(va);
            ''')
            for d in dies.values():
                con.execute('INSERT INTO dies VALUES(?,?,?,?,?,?,?,?)',(d['offset'],d['cu'],d['parent'],d['tag'],d['name'],d['low_pc'],d['high_pc'],pack(d)))
                if d['tag']=='DW_TAG_compile_unit':
                    row={'die':d['offset'],'path':d['name'],'low_pc':d['low_pc'],'high_pc':d['high_pc'],'producer':d['resolved'].get('DW_AT_producer')}
                    con.execute('INSERT INTO units VALUES(?,?,?)',(d['offset'],d['name'],pack(row)))
                if d['tag']=='DW_TAG_subprogram' and d['low_pc'] is not None and d['high_pc'] is not None:
                    cu=dies[d['cu']]['name'];size=d['high_pc']-d['low_pc']
                    if size<=0: continue
                    row={'name':d['name'],'die':d['offset'],'va':d['low_pc'],'size':size,'compile_unit':cu,'type_ref':d['type_ref'],'decl_file_path':d['decl_file_path'],'parameters':[c for c in d['children'] if dies[c]['tag']=='DW_TAG_formal_parameter']}
                    con.execute('INSERT INTO functions VALUES(?,?,?,?,?,?)',(d['offset'],cu,d['name'],d['low_pc'],size,pack(row)))
            # PE, all symbol/aux entries, imports, resources remain one compact record; narrow
            # function queries never read this. The Binary reader can inspect the original directly.
            pe=binary.census();pe.pop('symbols',None)
            for symbol in binary.symbols:
                con.execute('INSERT INTO symbols VALUES(?,?,?,?)',(symbol['index'],symbol['name'],symbol.get('va'),pack(symbol)))
            for key,value in [('extraction',signature),('summary',summary),('pe',pe)]:
                con.execute('INSERT INTO meta VALUES(?,?)',(key,pack(value)))
            for row in line_rows(run([ANALYSIS,'--dwarf=decodedline',ORACLE])):
                con.execute('INSERT INTO lines VALUES(?,?,?,?)',(row['address'],row['file'],row['line'],pack(row)))
            for table in tables:con.execute('INSERT INTO files VALUES(?,?)',(table['offset'],pack(table)))
            for kind,section in [('locations','.debug_loc'),('ranges','.debug_ranges')]:
                for row in address_lists(binary,section): con.execute('INSERT INTO lists VALUES(?,?,?)',(kind,row['offset'],pack(row)))
            con.commit()
            if con.execute('PRAGMA integrity_check').fetchone()[0]!='ok':raise ValueError('Evidence SQLite integrity failure')
        finally: con.close()
        verify_oracle();verify_analysis()
        if extraction_id()!=signature:raise ValueError('Evidence parser changed during extraction')
        pending.replace(DB)
        write_json(RECEIPT,{'extraction':signature,'database':identity(DB)})
        print('Regenerated oracle index:',summary['die_count'],'DIEs,',DB.stat().st_size,'bytes',flush=True)

class Evidence:
    def __init__(self):
        verify_oracle();verify_analysis()
        if not DB.exists() or not RECEIPT.exists() or read_json(RECEIPT).get('extraction')!=extraction_id():regenerate()
        if identity(DB)!=read_json(RECEIPT)['database']:raise ValueError('Evidence cache drift; regenerate with evidence.py --rebuild')
        self.connection=sqlite3.connect(DB.as_uri()+'?mode=ro',uri=True)
    def rows(self,sql,args=()):return [unpack(r[0]) for r in self.connection.execute(sql,args)]
    def functions(self,cu=None):
        return self.rows('SELECT data FROM functions'+(' WHERE cu=?' if cu else '')+' ORDER BY va',(cu,) if cu else ())
    def unit(self,cu):return self.rows('SELECT data FROM units WHERE path=?',(cu,))[0]
    def die(self,offset):return self.rows('SELECT data FROM dies WHERE id=?',(offset,))[0]
    def dies(self,cu):
        uid=self.unit(cu)['die'];rows=self.rows('SELECT data FROM dies WHERE cu=?',(uid,));result={r['offset']:r for r in rows}
        # Include cross-CU reference closure if one ever appears; retain its children too.
        todo=list(rows)
        while todo:
            d=todo.pop()
            for ref in [d.get('type_ref'),*d.get('children',[])]:
                if ref is not None and ref not in result:
                    target=self.die(ref);result[ref]=target;todo.append(target)
        return result
    def globals(self,cu):
        uid=self.unit(cu)['die']
        return [{'die':d['offset'],'name':d['name'],'type_ref':d['type_ref'],'cu':cu,'address':d['address'],'scope':d['parent'],'decl_file_path':d['decl_file_path'],'attrs':d['resolved']}
          for d in self.rows("SELECT data FROM dies WHERE cu=? AND parent=? AND tag='DW_TAG_variable'",(uid,uid)) if d.get('address') is not None]
    def lines(self,start,stop):return self.rows('SELECT data FROM lines WHERE address>=? AND address<? ORDER BY address',(start,stop))
    def address_list(self,kind,offset):return self.rows('SELECT data FROM lists WHERE kind=? AND offset=?',(kind,offset))
    def close(self):self.connection.close()
    def __del__(self):
        if hasattr(self,'connection'):self.connection.close()

if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--rebuild',action='store_true')
    ap.add_argument('--die',type=lambda x:int(x,0));ap.add_argument('--symbol')
    ap.add_argument('--location',type=lambda x:int(x,0));ap.add_argument('--range',dest='range_offset',type=lambda x:int(x,0))
    ap.add_argument('--pe',action='store_true');a=ap.parse_args()
    if a.rebuild:regenerate()
    db=Evidence()
    if a.die is not None:value=db.die(a.die)
    elif a.symbol:value=db.rows('SELECT data FROM symbols WHERE name=?',(a.symbol,))
    elif a.location is not None:value=db.address_list('locations',a.location)
    elif a.range_offset is not None:value=db.address_list('ranges',a.range_offset)
    elif a.pe:value=db.rows("SELECT value FROM meta WHERE key='pe'")[0]
    else:value=db.rows("SELECT value FROM meta WHERE key='summary'")[0]
    print(json.dumps(value,indent=2));db.close()
