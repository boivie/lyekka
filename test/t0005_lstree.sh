#!/bin/sh

test_description='Test indexing'

. ./test-lib.sh

cat >tree.xml <<EOF
<tree>
 <treeref name="subdir_one" mode="040755" mtime="123457890" sha="b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c" />
 <treeref name="subdir_two" mode="040755" mtime="123457891" sha="7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730" />
</tree>
EOF

cat >expected <<EOF
References:
       0: 7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730
       1: b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c

Contents:
drwxr-xr-x b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c subdir_one
drwxr-xr-x 7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730 subdir_two
EOF

test_expect_success 'listing simple tree' '
  (
    lyekka mktree < tree.xml > actual &&
    lyekka lstree -i b94df087dce858b5eec9e5cc31a10a793c79a95d79cd43b25dd36438e197b6f0 > actual &&
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

cat >expected <<EOF
References:
       0: 30520052695c48c1bae453774d29c2c1f9fdbf10087b16d96a367112c2c482ef
       1: 7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730
       2: abb4d6b4a7568591ce172590bb6d96f8968d86425761fbc148685a9ad0c4c644
       3: b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c

Contents:
drwxr-xr-x b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c subdir_one
drwxr-xr-x 7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730 subdir_two
-rw-r--r-- ---------------------------------------------------------------- file1
---------- 30520052695c48c1bae453774d29c2c1f9fdbf10087b16d96a367112c2c482ef [0--10000]
---------- abb4d6b4a7568591ce172590bb6d96f8968d86425761fbc148685a9ad0c4c644 [10000--20000]
EOF

test_expect_success 'listing more advanced tree' '
  (
    lyekka mktree < tree2.xml > actual &&
    lyekka lstree -i 8fb59ea99a3a384ad94ec04b13ef6c43b5d346726db8c5d49b9f8fe00b02bad3 > actual &&
    test_cmp expected actual 
  )'

test_done