#pragma once
#include<iostream>
#include<string>
#include<memory>
#include<vector>
class BaseAST;
class CompUnitAST;
class FuncDefAST;
class FuncTypeAST;
class BlockAST;
class BlockItemAST;
class StmtAST;

class ExpAST;
class LOrExpAST;
class LAndExpAST;
class EqExpAST;
class RelExpAST;
class AddExpAST;
class MulExpAST;
class UnaryExpAST;
class UnaryOpAST;
class PrimaryExpAST;
class NumberAST;

class DeclAST;
class ConstDeclAST;
class VarDeclAST;
class ConstDefAST;
class InitValAST;
class BTypeAST;
class ConstInitValAST;
class ConstDefAST;
class ConstExpAST;
class LValAST;
 
class BaseAST{
    public:
        virtual ~BaseAST() = default;
        virtual void Dump() const = 0;
};
class CompUnitAST:public BaseAST{
    public:
        std::vector<std::unique_ptr<FuncDefAST>> func_def_vec;
        void Dump() const override {
            //std::cout << "CompUnitAST" << std::endl;
        }
};
class FuncDefAST:public BaseAST{
    public:
        std::unique_ptr<FuncTypeAST> func_type;
        std::string ident;
        std::vector<std::unique_ptr<BlockAST>> block_vec;
        void Dump() const override {
            //std::cout << "FuncDefAST" << std::endl;
        }
};
class FuncTypeAST:public BaseAST{
    public:
        enum FuncType{
            FUNC_TYPE_INT,
            FUNC_TYPE_VOID,
            FUNC_TYPE_FLOAT
        } func_type;
        void Dump() const override {
            //std::cout << "FuncTypeAST" << std::endl;
        }
};
class BlockAST:public BaseAST{
    public:
        std::vector<std::unique_ptr<BlockItemAST>> block_item_vec;
        void Dump() const override {
            //std::cout << "BlockAST" << std::endl;
        }
};
class BlockItemAST:public BaseAST{
    public:
        enum BlockItemType{
            BLOCK_ITEM_STMT,
            BLOCK_ITEM_DECL
        } block_item_type;
        std::unique_ptr<StmtAST> stmt;
        std::unique_ptr<DeclAST> decl;
        BlockItemAST(BlockItemType type):block_item_type(type){}
        void Dump() const override {
            //std::cout << "BlockItemAST" << std::endl;
        }
};
class StmtAST:public BaseAST{
    public:
        enum StmtType{
            STMT_EXP,
            STMT_BLOCK,
            STMT_IF,
            STMT_ELSE,
            STMT_LVal_Assign_EXP,
            STMT_WHILE,
            STMT_RETURN,
            STMT_EMPTY
        } stmt_type;
        std::unique_ptr<ExpAST> exp;
        std::unique_ptr<BlockAST> block;
        std::unique_ptr<LValAST> l_val;
        StmtAST(StmtType type):stmt_type(type){}
        void Dump() const override {
            //std::cout << "StmtAST" << std::endl;
        }
};
class ExpAST:public BaseAST{
    public:
       std::unique_ptr<LOrExpAST> l_or_exp;
       void Dump() const override {
            //std::cout << "ExpAST" << std::endl;
        }
};
class LOrExpAST:public BaseAST{
    public:
        enum LOrExpType{
            LAND_EXP,
            LOR_EXP_Op_LAND_EXP
        } l_or_exp_type;
        enum Op{
            OR_OR
        }op;
        std::unique_ptr<LAndExpAST> l_and_exp;
        std::unique_ptr<LOrExpAST> l_or_exp;
        LOrExpAST(LOrExpType type):l_or_exp_type(type){}
        LOrExpAST(LOrExpType type, Op op):l_or_exp_type(type),op(op){}
        void Dump() const override {
            //std::cout << "LOrExpAST" << std::endl;
        }
};
class LAndExpAST:public BaseAST{
    public:
        enum LAndExpType{
            EQ_EXP,
            LAND_EXP_Op_EQ_EXP
        } l_and_exp_type;
        enum Op{
            AND_AND
        }op;
        std::unique_ptr<EqExpAST> eq_exp;
        std::unique_ptr<LAndExpAST> l_and_exp;
        LAndExpAST(LAndExpType type):l_and_exp_type(type){}
        LAndExpAST(LAndExpType type, Op op):l_and_exp_type(type),op(op){}
        void Dump() const override {
            //std::cout << "LAndExpAST" << std::endl;
        }
};
class EqExpAST:public BaseAST{
    public:
        enum EqExpType{
            Rel_EXP,
            EQ_EXP_Op_Rel_EXP
        } eq_exp_type;
        enum Op{
            EQ,
            NE
        }op;
        std::unique_ptr<RelExpAST> rel_exp;
        std::unique_ptr<EqExpAST> eq_exp;
        EqExpAST(EqExpType type):eq_exp_type(type){}
        EqExpAST(EqExpType type, Op op):eq_exp_type(type),op(op){}
        void Dump() const override {
            //std::cout << "EqExpAST" << std::endl;
        }
};
class RelExpAST:public BaseAST{
    public:
        enum RelExpType{
            Add_EXP,
            REL_EXP_Op_Add_EXP
        } rel_exp_type;
        enum Op{
            LT,
            GT,
            LE,
            GE
        }op;
        std::unique_ptr<AddExpAST> add_exp;
        std::unique_ptr<RelExpAST> rel_exp;
        RelExpAST(RelExpType type):rel_exp_type(type){}
        RelExpAST(RelExpType type, Op op):rel_exp_type(type),op(op){}
        void Dump() const override {
            //std::cout << "RelExpAST" << std::endl;
        }
};
class AddExpAST:public BaseAST{
    public:
        enum AddExpType{
            Mul_EXP,
            ADD_EXP_Op_Mul_EXP
        } add_exp_type;
        enum Op{
            PLUS,
            MINUS
        }op;
        std::unique_ptr<MulExpAST> mul_exp;
        std::unique_ptr<AddExpAST> add_exp;
        AddExpAST(AddExpType type):add_exp_type(type){}
        AddExpAST(AddExpType type, Op op):add_exp_type(type),op(op){}
        void Dump() const override {
            //std::cout << "AddExpAST" << std::endl;
        }
};
class MulExpAST:public BaseAST{
    public:
        enum MulExpType{
            Unary_EXP,
            MUL_EXP_Op_Unary_EXP
        } mul_exp_type;
        enum Op{
            MULT,
            DIV,
            MOD
        }op;
        std::unique_ptr<UnaryExpAST> unary_exp;
        std::unique_ptr<MulExpAST> mul_exp;
        MulExpAST(MulExpType type):mul_exp_type(type){}
        MulExpAST(MulExpType type, Op op):mul_exp_type(type),op(op){}
        void Dump() const override {
            //std::cout << "MulExpAST" << std::endl;
        }
};
class UnaryExpAST:public BaseAST{
    public:
        enum UnaryExpType{
            Primary_EXP,
            UnaryOp_EXP_Unary_EXP,
        } unary_exp_type;
        enum Op{
            PLUS,
            MINUS,
            NOT
        }op;
        std::unique_ptr<PrimaryExpAST> primary_exp;
        std::unique_ptr<UnaryExpAST> unary_exp;
        std::unique_ptr<UnaryOpAST> unary_op;
        UnaryExpAST(UnaryExpType type):unary_exp_type(type){}
        UnaryExpAST(UnaryExpType type, Op op):unary_exp_type(type),op(op){}
        void Dump() const override {
            //std::cout << "UnaryExpAST" << std::endl;
        }
};
class UnaryOpAST:public BaseAST{
    public:
        enum UnaryOpType{
            PLUS,
            MINUS,
            NOT
        } unary_op_type;
        UnaryOpAST(UnaryOpType type):unary_op_type(type){}
        void Dump() const override {
            //std::cout << "UnaryOpAST" << std::endl;
        }
};
class PrimaryExpAST:public BaseAST{
    public:
        enum PrimaryExpType{
            LVal,
            Number,
            Exp
        } primary_exp_type;
        std::unique_ptr<LValAST> l_val;
        std::unique_ptr<NumberAST> number;
        std::unique_ptr<ExpAST> exp;
        PrimaryExpAST(PrimaryExpType type):primary_exp_type(type){}
        void Dump() const override {
            //std::cout << "PrimaryExpAST" << std::endl;
        }
};
class NumberAST:public BaseAST{
    public:
        int number;
        void Dump() const override {
            //std::cout << "NumberAST" << std::endl;
        }
        NumberAST(){}
        NumberAST(int num):number(num){}
};
class DeclAST:public BaseAST{
    public:
        enum DeclType{
            ConstDecl,
            VarDecl
        } decl_type;
        std::unique_ptr<ConstDeclAST> const_decl;
        std::unique_ptr<VarDeclAST> var_decl;
        DeclAST(DeclType type):decl_type(type){}
        void Dump() const override {
            //std::cout << "DeclAST" << std::endl;
        }
};
class ConstDeclAST:public BaseAST{
    public:
        std::unique_ptr<BTypeAST> b_type;
        std::vector<std::unique_ptr<ConstDefAST>> const_def_vec;
        void Dump() const override {
            //std::cout << "ConstDeclAST" << std::endl;
        }
};
class VarDeclAST:public BaseAST{
    public:
        std::unique_ptr<BTypeAST> b_type;
        std::vector<std::unique_ptr<ConstDefAST>> const_def_vec;
        void Dump() const override {
            //std::cout << "VarDeclAST" << std::endl;
        }
};
class VarDefAST:public BaseAST{
    public:
    enum VarDefType{
        IDENT,
        IDENT_ASSIGN_INITVAL
    } var_def_type;
        std::string ident;
        std::unique_ptr<InitValAST> init_val;
        void Dump() const override {
            //std::cout << "VarDefAST" << std::endl;
        }
};  
class InitValAST:public BaseAST{
    public:
        std::unique_ptr<ExpAST> exp;
        void Dump() const override {
            //std::cout << "InitValAST" << std::endl;
        }
};
class BTypeAST:public BaseAST{
    public:
        enum BType{
            B_TYPE_INT,
            B_TYPE_FLOAT
        } b_type;
        void Dump() const override {
            //std::cout << "BTypeAST" << std::endl;
        }
};
class ConstDefAST:public BaseAST{
    public:
        std::string ident;
        std::unique_ptr<ConstInitValAST> const_init_val;
        void Dump() const override {
            //std::cout << "ConstDefAST" << std::endl;
        }
};
class ConstInitValAST:public BaseAST{
    public:
        std::unique_ptr<ConstExpAST> const_exp;
        void Dump() const override {
            //std::cout << "ConstInitValAST" << std::endl;
        }
};
class ConstExpAST:public BaseAST{
    public:
        std::unique_ptr<ExpAST> exp;
        void Dump() const override {
            //std::cout << "ConstExpAST" << std::endl;
        }
};
class LValAST:public BaseAST{
    public:
        std::string ident;
        void Dump() const override {
            //std::cout << "LValAST" << std::endl;
        }
};