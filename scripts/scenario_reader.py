"""Read-only layout-driven scenario reader; section rules from mjolnir-core docs.
No game process access. Offsets describe serialized files, not runtime memory.
"""
import struct

class Scenario:
    def __init__(self, data, group=b'rncs'):
        self.data=data
        assert data[0x30:0x34]==group and data[0x3c:0x44]==b'MALB!gat'
        assert self.u32(0x48)+0x4c==len(data)
        m,v,c,n=self.section(0x4c);assert m=='blay'
        self.bdat=c+n
        m,v,c,n=self.section(0xa4);assert m=='tgly'
        end=c+n;tables={}
        while c<end:
            m,v,s,n=self.section(c);tables[m]=data[s:s+n];c=s+n
        assert c==end
        def string(o):return tables['str*'][o:].split(b'\0')[0].decode()
        types=[(string(a),b) for a,b,_ in struct.iter_unpack('<III',tables['tgft'])]
        self.fields=[(string(a),types[b][0],c,types[b][1]) for a,b,c in struct.iter_unpack('<III',tables['gras'])]
        self.structs=[(string(a),b) for _,a,b,_ in struct.iter_unpack('<16sIII',tables['stv4'])]
        self.blocks=[(string(a),b,c) for a,b,c in struct.iter_unpack('<III',tables['blv2'])]
        self.arrays=list(struct.iter_unpack('<III',tables['arr!']))
        m,v,c,n=self.section(self.bdat);assert m=='bdat' and c+n==len(data)
        self.root_block=c
        m,v,c,n=self.section(c);assert m=='tgbl'
        assert self.u32(c)==1 and self.u32(c+4)==0
        self.root_data=c+8
        self.root_wrapper=self.root_data+self.struct_size(0)
        m,v,c,n=self.section(self.root_wrapper);assert m=='tgst' and v==n
        self.rows=[]
        assert self.walk(self.run(0),c,self.root_data)==c+n
    def u32(self,o):return struct.unpack_from('<I',self.data,o)[0]
    def section(self,o):
        m,v,n=struct.unpack_from('<4sII',self.data,o)
        assert o+12+n<=len(self.data)
        return m[::-1].decode(),v,o+12,n
    def run(self,s):
        out=[]
        for f in self.fields[self.structs[s][1]:]:
            if f[1]=='terminator X':return out
            out.append(f)
        raise ValueError('Unterminated struct')
    def struct_size(self,s):return sum(self.size(f) for f in self.run(s))
    def size(self,f):
        _,t,a,n=f
        if t=='pad':return a
        if t=='struct':return self.struct_size(a)
        if t=='array':return self.arrays[a][1]*self.struct_size(self.arrays[a][2])
        return n
    def walk(self,fs,pos,base,prefix=''):
        off=0
        for f in fs:
            name,t,a,n=f;path=prefix+name
            if t=='array':
                _,count,s=self.arrays[a]
                for i in range(count):pos=self.walk(self.run(s),pos,base+off+i*self.struct_size(s),path+f'[{i}].')
            elif t in ('block','struct','string id','data','tag reference','pageable resource'):
                m,v,c,n=self.section(pos)
                expected={'block':'tgbl','struct':'tgst','string id':'tgsi','data':'tgda','tag reference':'tgrf'}.get(t)
                if expected:assert m==expected,(path,m,expected)
                row=dict(name=path,type=t,packed_offset=base+off,section=pos,length=n)
                if t=='block':
                    count,flags=struct.unpack_from('<II',self.data,c)
                    size=self.struct_size(self.blocks[a][2])
                    assert count<=self.blocks[a][1] and flags in (0,1) and 8+count*size<=n
                    assert self.u32(base+off)==count,(path,'inline count')
                    row.update(count=count,flags=flags,element_size=size,struct=self.blocks[a][2],element_data=c+8)
                self.rows.append(row);pos=c+n
            off+=self.size(f)
        return pos
    def squads(self):return next(r for r in self.rows if r['name']=='squads')
    def squad_parts(self):
        s=self.squads();a=s['element_data'];b=a+s['count']*s['element_size']
        packed=self.data[a:b];wrappers=[];pos=b
        if s['flags']==0:
            for _ in range(s['count']):
                m,v,c,n=self.section(pos);assert m=='tgst' and v==n
                wrappers.append(self.data[pos:c+n]);pos=c+n
        assert pos==s['section']+12+s['length']
        return packed,wrappers

def section(magic,version,content):
    return struct.pack('<4sII',magic[::-1].encode(),version,len(content))+content
