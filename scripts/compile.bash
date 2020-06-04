#!/bin/bash

FILE=$1
BASE="/home/mpresident/cs/compiler"

cd "$BASE"/build && make -j8 main \
&& "$BASE"/bin/main "$BASE"/src/main/"$FILE".prez "$BASE"/log/"$FILE".s \
&& as "$BASE"/log/"$FILE".s "$BASE"/src/main/syscall.s -o "$BASE"/bin/"$FILE".o \
&& ld "$BASE"/bin/"$FILE".o -o "$BASE"/bin/"$FILE" \
&& "$BASE"/bin/"$FILE"
