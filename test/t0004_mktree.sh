#!/bin/sh

test_description='Test indexing'

. ./test-lib.sh

cat >tree.xml <<EOF
<tree>
 <treeref name="subdir_one" mode="040755" mtime="123457890" sha="b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c" />
 <treeref name="subdir_two" mode="040755" mtime="123457891" sha="7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730" />
</tree>
EOF

echo "e5f7d3b6f329907830b479aefd5ef41815a8ab6d41a12139e0e54eee79389fad" > expected

test_expect_success 'generate simple tree' '
  (
    lyekka mktree < tree.xml > actual &&
    test_cmp expected actual 
  )'


cat >tree2.xml <<EOF
<tree>
 <treeref name="subdir_one" mode="040755" mtime="123457890" sha="b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c"/>
 <treeref name="subdir_two" mode="040755" mtime="123457891" sha="7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730"/>
 <fileref name="file1" mode="100644" mtime="0987654321" size="1020304050">
  <part offset="0" size="10000" sha="30520052695c48c1bae453774d29c2c1f9fdbf10087b16d96a367112c2c482ef"/>
  <part offset="10000" size="10000" sha="abb4d6b4a7568591ce172590bb6d96f8968d86425761fbc148685a9ad0c4c644"/>
 </fileref>
</tree>
EOF

echo "e1593176d4c1b57ccfe29fcab28cf9da8c162e34b0f2b1d733e4383d3b6fd579" > expected

test_expect_success 'generate more advanced tree' '
  (
    lyekka mktree < tree2.xml > actual &&
    test_cmp expected actual 
  )'

test_done