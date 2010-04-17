#!/bin/sh

test_description='AES-128-CBC'

. ./test-lib.sh

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


test_expect_success 'AES128CBC: Encrypt extremely long string' '
  (
    dd if=/dev/zero of=file.txt bs=3145728 count=10 &&
    openssl enc -aes-128-cbc -in file.txt -out file.enc1 \
     -iv $IV -K $KEY &&
    lyekka enc -c aes-128-cbc -K $KEY --iv $IV --in file.txt --out file.enc2 &&
    lyekka sha256 file.enc1 > expected &&
    lyekka sha256 file.enc2 > actual &&
    test_cmp expected actual
  )'

test_expect_success 'AES128CBC: Decrypt extremely long string' '
  (
    lyekka enc -d -c aes-128-cbc -K $KEY --iv $IV --in file.enc1 --out file.dec &&
    test_cmp file.txt file.dec &&
    rm file.txt file.enc? file.dec
  )'


test_done
