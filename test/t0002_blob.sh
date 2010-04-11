#!/bin/sh

test_description='Test blob creation'

. ./test-lib.sh

SHA=1b21a586e59df809125fcf09eb3a2919a219fde2405a905b6763116855ab9339
cat > expected <<EOF
$SHA
EOF

test_expect_success 'empty blob' '
  (
    rm -f input_file &&
    touch input_file &&
    lyekka create-blob input_file > actual &&
    test -f $SHA &&
    test_cmp expected actual &&
    lyekka cat-blob $SHA > out.blob &&
    test_cmp input_file out.blob
  )'


SHA=90b3d3012a93266891aebd3d8bd63145097a85502581491f0b2f011455c1b3bf
cat > expected <<EOF
$SHA
EOF

test_expect_success 'small blob' '
  (
    echo "foo bar" > input_file &&
    lyekka create-blob input_file > actual &&
    test -f $SHA &&
    test_cmp expected actual &&
    lyekka cat-blob $SHA > out.blob &&
    test_cmp input_file out.blob
  )'


SHA=f637f8839a6b3af1352f3ca7a1b8373984ee7ba63a9eece6843e4172505d12ad
cat > expected <<EOF
$SHA
EOF

test_expect_success 'large blob' '
  (
    dd if=/dev/zero of=input_file bs=3145728 count=10 &&
    lyekka create-blob input_file > actual &&
    test -f $SHA &&
    test_cmp expected actual &&
    lyekka cat-blob $SHA > out.blob &&
    test_cmp input_file out.blob
  )'

test_done