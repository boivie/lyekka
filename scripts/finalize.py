import sys, os, glob, re


if len(sys.argv) != 2:
    print>>sys.stderr, "Syntax: %s <partial-pack>" % sys.argv[0]
    sys.exit(1)

# First find out the first available pack.
base = os.path.realpath(os.path.join(sys.argv[1], os.pardir))
pack_no = -1
re_match = re.compile(r"^pack-([0-9]+)$")
for f in glob.glob(os.path.join(base, "pack-*")):
    m = re_match.match(os.path.basename(f))
    if not m:
        continue
    num = int(m.groups()[0])
    if num > pack_no:
        pack_no = num


pack_no += 1
pack_fname = "pack-%012d" % pack_no
assert(not os.path.exists(pack_fname))
print>>sys.stderr, "Finalizing into %s" % pack_fname

os.rename(sys.argv[1], pack_fname)
index = sys.argv[1] + ".idx"
if os.path.exists(index):
    os.rename(index, pack_fname + ".idx")
