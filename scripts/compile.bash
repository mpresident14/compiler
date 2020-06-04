#!/bin/bash

FILE=$1

make -j8 main && cd .. && bin/main src/main/"$FILE" log/"$FILE".s
&& gcc -g -o bin/"$FILE" src/main/"$FILE" src/assembly/runtime.c
