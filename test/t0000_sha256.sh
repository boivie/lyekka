#!/bin/sh

test_description='Basic tests'

. ./test-lib.sh

cat >expected <<EOF
e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
EOF

test_expect_success 'SHA256 test vector: empty string' '
  (
    rm -f foo &&
    touch foo &&
    lyekka sha256 foo > actual && 
    test_cmp expected actual 
  )'

cat >expected <<EOF
d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592
EOF

test_expect_success 'SHA256 test vector: longer string' '
  (
    echo "The quick brown fox jumps over the lazy dog\c" > text &&
    lyekka sha256 text > actual && 
    test_cmp expected actual 
  )'

cat >expected <<EOF
d29751f2649b32ff572b5e0a9f541ea660a50f94ff0beedfb0b692b924cc8025
EOF

test_expect_success 'SHA256 test vector: really long data' '
  (
    dd if=/dev/zero of=input_file bs=100000 count=10 &&
    lyekka sha256 input_file > actual && 
    test_cmp expected actual 
  )'
test_done

