#!/bin/sh

test_description='Manifest'

. ./test-lib.sh

cat >simple_manifest.xml <<EOF
<manifest>
 <entry_point>5891b5b522d5df086d0ff0b110fbd9d21bb4fc7163af34d08286a2e846f6be03</entry_point>
</manifest>
EOF

cat >simple_expected <<EOF
Entry-Point: 5891b5b522d5df086d0ff0b110fbd9d21bb4fc7163af34d08286a2e846f6be03
Properties:
Secure Properties:
EOF

cat >advanced_manifest.xml <<EOF
<manifest>
 <entry_point>5891b5b522d5df086d0ff0b110fbd9d21bb4fc7163af34d08286a2e846f6be03</entry_point>
 <properties>
  <property name="name">emperor</property>
  <property name="something.something">dark side</property>
 </properties>
</manifest>
EOF

cat >advanced_expected <<EOF
Entry-Point: 5891b5b522d5df086d0ff0b110fbd9d21bb4fc7163af34d08286a2e846f6be03
Properties:
  name: emperor
  something.something: dark side
Secure Properties:
EOF

cat >secure_manifest.xml <<EOF
<manifest>
 <entry_point>5891b5b522d5df086d0ff0b110fbd9d21bb4fc7163af34d08286a2e846f6be03</entry_point>
 <properties>
  <property name="name">emperor</property>
  <property name="something.something">dark side</property>
 </properties>
 <secure_properties>
  <property name="goal">death star</property>
  <property name="weakness">thermal exhaust port</property>
 </secure_properties>
</manifest>
EOF

cat >secure_expected <<EOF
Entry-Point: 5891b5b522d5df086d0ff0b110fbd9d21bb4fc7163af34d08286a2e846f6be03
Properties:
  name: emperor
  something.something: dark side
Secure Properties:
  goal: death star
  weakness: thermal exhaust port
EOF

test_expect_success 'create simple' '
  (
    lyekka mk-manifest simple_manifest.xml > id &&
    SHA=`cat id | cut -d " " -f 1` &&
    test -f $SHA &&
    TYPE=`lyekka show-object -t $SHA`
    test "$TYPE" = "manifest"
  )'

test_expect_success 'list simple' '
  (
    SHA=`cat id | cut -d " " -f 1` &&
    lyekka show-manifest $SHA > actual &&
    test_cmp actual simple_expected
  )'

test_expect_success 'create advanced' '
  (
    lyekka mk-manifest advanced_manifest.xml > id &&
    SHA=`cat id | cut -d " " -f 1` &&
    test -f $SHA &&
    TYPE=`lyekka show-object -t $SHA`
    test "$TYPE" = "manifest"
  )'

test_expect_success 'list advanced' '
  (
    SHA=`cat id | cut -d " " -f 1` &&
    lyekka show-manifest $SHA > actual &&
    test_cmp actual advanced_expected
  )'

test_expect_success 'create secure' '
  (
    lyekka mk-manifest secure_manifest.xml > id &&
    SHA=`cat id | cut -d " " -f 1` &&
    test -f $SHA &&
    TYPE=`lyekka show-object -t $SHA`
    test "$TYPE" = "manifest"
  )'

test_expect_success 'list secure without key' '
  (
    SHA=`cat id | cut -d " " -f 1` &&
    lyekka show-manifest $SHA > actual &&
    test_cmp actual advanced_expected
  )'


test_expect_success 'list secure with key' '
  (
    SHA=`cat id | cut -d " " -f 1` &&
    KEY=`cat id | cut -d " " -f 2` &&
    lyekka show-manifest -K $KEY $SHA > actual &&
    test_cmp actual secure_expected
  )'


test_done