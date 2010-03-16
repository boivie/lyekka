#!/bin/sh

test_description='Test blob creation'

. ./test-lib.sh

cat > expected <<EOF
1b21a586e59df809125fcf09eb3a2919a219fde2405a905b6763116855ab9339
EOF

test_expect_success 'empty blob' '
  (
    touch input_file &&
    lyekka create-blob -i input_file -o blob.blob > actual &&
    test -f blob.blob &&
    test_cmp expected actual &&
    lyekka cat-blob -i blob.blob > out.blob &&
    test_cmp input_file out.blob
  )'


cat > expected <<EOF
90b3d3012a93266891aebd3d8bd63145097a85502581491f0b2f011455c1b3bf
EOF

test_expect_success 'small blob' '
  (
    echo "foo bar" > input_file &&
    lyekka create-blob -i input_file -o blob.blob > actual &&
    test -f blob.blob &&
    test_cmp expected actual &&
    lyekka cat-blob -i blob.blob > out.blob &&
    test_cmp input_file out.blob
  )'


cat > expected <<EOF
f637f8839a6b3af1352f3ca7a1b8373984ee7ba63a9eece6843e4172505d12ad
EOF

test_expect_success 'large blob' '
  (
    dd if=/dev/zero of=input_file bs=31457280 count=1 &&
    lyekka create-blob -i input_file -o blob.blob > actual &&
    test -f blob.blob &&
    test_cmp expected actual &&
    lyekka cat-blob -i blob.blob > out.blob &&
    test_cmp input_file out.blob
  )'

test_done