#!/bin/bash

make -j8 main && cd .. && bin/main \
&& gcc -g -o prez prez.s src/x86gen/runtime.c && cat prez.s
