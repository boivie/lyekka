#!/bin/sh

test_description='Create'

. ./test-lib.sh

test_expect_success 'create' '
  (
    lyekka create &&
    test -f lyekka.db
  )'

test_done