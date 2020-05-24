#!/bin/bash

make -j8 main && cd .. && bin/main && cat src/main/test.s
# && gcc -g -o prez prez.s src/x86gen/runtime.c
