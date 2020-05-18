#!/bin/bash

PARSER_DIR="../src/parser"

cp "$PARSER_DIR"/obsolete/regex_parser_backup.hpp "$PARSER_DIR"/regex_parser.hpp
cp "$PARSER_DIR"/obsolete/regex_parser_backup.cpp "$PARSER_DIR"/regex_parser.cpp
