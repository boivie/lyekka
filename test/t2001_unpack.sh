#!/bin/sh

test_description='Pack and unpack'

. ./test-lib.sh

test_expect_success 'files' '
  (
    mkdir -p path1 &&
    echo "hello" > path1/hello &&
    echo "world" > path1/world &&
    lyekka gen-objects path1 > actual &&
    sha=`tail -1 actual | cut -c 3-` &&
    mkdir path2 &&
    lyekka unpack -i $sha -o path2 &&
    diff -rq path1 path2 &&
    rm -rf path2 [0-9]*
  )'

test_expect_success 'files in archive' ' 
  (
    lyekka gen-objects path1 -a archive.ly &&
    mkdir -p path2 &&
    lyekka unpack -a archive.ly -o path2 &&
    diff -rq path1 path2 &&
    rm -rf path1 path2 archive.ly
  )'

test_expect_success 'files and directories' '
  (
    mkdir -p path1/path2/path3 &&
    mkdir -p path1/path4 &&
    mkdir -p path1/path5/path6 &&
    echo "hello" > path1/path4/hello &&
    echo "world" > path1/world &&
    echo "fum" >   path1/path5/path6/fum &&
    echo "gazonk" > path1/gazonk &&
    lyekka gen-objects path1 > actual &&
    sha=`tail -1 actual | cut -c 3-` &&
    mkdir path2 &&
    lyekka unpack -i $sha -o path2 &&
    diff -rq path1 path2 && 
    rm -rf path2 
  )'


test_expect_success 'files and directories in archive' '
  (
    lyekka gen-objects path1 -a archive.ly &&
    mkdir path2 &&
    lyekka unpack -a archive.ly -o path2 &&
    diff -rq path1 path2 && 
    rm -rf path1 path2 
  )'



test_done