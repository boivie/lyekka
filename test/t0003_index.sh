#!/bin/sh

test_description='Test indexing'

. ./test-lib.sh

cat >expected <<EOF
B 3cb6694588efb3ec8aea3e44e7f7dfe6853c5fd33a70e17cc43208d78d9ebccb
B 0bf20f15f8f9c33c67ea486ee7cf2a28df0b50662c5d3684ab1f61a9fa71779a
T 48ed1441e9fe3483309a070fce0185e124f2bbbd967e056fd372b6befeec31f0
EOF

test_expect_success 'simple tree' '
  (
    rm -f lyekka.db && 
    mkdir -p path1 &&
    echo "hello" > path1/hello &&
    touch -t 1001042104.01 -m -c path1/hello && 
    echo "world" > path1/world &&
    touch -t 1001042105.43 -m -c path1/world && 
    lyekka create &&
    lyekka path add path1 &&
    lyekka update-index &&
    lyekka gen-objects > actual &&
    test_cmp expected actual
  )'

test_done