// Acl - the Accel Language.
//
// A tiny float expression language that the front-end (aclc.py) compiles into
// the `accel` MLIR dialect. Example:
//
//     def quad(x)    = 2*x*x + 3*x + 5;
//     def dot(a, b)  = a*b;
//
// Each `def` becomes a `func.func` over f32; the body lowers to arith ops,
// which --fuse-mac then raises into accel.mac.
grammar Acl;

program  : function+ EOF ;

function : 'def' ID '(' params? ')' '=' expr ';' ;

params   : ID (',' ID)* ;

// Precedence climbing: '*' '/' bind tighter than '+' '-', both left-assoc.
expr
    : '(' expr ')'              # ParenExpr
    | expr op=('*' | '/') expr  # MulDivExpr
    | expr op=('+' | '-') expr  # AddSubExpr
    | NUMBER                    # NumberExpr
    | ID                        # VarExpr
    ;

ID      : [a-zA-Z_] [a-zA-Z0-9_]* ;
NUMBER  : [0-9]+ ('.' [0-9]+)? ;
WS      : [ \t\r\n]+ -> skip ;
COMMENT : '#' ~[\r\n]* -> skip ;
