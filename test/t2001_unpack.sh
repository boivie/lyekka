#!/bin/sh

test_description='Pack and unpack'

. ./test-lib.sh

pack_unpack_files()
{
    lyekka gen-objects path1 > actual &&
    sha=`tail -1 actual | cut -c 3-` &&
    mkdir path2 &&
    lyekka unpack -i $sha -o path2 &&
    diff -rq path1 path2 &&
    rm -rf path2 [0-9]*
}

pack_unpack_archive()
{
    lyekka gen-objects path1 -a archive.ly &&
    mkdir -p path2 &&
    lyekka unpack -a archive.ly -o path2 &&
    diff -rq path1 path2 &&
    rm -rf path1 path2 archive.ly
}

test_expect_success 'files' '
  (
    mkdir -p path1 &&
    echo "hello" > path1/hello &&
    echo "world" > path1/world &&
    pack_unpack_files
  )'

test_expect_success 'files in archive' ' 
  (
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
    pack_unpack_files
  )'


test_expect_success 'files and directories in archive' '
  (
    pack_unpack_archive
  )'


mkdir path1
for (( i = 1; i <= 999; i++ ))
do
    echo "file: $i" > path1/file_$i
done

test_expect_success 'a lot of files' '
  ( 
    pack_unpack_archive
  )'

test_done
