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

KEY=27ae41e4649b934ca495991b7852b855
IV=e3b0c44298fc1c149afbf4c8996fb924

# Shorter than one block (16 bytes)
test_expect_success 'AES128CBC: Encrypt short string' '
  (
    echo "foobar\c" > file.txt && 
    openssl enc -aes-128-cbc -in file.txt -out file.enc1 \
     -iv $IV -K $KEY &&
    lyekka enc -c aes-128-cbc -K $KEY --iv $IV --in file.txt --out file.enc2 &&
    lyekka sha256 file.enc1 > expected &&
    lyekka sha256 file.enc2 > actual &&
    test_cmp expected actual
  )'

test_expect_success 'AES128CBC: Decrypt short string' '
  (
    lyekka enc -d -c aes-128-cbc -K $KEY --iv $IV --in file.enc1 --out file.dec &&
    test_cmp file.txt file.dec &&
    rm file.txt file.enc? file.dec
  )'

# Longer than one block, but still rather short.
test_expect_success 'AES128CBC: Encrypt onger string' '
  (
    echo "The quick brown fox jumps over the lazy dog\c" > file.txt && 
    openssl enc -aes-128-cbc -in file.txt -out file.enc1 \
     -iv $IV -K $KEY &&
    lyekka enc -c aes-128-cbc -K $KEY --iv $IV --in file.txt --out file.enc2 &&
    lyekka sha256 file.enc1 > expected &&
    lyekka sha256 file.enc2 > actual &&
    test_cmp expected actual
  )'

test_expect_success 'AES128CBC: Decrypt longer string' '
  (
    lyekka enc -d -c aes-128-cbc -K $KEY --iv $IV --in file.enc1 --out file.dec &&
    test_cmp file.txt file.dec &&
    rm file.txt file.enc? file.dec
  )'

test_expect_success 'AES128CBC: Encrypt odd and large string' '
  (
    dd if=/dev/urandom of=file.txt bs=4099 count=1 &&
    openssl enc -aes-128-cbc -in file.txt -out file.enc1 \
     -iv $IV -K $KEY &&
    lyekka enc -c aes-128-cbc -K $KEY --iv $IV --in file.txt --out file.enc2 &&
    lyekka sha256 file.enc1 > expected &&
    lyekka sha256 file.enc2 > actual &&
    test_cmp expected actual
  )'

test_expect_success 'AES128CBC: Decrypt odd and large string' '
  (
    lyekka enc -d -c aes-128-cbc -K $KEY --iv $IV --in file.enc1 --out file.dec &&
    test_cmp file.txt file.dec &&
    rm file.txt file.enc? file.dec
  )'

# Very long, so that we need to get several output blocks.
test_expect_success 'AES128CBC: Encrypt very long string' '
  (
    dd if=/dev/zero of=file.txt bs=100000 count=10 &&
    openssl enc -aes-128-cbc -in file.txt -out file.enc1 \
     -iv $IV -K $KEY &&
    lyekka enc -c aes-128-cbc -K $KEY --iv $IV --in file.txt --out file.enc2 &&
    lyekka sha256 file.enc1 > expected &&
    lyekka sha256 file.enc2 > actual &&
    test_cmp expected actual
  )'

test_expect_success 'AES128CBC: Decrypt very long string' '
  (
    lyekka enc -d -c aes-128-cbc -K $KEY --iv $IV --in file.enc1 --out file.dec &&
    test_cmp file.txt file.dec &&
    rm file.txt file.enc? file.dec
  )'

test_done

