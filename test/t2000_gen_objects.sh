#!/bin/sh

test_description='Generate objects from directory'

. ./test-lib.sh

cat >expected <<EOF
B 0bf20f15f8f9c33c67ea486ee7cf2a28df0b50662c5d3684ab1f61a9fa71779a
B 3cb6694588efb3ec8aea3e44e7f7dfe6853c5fd33a70e17cc43208d78d9ebccb
T 63858e3186d778235ce383aa6719cd777828e3f464200e3a3a826953661c2564
EOF

test_expect_success 'simple tree' '
  (
    mkdir -p path1 &&
    echo "hello" > path1/hello &&
    touch -t 1001042104.01 -m -c path1/hello && 
    echo "world" > path1/world &&
    touch -t 1001042105.43 -m -c path1/world && 
    chmod 755 path1/hello path1/world && 
    lyekka gen-objects -i path1 > actual &&
    sort actual > actual.sorted &&
    test_cmp expected actual.sorted
  )'

cat >expected <<EOF
B 0bf20f15f8f9c33c67ea486ee7cf2a28df0b50662c5d3684ab1f61a9fa71779a
B 0bf20f15f8f9c33c67ea486ee7cf2a28df0b50662c5d3684ab1f61a9fa71779a
B 3cb6694588efb3ec8aea3e44e7f7dfe6853c5fd33a70e17cc43208d78d9ebccb
B 3cb6694588efb3ec8aea3e44e7f7dfe6853c5fd33a70e17cc43208d78d9ebccb
B 65bd5076c3d67e0d6a40d9e7e5bb6500da9a538bda6eea15cd60315a22215784
B 72287d4ae97d533db9ebd97cd64df7ee53c80366ced90c0e0230677eac059e17
T 0214df8fe2549ff1ddd499bf5e3d08eb48bd0ef87f03e94c66be8164ab2bd6cc
T 079abedcb77e8901b14e6d49c255bf3d561d4c48dfba8b384e01b8e57fc315fd
T 208454e49f01f08dc59f98462c1f0114a1db2573c348127aa7857f5265134c0c
T 8be19b5408482123df6892b16d770abdc22f3c3adaaf4ce58fdb4f6a4e22b9cc
T 97128982ddde77aef406b62ae12fed0a8e5937b83ac19ee51780b42b36bc00be
T d5701346e9d6f3aa2855b771b354c087508d5fcc2ad0fc0773c122e7327e7b88
EOF

# Build upon the more advanced one and make it larger
test_expect_success 'tree with subdirs' '
  (
    mkdir -p path1/path2/path3 &&
    mkdir -p path1/path4 &&
    mkdir -p path1/path5/path6 &&
    echo "hello" > path1/path4/hello &&
    echo "world" > path1/world2 &&
    echo "fum" >   path1/path5/path6/fum &&
    echo "gazonk" > path1/gazonk &&
    find path1 | xargs touch -t 1001042104.01 -m -c &&
    lyekka gen-objects -i path1 > actual &&
    sort actual > actual.sorted &&
    test_cmp expected actual.sorted
  )'


test_done