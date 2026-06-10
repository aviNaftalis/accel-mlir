# Generated from /home/avin/accel-mlir/frontend/Acl.g4 by ANTLR 4.13.2
from antlr4 import *
if "." in __name__:
    from .AclParser import AclParser
else:
    from AclParser import AclParser

# This class defines a complete generic visitor for a parse tree produced by AclParser.

class AclVisitor(ParseTreeVisitor):

    # Visit a parse tree produced by AclParser#program.
    def visitProgram(self, ctx:AclParser.ProgramContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by AclParser#function.
    def visitFunction(self, ctx:AclParser.FunctionContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by AclParser#params.
    def visitParams(self, ctx:AclParser.ParamsContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by AclParser#MulDivExpr.
    def visitMulDivExpr(self, ctx:AclParser.MulDivExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by AclParser#NumberExpr.
    def visitNumberExpr(self, ctx:AclParser.NumberExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by AclParser#VarExpr.
    def visitVarExpr(self, ctx:AclParser.VarExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by AclParser#ParenExpr.
    def visitParenExpr(self, ctx:AclParser.ParenExprContext):
        return self.visitChildren(ctx)


    # Visit a parse tree produced by AclParser#AddSubExpr.
    def visitAddSubExpr(self, ctx:AclParser.AddSubExprContext):
        return self.visitChildren(ctx)



del AclParser