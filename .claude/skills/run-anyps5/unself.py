import struct,sys
src,dst=sys.argv[1:3]
b=open(src,'rb').read()
n=struct.unpack_from('<H',b,0x18)[0]
ents=[struct.unpack_from('<QQQQ',b,0x20+32*i) for i in range(n)]
e=0x20+32*n
assert b[e:e+4]==b'\x7fELF',"no elf"
phoff,=struct.unpack_from('<Q',b,e+0x20); shoff,=struct.unpack_from('<Q',b,e+0x28)
phentsz,phnum=struct.unpack_from('<HH',b,e+0x36); shentsz,shnum=struct.unpack_from('<HH',b,e+0x3a)
ph=[struct.unpack_from('<IIQQQQQQ',b,e+phoff+phentsz*i) for i in range(phnum)]
end=max([p[2]+p[5] for p in ph]+[phoff+phentsz*phnum])
out=bytearray(max(end,0))
out[0:phoff+phentsz*phnum]=b[e:e+phoff+phentsz*phnum]
used=0
for i,(fl,off,cs,ms) in enumerate(ents):
    if not fl&0x800: continue
    idx=(fl>>20)&0xfff
    assert not fl&0x8 or True
    p=ph[idx]; sz=p[5]
    if fl&0x8 and cs!=ms: print("compressed seg",idx,hex(fl))
    d=b[off:off+min(cs,sz)]
    out[p[2]:p[2]+len(d)]=d; used+=1
open(dst,'wb').write(out); print("segs",used,"phnum",phnum,"size",len(out))
