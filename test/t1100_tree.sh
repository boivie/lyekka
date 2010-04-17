#!/bin/sh

test_description='Tree creation'

. ./test-lib.sh

cat >tree.xml <<EOF
<tree>
 <treeref name="subdir_one" mode="040755" mtime="123457890" 
          sha="b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c"/>
 <treeref name="subdir_two" mode="040755" mtime="123457891"
          sha="7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730"/>
</tree>
EOF

echo "e5f7d3b6f329907830b479aefd5ef41815a8ab6d41a12139e0e54eee79389fad" > ref_sha

test_expect_success 'creating simple tree' '
  (
    lyekka mktree tree.xml > sha &&
    test_cmp ref_sha sha 
  )'

cat >expected <<EOF
ref:7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730
ref:b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c
tree:b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c 16877 123457890 subdir_one
tree:7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730 16877 123457891 subdir_two
EOF

test_expect_success 'listing simple tree' '
  (
    lyekka lstree --show raw `cat sha` > actual &&
    test_cmp expected actual 
  )'

cat >tree2.xml <<EOF
<tree>
 <treeref name="subdir_one" mode="040755" mtime="123457890"
          sha="b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c"/>
 <treeref name="subdir_two" mode="040755" mtime="123457891"
          sha="7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730"/>
 <fileref name="file1" mode="100644" mtime="0987654321" size="1020304050">
  <part offset="0" size="10000" 
        sha="30520052695c48c1bae453774d29c2c1f9fdbf10087b16d96a367112c2c482ef"/>
  <part offset="10000" size="10000" 
        sha="abb4d6b4a7568591ce172590bb6d96f8968d86425761fbc148685a9ad0c4c644"/>
 </fileref>
</tree>
EOF

echo "e1593176d4c1b57ccfe29fcab28cf9da8c162e34b0f2b1d733e4383d3b6fd579" > ref_sha

test_expect_success 'creating more advanced tree' '
  (
    lyekka mktree tree2.xml > sha &&
    test_cmp ref_sha sha
  )'

cat >expected <<EOF
ref:30520052695c48c1bae453774d29c2c1f9fdbf10087b16d96a367112c2c482ef
ref:7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730
ref:abb4d6b4a7568591ce172590bb6d96f8968d86425761fbc148685a9ad0c4c644
ref:b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c
tree:b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c 16877 123457890 subdir_one
tree:7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730 16877 123457891 subdir_two
file:33188 1020304050 987654321 file1
part:30520052695c48c1bae453774d29c2c1f9fdbf10087b16d96a367112c2c482ef 0 10000
part:abb4d6b4a7568591ce172590bb6d96f8968d86425761fbc148685a9ad0c4c644 10000 10000
EOF

test_expect_success 'listing more advanced tree' '
  (
    lyekka lstree --show raw `cat sha` > actual &&
    test_cmp expected actual 
  )'

test_done