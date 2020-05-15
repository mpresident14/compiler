#!/bin/bash

make -j8 main && cd .. && bin/main && cat prez.s
# && gcc -g -o prez prez.s src/x86gen/runtime.c
