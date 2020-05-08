cp src/parser/regex.hpp src/parser/regex_backup.hpp
cp src/parser/regex.cpp src/parser/regex_backup.cpp
make parsegen
parsegen -g src/parser/regex.pgen -f src/parser/regex_parser
clang-format -i src/parser/regex_parser.hpp src/parser/regex_parser.cpp
