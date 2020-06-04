#!/bin/bash

FILE=$1

make -j8 main && cd .. && bin/main src/main/"$FILE".prez log/"$FILE".s \
&& gcc -g -o bin/"$FILE" log/"$FILE".s src/main/syscall.s src/main/runtime.c \
&& bin/"$FILE"
