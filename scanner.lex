%{
  #include "Types.h"
  #include "hw3_output.hpp"
  #include "parser.tab.hpp"
%}

%option yylineno
%option noyywrap

whitespace                                    [ \t\n\r]

%%

void                                          return VOID;
int                                           return INT;
byte                                          return BYTE;
b                                             return B;
bool                                          return BOOL;
const										  return CONST;
and                                           return AND;
or                                            return OR;
not                                           return NOT;
true                                          return TRUE;
false                                         return FALSE;
return                                        return RETURN;
if                                            return IF;
else                                          return ELSE;
while                                         return WHILE;
break                                         return BREAK;
continue                                      return CONTINUE;
;                                             return SC;
,                                             return COMMA;
\(                                            return LPAREN;
\)                                            return RPAREN;
\{                                            return LBRACE;
\}                                            return RBRACE;
=                                             return ASSIGN;
\<|\>|\<\=|\>\=                               return RELOP_COMPARE;
==|!=                                         return RELOP_EQUAL;
[\*\/]                                    	  yylval = new BinopMulCls(yytext); return BINOP_MUL;
[\+\-]                                        yylval = new BinopAddCls(yytext); return BINOP_ADD;
[a-zA-Z][a-zA-Z0-9]*                          yylval = new IDCls(yytext, yylineno); return ID;
0|[1-9][0-9]*                                 yylval = new NumCls(yytext); return NUM;
\"([^\n\r\"\\]|\\[rnt"\\])+\"			            return STRING;

\/\/[^\r\n]*[ \r|\n|\r\n]?                    ;
{whitespace}                                  ;
.                                             output::errorLex(yylineno); exit(1);

%%
