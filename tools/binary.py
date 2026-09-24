"""Read PE32 and i386 COFF directly; preserve symbol/aux and relocation records."""
import struct
from pathlib import Path
from common import sha

class Binary:
    def __init__(self, path):
        self.path = Path(path)
        self.data = self.path.read_bytes()
        d = self.data
        self.pe = d[:2] == b'MZ'
        self.coff_offset = self.u32(0x3c) + 4 if self.pe else 0
        c = self.coff_offset
        if self.pe and d[c-4:c] != b'PE\0\0':
            raise ValueError('Bad PE signature')
        machine, ns, timestamp, symptr, nsyms, optsize, flags = struct.unpack_from('<HHIIIHH', d, c)
        if machine != 0x14c:
            raise ValueError(f'Expected i386 COFF, got {machine:x}')
        self.header = dict(machine=machine, section_count=ns, timestamp=timestamp,
                           symbol_table_pointer=symptr, symbol_table_entries=nsyms,
                           optional_header_size=optsize, characteristics=flags)
        strings = symptr + nsyms*18
        self.strings = d[strings:strings+self.u32(strings)] if symptr else b''
        self.image_base = 0
        self.optional = {}
        self.directories = []
        if optsize:
            o = c + 20
            if self.u16(o) != 0x10b:
                raise ValueError('Expected PE32')
            fields = [('magic',0,'H'), ('linker_major',2,'B'), ('linker_minor',3,'B'),
                      ('size_of_code',4,'I'), ('size_initialized_data',8,'I'), ('size_uninitialized_data',12,'I'),
                      ('entry_rva',16,'I'), ('base_of_code',20,'I'), ('base_of_data',24,'I'), ('image_base',28,'I'),
                      ('section_alignment',32,'I'), ('file_alignment',36,'I'),
                      ('os_major',40,'H'), ('os_minor',42,'H'), ('image_major',44,'H'), ('image_minor',46,'H'),
                      ('subsystem_major',48,'H'), ('subsystem_minor',50,'H'), ('win32_version',52,'I'),
                      ('size_of_image',56,'I'), ('size_of_headers',60,'I'), ('checksum',64,'I'),
                      ('subsystem',68,'H'), ('dll_characteristics',70,'H'), ('stack_reserve',72,'I'),
                      ('stack_commit',76,'I'), ('heap_reserve',80,'I'), ('heap_commit',84,'I'),
                      ('loader_flags',88,'I'), ('directory_count',92,'I')]
            self.optional = {name:struct.unpack_from('<'+fmt,d,o+offset)[0] for name,offset,fmt in fields}
            self.image_base = self.optional['image_base']
            names = ['export','import','resource','exception','certificate','base_relocation','debug','architecture','global_ptr','tls','load_config','bound_import','iat','delay_import','clr','reserved']
            for i in range(min(self.optional['directory_count'],16)):
                rva,size = struct.unpack_from('<II', d,o+96+i*8)
                self.directories.append({'name':names[i], 'rva':rva, 'size':size, 'present':bool(rva or size)})
        self.sections = []
        for i in range(ns):
            off = c+20+optsize+i*40
            name,vs,rva,rs,rp,relp,lnp,nrel,nln,chars = struct.unpack_from('<8sIIIIIIHHI',d,off)
            name = self.string_name(name)
            raw = d[rp:rp+rs] if rp else b''
            self.sections.append(dict(index=i+1,name=name,virtual_size=vs,rva=rva,raw_size=rs,raw_pointer=rp,
                                      relocation_pointer=relp,line_pointer=lnp,relocation_count=nrel,line_count=nln,
                                      characteristics=chars,sha256=sha(raw)))
        self.symbols = []
        self.by_index = {}
        i = 0
        current_file = None
        while i < nsyms:
            off = symptr+i*18
            rawname,value,sec,typ,storage,naux = struct.unpack_from('<8sIhHBB',d,off)
            name = self.string_name(rawname)
            aux = d[off+18:off+18*(1+naux)]
            if storage == 103:
                current_file = aux.split(b'\0')[0].decode('utf-8',errors='replace')
            symbol = dict(index=i,name=name,value=value,section=sec,type=typ,storage_class=storage,
                          file=current_file,aux_count=naux,aux_hex=aux.hex())
            if 0 < sec <= ns:
                symbol['va'] = self.image_base+self.sections[sec-1]['rva']+value
            self.symbols.append(symbol)
            self.by_index[i] = symbol
            i += 1+naux
        self.relocations = []
        for s in self.sections:
            for i in range(s['relocation_count']):
                off,idx,typ = struct.unpack_from('<IIH', d,s['relocation_pointer']+10*i)
                self.relocations.append({'section':s['index'],'offset':off,'symbol_index':idx,
                                         'symbol':self.by_index[idx]['name'],'type':typ})

    def u16(self, off): return struct.unpack_from('<H', self.data, off)[0]
    def u32(self, off): return struct.unpack_from('<I', self.data, off)[0]
    def string_name(self, raw):
        if raw[:4] == b'\0'*4:
            off = struct.unpack_from('<I',raw,4)[0]
            return self.strings[off:].split(b'\0')[0].decode('utf-8',errors='replace')
        name = raw.split(b'\0')[0].decode('utf-8',errors='replace')
        if name.startswith('/') and name[1:].isdigit():
            return self.strings[int(name[1:]):].split(b'\0')[0].decode('utf-8',errors='replace')
        return name
    def rva_offset(self,rva):
        for s in self.sections:
            delta = rva-s['rva']
            if 0 <= delta < s['raw_size'] and s['raw_pointer']:
                return s['raw_pointer']+delta
        if self.pe and rva < self.optional['size_of_headers']:
            return rva
        raise ValueError(f'RVA {rva:x} is not file backed')
    def at_va(self,va,size):
        off = self.rva_offset(va-self.image_base)
        return self.data[off:off+size]
    def section_bytes(self,s):
        return self.data[s['raw_pointer']:s['raw_pointer']+s['raw_size']] if s['raw_pointer'] else b''
    def cstring(self,rva):
        return self.data[self.rva_offset(rva):].split(b'\0')[0].decode('ascii',errors='replace')
    def directory(self,name):
        return next((d for d in self.directories if d['name']==name), {'rva':0,'size':0})
    def imports(self):
        directory = self.directory('import')
        if not directory['rva']: return []
        off = self.rva_offset(directory['rva'])
        result = []
        while True:
            original,timestamp,forwarder,name,iat = struct.unpack_from('<IIIII',self.data,off)
            if not any((original,timestamp,forwarder,name,iat)): break
            entries = []
            p = self.rva_offset(original or iat)
            i = 0
            while self.u32(p+i*4):
                value = self.u32(p+i*4)
                entries.append({'iat_va':self.image_base+iat+i*4,'ordinal':value&0xffff if value&0x80000000 else None,
                                'name':None if value&0x80000000 else self.cstring(value+2),
                                'hint':None if value&0x80000000 else self.u16(self.rva_offset(value))})
                i += 1
            result.append({'dll':self.cstring(name),'timestamp':timestamp,'forwarder_chain':forwarder,'entries':entries})
            off += 20
        return result
    def resources(self):
        directory = self.directory('resource')
        if not directory['rva']: return {'directories':[], 'leaves':[]}
        base = self.rva_offset(directory['rva'])
        dirs,leaves,seen = [],[],set()
        def walk(relative,path):
            if relative in seen: raise ValueError('Resource directory cycle')
            seen.add(relative)
            off = base+relative
            chars,stamp,major,minor,named,ids = struct.unpack_from('<IIHHHH',self.data,off)
            dirs.append(dict(path=path,offset=relative,characteristics=chars,timestamp=stamp,major=major,minor=minor,named_count=named,id_count=ids))
            for i in range(named+ids):
                name,child = struct.unpack_from('<II',self.data,off+16+8*i)
                if name&0x80000000:
                    p = base+(name&0x7fffffff)
                    key = self.data[p+2:p+2+2*self.u16(p)].decode('utf-16le')
                else: key=name
                if child&0x80000000:
                    walk(child&0x7fffffff,path+[key])
                else:
                    rva,size,codepage,reserved = struct.unpack_from('<IIII',self.data,base+child)
                    data = self.at_va(self.image_base+rva,size)
                    leaves.append(dict(path=path+[key],rva=rva,size=size,codepage=codepage,reserved=reserved,sha256=sha(data)))
        walk(0,[])
        return {'directories':dirs,'leaves':leaves}

    def census(self):
        return {'identity':{'size':len(self.data),'sha256':sha(self.data)},'coff_header':self.header,
                'optional_header':self.optional,'directories':self.directories,'sections':self.sections,
                'imports':self.imports(),'resources':self.resources(),
                'coff_files':[s for s in self.symbols if s['storage_class']==103],
                'coff_symbols':self.symbols,'coff_relocations':self.relocations,
                'dos_header_hex':self.data[:self.coff_offset-4].hex() if self.pe else None}
