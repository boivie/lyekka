#!/bin/sh

test_description='Test indexing'

. ./test-lib.sh

test_expect_success 'simple tree' '
  (
    mkdir -p path1 &&
    echo "hello" > path1/hello &&
    echo "world" > path1/world &&
    lyekka create &&
    lyekka path add path1 &&
    lyekka update-index
  )'

test_done