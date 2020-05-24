#!/bin/bash

make -j8 main && cd .. && bin/main && cat src/main/test.s \
&& gcc -g -o bin/test src/main/test.s src/assembly/runtime.c
