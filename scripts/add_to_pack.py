import sys, struct, hashlib, zlib


if len(sys.argv) != 3:
    print>>sys.stderr, "Syntax: %s <pack> <file>" % sys.argv[0]
    sys.exit(1)

outf = open(sys.argv[1], "ab")
inf = open(sys.argv[2], "rb")
CHUNK_SIZE = 4 * 1024 * 1024

if outf.tell() == 0:
    # Create superblock.
    sb = "\x32\x7d\x25\x2e\x42\x18\x77\x3a\x5b\x47\xc8\x58\x4d\xf3\x10\x2d"
    sb += struct.pack(">II", 1, 64)
    sb += "\x00" * 20
    sb += hashlib.sha1(sb).digest()
    assert(len(sb) == 64)
    outf.write(sb)

while True:
    # Snag a chunk from the file.
    payload = inf.read(CHUNK_SIZE)
    if len(payload) == 0:
        break

    hash_obj = hashlib.sha1(payload)
    print>>sys.stderr, hash_obj.hexdigest()
    header = "\x6f\x66\xac\x53\x1e\xb1\x46\xef\xe2\xec\x95\x86\x04\x32\xcb\x85"
    header += hash_obj.digest()
    header += struct.pack(">III", len(payload), 64, 0)
    header += "\x00" * 16
    assert(len(header) == 64)
    padding = "\x00" * ((32 - len(payload)) % 32)
    footer = "\x5d\x6d\xcc\x7d\xfe\x63\x1e\x4f"
    outf.write(header)
    outf.write(payload)
    outf.write(padding)
    outf.write(hashlib.sha1(header + payload + padding).digest())
    outf.write(struct.pack(">i", zlib.crc32(header + payload + padding)))
    outf.write(footer)
