#!/bin/sh

test_description='Pack and unpack encrypted'

. ./test-lib.sh

pack_unpack_archive()
{
    lyekka gen-objects -e path1 -a archive.ly > files &&
    KEY=`tail -1 files | cut -d " " -f 3`
    mkdir -p path2 &&
    lyekka unpack -K $KEY -a archive.ly -o path2 &&
    diff -rq path1 path2 &&
    rm -rf path1 path2 archive.ly
}

test_expect_success 'files' '
  (
    mkdir -p path1 &&
    echo "hello" > path1/hello &&
    echo "world" > path1/world &&
    pack_unpack_archive
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
    pack_unpack_archive
  )'

test_done
