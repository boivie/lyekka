#!/bin/sh

test_description='ZLIB compression'

. ./test-lib.sh

compress_decompress()
{
    lyekka zlib < input > output &&
    lyekka zlib -d < output > input2 &&
    test_cmp input input2
}

test_expect_success 'Empty string' '
  (
    rm -f input &&
    touch input &&
    compress_decompress
  )'

test_expect_success 'Short string' '
  (
    echo "foo bar" > input &&
    compress_decompress
  )'

test_expect_success 'Longer string' '
  (
    echo "The quick brown fox jumps over the lazy dog\c" > input && 
    compress_decompress
  )'

test_expect_success 'Odd and large string' '
  (
    dd if=/dev/urandom of=input bs=4099 count=1 &&
    compress_decompress
  )'

test_expect_success 'Very long string' '
  (
    dd if=/dev/zero of=input bs=100000 count=10 &&
    compress_decompress
  )'

test_expect_success 'Extremely long string' '
  (
    dd if=/dev/zero of=input bs=3145728 count=10 &&
    compress_decompress
  )'

test_done
