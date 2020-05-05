#header
%
  #include "test/utils/expr.hpp"
  #include <iostream>
%

// This is a comment. It should be ignored!

#source
%
  char c[] = "Just for testing \%";
%


#tokens
INT  "[1-9][0-9]*" -> int % stoi(string(str)) %
                          % cerr << "INT deleter called" << endl; %
PLUS "+"
MINUS "-"
STAR "\*"
FSLASH "/"
TILDE "~"
LPAREN "\("
RPAREN "\)"
#skip "???"

#prec
PLUS MINUS #left
STAR FSLASH #left
TILDE NEG #nonassoc

#grammar
Expr -> Expr* % delete #0; %
  := INT                       % new EInt(#0) %
   | Expr PLUS Expr            % new EBinOp(#0, PLUS, #2) %
   | Expr STAR Expr            % new EBinOp(#0, TIMES, #2) %
   | Expr MINUS Expr           % new EBinOp(#0, MINUS, #2) %
   | Expr FSLASH Expr          % new EBinOp(#0, DIVIDE, #2) %
   | TILDE Expr                % new EUOp(#1, BITNEG) %
   | MINUS Expr #prec TILDE    % new EUOp(#1, NEG) %
   | ParenExpr                 % #0 %

ParenExpr -> Expr*
  := LPAREN Expr RPAREN        % #1 %
