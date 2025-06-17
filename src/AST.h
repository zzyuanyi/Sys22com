#pragma once
#include <string>
#include <memory>
#include <vector>
// 所有类的声明
class BaseAST; 
class CompilationUnitAST;
class FunctionDefinitionAST;
class FunctionParametersAST;
class FunctionParameterAST;

class CodeBlockAST;
class BlockElementAST;

class DeclarationAST;
class StatementAST;// Stmt


// Decl
class ConstantDeclarationAST;
class VariableDeclarationAST;
class BasicTypeAST;
class ConstantDefinitionAST;
class VariableDefinitionAST;
class InitialValueAST;
class ConstantInitialValueAST;
class LeftValueAST;
class ConstantExpressionAST;
class ArrayIndexConstantList;
class ArrayIndexExpressionList;

// Expression
class ExpressionAST;
class PrimaryExpressionAST;
class UnaryExpressionAST;
class MultiplicationExpressionAST;
class AdditionExpressionAST;
class RelationalExpressionAST;
class EqualityExpressionAST;
class LogicalAndExpressionAST;
class LogicalOrExpressionAST;

class FunctionCallParametersAST;

// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;
    // 完成LR的打印，并返回计算结果（临时变量或立即数）所在的变量
    // 删掉了，不用多态。
    // virtual std::string Dump() const = 0;
};

// CompUnit 是 BaseAST
class CompilationUnitAST : public BaseAST {
public:
    std::vector<std::unique_ptr<FunctionDefinitionAST>> function_definitions;
    std::vector<std::unique_ptr<DeclarationAST>> declarations;
    void Dump()const;
    void DumpGlobalVariables() const;
};

// FuncDef 也是 BaseAST
class FunctionDefinitionAST : public BaseAST {
public:
    std::unique_ptr<BasicTypeAST> return_type;    // 返回值类型
    std::string function_name;                  // 函数名标识符
    std::unique_ptr<FunctionParametersAST> parameters;    // 函数参数, nullptr则无参数
    std::unique_ptr<CodeBlockAST> function_body;    // 函数体
    void Dump() const;
};

class FunctionParametersAST : public BaseAST {
public:
    std::vector<std::unique_ptr<FunctionParameterAST>> parameter_list;
    void Dump() const;
};


class FunctionParameterAST : public BaseAST {
public:
    enum ParameterType { SINGLE_VARIABLE, ARRAY };
    ParameterType param_type;
    std::unique_ptr<BasicTypeAST> param_type_info;
    std::string parameter_name;
    std::vector<std::unique_ptr<ConstantExpressionAST>> array_dimensions;   
    std::string Dump() const; // 返回参数类型，如i32, *[i32, 4]
    void getArrayDimensions(std::vector<int> &dimensions);
};


// Block 也是 BaseAST
class CodeBlockAST : public BaseAST {
public:
    std::vector<std::unique_ptr<BlockElementAST>> block_items;
    void Dump(bool new_symbol_tb = true) const;
};

class BlockElementAST : public BaseAST {
public:
    enum TAG {DECL, STMT};
    TAG tag;
    std::unique_ptr<DeclarationAST> decl;
    std::unique_ptr<StatementAST> stmt;
    void Dump() const;
};

class DeclarationAST : public BaseAST {
public:
    enum TAG {CONST_DECL, VAR_DECL};
    TAG tag;
    std::unique_ptr<ConstantDeclarationAST> const_decl;
    std::unique_ptr<VariableDeclarationAST> var_decl;
    void Dump() const;
};

// StmtAST OtherStmt && IF 
class StatementAST : public BaseAST {
public:
    /**
     * OtherStmt ::= LVal "=" Exp ";"
       | [Exp] ";"
       | Block
       | "return" [Exp] ";";
       | "while" "(" Exp ")" Stmt
       | "break" ";"
       | "continue" ";"
     * 以及IF语句
    */
    enum TAG {RETURN, ASSIGN, BLOCK, EXP, WHILE, BREAK, CONTINUE, IF};
    TAG tag;
    std::unique_ptr<ExpressionAST> exp;
    std::unique_ptr<LeftValueAST> lval;
    std::unique_ptr<CodeBlockAST> block;
    std::unique_ptr<StatementAST> stmt;
    std::unique_ptr<StatementAST> if_stmt;
    std::unique_ptr<StatementAST> else_stmt;
    void Dump() const;
};

class ConstantDeclarationAST : public BaseAST {
public:
    std::vector<std::unique_ptr<ConstantDefinitionAST>> const_defs;
    std::unique_ptr<BasicTypeAST> btype;
    void Dump() const;
};

class VariableDeclarationAST : public BaseAST {
public:
    std::vector<std::unique_ptr<VariableDefinitionAST>> var_defs;
    std::unique_ptr<BasicTypeAST> btype;
    void Dump() const;
};

class BasicTypeAST : public BaseAST {
public:
    enum TAG {VOID, INT};
    TAG tag;
    void Dump() const;
};

class ConstantDefinitionAST : public BaseAST {
public:
    enum TAG { VARIABLE, ARRAY };
    TAG tag;
    std::string ident;
    std::vector<std::unique_ptr<ConstantExpressionAST>> const_exps;   // size !=0, Array
    std::unique_ptr<ConstantInitialValueAST> const_init_val;
    void Dump(bool is_global = false) const;
    void DumpArray(bool is_global = false) const;
};

class VariableDefinitionAST: public BaseAST {
public:
    enum TAG { VARIABLE, ARRAY };
    TAG tag;
    std::string ident;
    std::vector<std::unique_ptr<ConstantExpressionAST>> const_exps;   // size != 0, Array
    std::unique_ptr<InitialValueAST> init_val;   // nullptr implies no init_val
    void Dump(bool is_global = false) const;
    void DumpArray(bool is_global = false) const;
};

class InitialValueAST : public BaseAST{
public:
    enum TAG { EXP, INIT_LIST};
    TAG tag;
    std::unique_ptr<ExpressionAST> exp;
    std::vector<std::unique_ptr<InitialValueAST>> inits; // can be 0, 1, 2,....
    std::string Dump() const;
    void getInitVal(std::string *ptr, const std::vector<int> &len, bool is_global = false) const;
};

class ConstantInitialValueAST : public BaseAST {
public:
    enum TAG { CONST_EXP, CONST_INIT_LIST };
    TAG tag;
    std::unique_ptr<ConstantExpressionAST> const_exp;
    std::vector<std::unique_ptr<ConstantInitialValueAST>> inits;    // size can be 0, 1, ...
    // 表达式求值，计算结果放在pi所指的int内存地址
    int getValue();
    void getInitVal(std::string *ptr, const std::vector<int> &len) const;
};


class LeftValueAST : public BaseAST {
public:
    enum TAG { VARIABLE, ARRAY };
    TAG tag;
    std::string ident;
    std::vector<std::unique_ptr<ExpressionAST>> exps;      // exps.size() != 0 implies ARRAY
    std::string Dump(bool dump_ptr = false) const;   // 默认返回的是i32而非指针。
    int getValue();
};

class ConstantExpressionAST : public BaseAST {
public:
    std::unique_ptr<ExpressionAST> exp;
    int getValue();
};

class ArrayIndexConstantList : public BaseAST {
public:
    std::vector<std::unique_ptr<ConstantExpressionAST>> const_exps;
};

class ArrayIndexExpressionList : public BaseAST {
public:
    std::vector<std::unique_ptr<ExpressionAST>> exps;
};


class ConstantExpressionListAST: public BaseAST {
public:
    std::vector<std::unique_ptr<ConstantExpressionAST>> const_exps;
};

class ExpressionListAST: public BaseAST {
public:
    std::vector<std::unique_ptr<ExpressionAST>> exps;
    int getValue();
};


// Exp
class ExpressionAST : public BaseAST {
public:
    std::unique_ptr<LogicalOrExpressionAST> l_or_exp;
    std::string Dump() const;
    int getValue();
};

class PrimaryExpressionAST : public BaseAST {
public:
    enum TAG { PARENTHESES, NUMBER, LVAL};
    TAG tag;
    std::unique_ptr<ExpressionAST> exp;
    std::unique_ptr<LeftValueAST> lval;
    int number;
    std::string Dump() const ;
    int getValue();
};


class UnaryExpressionAST : public BaseAST {
public:
    enum TAG { PRIMARY_EXP, OP_UNITARY_EXP, FUNC_CALL};
    TAG tag;
    std::unique_ptr<PrimaryExpressionAST> primary_exp;
    char unary_op;
    std::unique_ptr<UnaryExpressionAST> unary_exp;
    std::string ident;
    std::unique_ptr<FunctionCallParametersAST> func_params;
    std::string Dump() const;
    int getValue();
};

// MulExp
class MultiplicationExpressionAST : public BaseAST {
public:
    enum TAG {UNARY_EXP, OP_MUL_EXP};
    TAG tag;
    std::unique_ptr<UnaryExpressionAST> unary_exp;
    std::unique_ptr<MultiplicationExpressionAST> mul_exp_1;
    std::unique_ptr<UnaryExpressionAST> unary_exp_2;
    char mul_op;
    std::string Dump() const;
    int getValue();
};

// AddExp
class AdditionExpressionAST : public BaseAST {
public:
    enum TAG {MUL_EXP, OP_ADD_EXP};
    TAG tag;
    std::unique_ptr<MultiplicationExpressionAST> mul_exp;
    std::unique_ptr<AdditionExpressionAST> add_exp_1;
    std::unique_ptr<MultiplicationExpressionAST> mul_exp_2;
    char add_op;
    std::string Dump() const;
    int getValue();
};

// RelExp
class RelationalExpressionAST : public BaseAST {
public:
    enum TAG {ADD_EXP, OP_REL_EXP};
    TAG tag;
    std::unique_ptr<AdditionExpressionAST> add_exp;
    std::unique_ptr<RelationalExpressionAST> rel_exp_1;
    std::unique_ptr<AdditionExpressionAST> add_exp_2;
    char rel_op[2];     // <,>,<=,>=
    std::string Dump() const;
    int getValue();
};

class EqualityExpressionAST : public BaseAST {
public:
    enum TAG {REL_EXP, OP_EQ_EXP};
    TAG tag;
    std::unique_ptr<RelationalExpressionAST> rel_exp;
    std::unique_ptr<EqualityExpressionAST> eq_exp_1;
    std::unique_ptr<RelationalExpressionAST> rel_exp_2;
    char eq_op;     // =,!
    std::string Dump() const;
    int getValue();
};

class LogicalAndExpressionAST : public BaseAST {
public:
    enum TAG {EQ_EXP, OP_L_AND_EXP};
    TAG tag;
    std::unique_ptr<EqualityExpressionAST> eq_exp;
    std::unique_ptr<LogicalAndExpressionAST> l_and_exp_1;
    std::unique_ptr<EqualityExpressionAST> eq_exp_2;
    std::string Dump() const;
    int getValue();
};

class LogicalOrExpressionAST : public BaseAST {
public:
    enum TAG {L_AND_EXP, OP_L_OR_EXP};
    TAG tag;
    std::unique_ptr<LogicalAndExpressionAST> l_and_exp;
    std::unique_ptr<LogicalOrExpressionAST> l_or_exp_1;
    std::unique_ptr<LogicalAndExpressionAST> l_and_exp_2;
    std::string Dump() const;
    int getValue();
};

class FunctionCallParametersAST : public BaseAST {
public:
    std::vector<std::unique_ptr<ExpressionAST>> exps;
    std::string Dump() const;
};
