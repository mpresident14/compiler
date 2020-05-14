#!/bin/bash

make main && bin/main && gcc -g -o prez prez.s src/assembly/runtime.c && cat prez.s
