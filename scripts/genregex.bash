#!/bin/bash

cp src/parser/regex_parser.hpp src/parser/obsolete/regex_parser_backup.hpp
cp src/parser/regex_parser.cpp src/parser/obsolete/regex_parser_backup.cpp
make parsegen
parsegen -g src/parser/regex.pgen -f src/parser/regex_parser
clang-format -i src/parser/regex_parser.hpp src/parser/regex_parser.cpp
