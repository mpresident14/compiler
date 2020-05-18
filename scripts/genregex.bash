#!/bin/bash

PARSER_DIR="../src/parser"

cp "$PARSER_DIR"/regex_parser.hpp "$PARSER_DIR"/obsolete/regex_parser_backup.hpp
cp "$PARSER_DIR"/regex_parser.cpp "$PARSER_DIR"/obsolete/regex_parser_backup.cpp
make parsegen
../bin/parsegen -g "$PARSER_DIR"/regex.pgen -f "$PARSER_DIR"/regex_parser
clang-format -i "$PARSER_DIR"/regex_parser.hpp "$PARSER_DIR"/regex_parser.cpp
