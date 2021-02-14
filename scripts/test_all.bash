#!/bin/bash
cd $HOME/cs/compiler
TESTS=""
for file in $(find src -type d -regex ".*tests$")
do
  TESTS+="//${file}:all "
done
bazel test $TESTS
