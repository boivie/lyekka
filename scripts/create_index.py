import sys, struct, hashlib


if len(sys.argv) < 2:
    print>>sys.stderr, "Syntax: %s <pack>" % sys.argv[0]
    sys.exit(1)

for fname in sys.argv[1:]:
    print("Handling %s" % fname)
    inf = open(fname, "rb")
    outf = open(fname + ".idx", "wb")

    # Read and validate superblock
    sb = inf.read(64)
    assert(len(sb) == 64)
    head_magic = "\x32\x7d\x25\x2e\x42\x18\x77\x3a\x5b\x47\xc8\x58\x4d\xf3\x10\x2d"
    assert(sb[0:16] == head_magic)
    version, superblock_size = struct.unpack(">II", sb[16:24])
    assert(version == 1)
    assert(superblock_size == 64)
    assert(hashlib.sha1(sb[0:(64 - 20)]).digest() == sb[64 - 20:])

    # Write index superblock
    isb = "\xca\xf7\xe7\xbf\x96\xa7\x99\xf0\x54\x74\x69\xcb\x69\x8b\xfb\x68"
    isb += "\x00" * (32 - len(isb))
    outf.write(isb)

    # Handle all chunks
    chunk_idx = 0
    chunk_magic = "\x6f\x66\xac\x53\x1e\xb1\x46\xef\xe2\xec\x95\x86\x04\x32\xcb\x85"
    footer_size = 32
    try:
        while True:
            offset = inf.tell()
            if offset % 32 != 0:
                raise Exception("Invalid offset")
            head = inf.read(64)
            if len(head) == 0:
                break
            if len(head) != 64:
                raise Exception("Truncated header")
            if head[0:16] != chunk_magic:
                raise Exception("Invalid magic")
            chunk_id = head[16:36]

            payload_size, header_size, flags  = struct.unpack(">III", head[36:48])
            if header_size != 64:
                raise Exception("Invalid header size")
            adjusted_size = payload_size + ((32 - payload_size) % 32)
            inf.seek(adjusted_size + 24, 1)
            footer = inf.read(8)
            if footer != "\x5d\x6d\xcc\x7d\xfe\x63\x1e\x4f":
                raise Exception("Invalid footer")

            # index entry
            total_size = header_size + adjusted_size + footer_size
            entry = chunk_id + struct.pack(">III", offset, total_size, flags)
            outf.write(entry)
            chunk_idx += 1

    except KeyError, e:
        print >>sys.stderr, "error: Parsing stopped at chunk index %d," % chunk_idx
        print >>sys.stderr, "reason: %s" % e

    print >>sys.stderr, "Wrote %d entries" % chunk_idx
