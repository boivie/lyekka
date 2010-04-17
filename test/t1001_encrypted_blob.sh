#!/bin/sh

test_description='Test blob creation'

. ./test-lib.sh

encrypt_decrypt()
{
    lyekka create-blob -e input_file > id 
    lyekka create-blob -e input_file > id2
    SHA=`cut -d " " -f 1 id`
    KEY=`cut -d " " -f 2 id`
    test -f $SHA &&
    lyekka cat-blob -K $KEY $SHA > out.blob &&
    test_cmp input_file out.blob &&
    test_cmp id id2
}

test_expect_success 'empty blob' '
  (
    rm -f input_file out.blob &&
    touch input_file &&
    encrypt_decrypt
  )'

test_expect_success 'small blob' '
  (
    echo "foo bar" > input_file &&
    encrypt_decrypt
  )'

test_expect_success 'large blob' '
  (
    dd if=/dev/zero of=input_file bs=3145728 count=1 &&
    encrypt_decrypt
  )'

slow_test_expect_success 'very large blob' '
  (
    dd if=/dev/zero of=input_file bs=3145728 count=10 &&
    encrypt_decrypt
  )'

test_done