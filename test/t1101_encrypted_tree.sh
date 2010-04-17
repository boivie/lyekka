#!/bin/sh

test_description='Tree creation'

. ./test-lib.sh

cat >tree.xml <<EOF
<tree>
 <treeref name="subdir_one" mode="040755" mtime="123457890" 
  sha="b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c" 
  key="5891b5b522d5df086d0ff0b110fbd9d21bb4fc7163af34d08286a2e846f6be03" />
 <treeref name="subdir_two" mode="040755" mtime="123457891" 
  sha="7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730" 
  key="e258d248fda94c63753607f7c4494ee0fcbe92f1a76bfdac795c9d84101eb317" />
</tree>
EOF

test_expect_success 'encrypt simple tree' '
  (
    lyekka mktree -e tree.xml > id &&
    SHA=`cut -d " " -f 1 id` &&
    type=`lyekka show-object -t $SHA` &&
    test "$type" = "encrypted tree"
  )'

cat >references <<EOF
7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730
b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c
EOF

test_expect_success 'get refs from encrypted tree' '
  (
    SHA=`cut -d " " -f 1 id` &&
    KEY=`cut -d " " -f 2 id` &&
    lyekka lstree --show refs $SHA > actual &&
    test_cmp references actual
  )'

cat >expected <<EOF
ref:7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730
ref:b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c
tree:b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c 16877 123457890 subdir_one
tree:7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730 16877 123457891 subdir_two
EOF

test_expect_success 'decrypt simple encrypted tree' '
  (
    SHA=`cut -d " " -f 1 id` &&
    KEY=`cut -d " " -f 2 id` &&
    lyekka lstree --show raw -K $KEY $SHA > actual &&
    test_cmp expected actual
  )'

test_done
