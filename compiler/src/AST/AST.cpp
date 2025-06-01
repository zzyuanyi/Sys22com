#include <iostream>
#include<cassert>
#include"AST/AST.hpp"
void BaseAST::Dump() const {
    std::cout << "BaseAST" << std::endl;
}
void CompUnitAST::Dump() const {
    std::cout << "CompUnitAST {}" << std::endl;
    for(auto& func_def:func_def_vec){
        func_def->Dump();
    }
    std::cout << "}" ;

}
void FuncDefAST::Dump() const {
    std::cout << "FuncDefAST {" << std::endl;
    func_type->Dump();
    std::cout << "ident: " << ident << std::endl;
    for(auto& block:block_vec){
        block->Dump();
    }
    std::cout << "}" ;
}
void FuncTypeAST::Dump() const {
    std::cout << "FuncTypeAST {" << std::endl;
    switch(func_type){
        case FuncType::FUNC_TYPE_INT:
            std::cout << "FUNC_TYPE_INT" << std::endl;
            break;
        case FuncType::FUNC_TYPE_VOID:
            std::cout << "FUNC_TYPE_VOID" << std::endl;
            break;
        case FuncType::FUNC_TYPE_FLOAT:
            std::cout << "FUNC_TYPE_FLOAT" << std::endl;
            break;
        default:
            std::cout << "Unknown FuncType" << std::endl;
    }
    std::cout << "}" ;
}
void BlockAST::Dump() const {
    std::cout << "BlockAST {" << std::endl;
    for(auto& block_item:block_item_vec){
        block_item->Dump();
    }
    std::cout << "}" ;
}

void BlockItemAST::Dump() const {
    std::cout << "BlockItemAST {" << std::endl;
    switch(block_item_type){
        case BlockItemType::BLOCK_ITEM_STMT:
            std::cout << "BLOCK_ITEM_STMT" << std::endl;
            stmt->Dump();
            break;
        case BlockItemType::BLOCK_ITEM_DECL:
            std::cout << "BLOCK_ITEM_DECL" << std::endl;
            decl->Dump();
            break;
        default:
            std::cout << "Unknown BlockItemType" << std::endl;
    }
    std::cout << "}" ;
}
void StmtAST::Dump() const {
    std::cout << "StmtAST {" << std::endl;
    switch(stmt_type){
        case StmtType::STMT_EXP:
            std::cout << "STMT_EXP" << std::endl;
            exp->Dump();
            break;
        case StmtType::STMT_BLOCK:
            std::cout << "STMT_BLOCK" << std::endl;
            block->Dump();
            break;
        case StmtType::STMT_IF:
            std::cout << "STMT_IF" << std::endl;
            exp->Dump();
            block->Dump();
            break;
        case StmtType::STMT_WHILE:
            std::cout << "STMT_WHILE" << std::endl;
            exp->Dump();
            block->Dump();
            break;
        case StmtType::STMT_RETURN:
            std::cout << "STMT_RETURN" << std::endl;
            exp->Dump();
            break;
        case StmtType::STMT_LVal_Assign_EXP:
            std::cout << "STMT_LVal_Assign_EXP" << std::endl;
            l_val->Dump();
            exp->Dump();
            break;
        case StmtType::STMT_EMPTY:
            break;
        default:
            std::cout << "Unknown StmtType" << std::endl;
    }
    std::cout << "}" ;
}
void ExpAST::Dump() const {
    std::cout << "ExpAST {" << std::endl;
    l_or_exp->Dump();
    std::cout << "}" ;
}
void LOrExpAST::Dump() const {
    std::cout << "LOrExpAST {" << std::endl;
    switch(l_or_exp_type){
        case LOrExpType::LAND_EXP:
            std::cout << "LAND_EXP" << std::endl;
            l_and_exp->Dump();
            break;
        case LOrExpType::LOR_EXP_Op_LAND_EXP:
            l_or_exp->Dump();
            std::cout << op;
            l_and_exp->Dump();
            break;
        default:
            std::cout << "Unknown LOrExpType" << std::endl;
    }
    std::cout << "}" ;
}
void LAndExpAST::Dump() const {
    std::cout << "LAndExpAST {" << std::endl;
    switch(l_and_exp_type){
        case LAndExpType::EQ_EXP:
            std::cout << "EQ_EXP" << std::endl;
            eq_exp->Dump();
            break;
        case LAndExpType::LAND_EXP_Op_EQ_EXP:
            l_and_exp->Dump();
            std::cout << op;
            eq_exp->Dump();
            break;
        default:
            std::cout << "Unknown LAndExpType" << std::endl;
    }
    std::cout << "}" ;
}
void EqExpAST::Dump() const {
    std::cout << "EqExpAST {" << std::endl;
    switch(eq_exp_type){
        case EqExpType::Rel_EXP:
            std::cout << "Rel_EXP" << std::endl;
            rel_exp->Dump();
            break;
        case EqExpType::EQ_EXP_Op_Rel_EXP:
            eq_exp->Dump();
            std::cout << op;
            rel_exp->Dump();
            break;
        default:
            std::cout << "Unknown EqExpType" << std::endl;
    }
    std::cout << "}" ;
}
void RelExpAST::Dump() const {
    std::cout << "RelExpAST {" << std::endl;
    switch(rel_exp_type){
        case RelExpType::Add_EXP:
            add_exp->Dump();
            break;
        case RelExpType::REL_EXP_Op_Add_EXP:
            rel_exp->Dump();
            std::cout << op;
            add_exp->Dump();
            break;
        default:
            std::cout << "Unknown RelExpType" << std::endl;
    }
    std::cout << "}" ;
}
void AddExpAST::Dump() const {
    std::cout << "AddExpAST {" << std::endl;
    switch(add_exp_type){
        case AddExpType::Mul_EXP:
            mul_exp->Dump();
            break;
        case AddExpType::ADD_EXP_Op_Mul_EXP:
            add_exp->Dump();
            std::cout << op;
            mul_exp->Dump();
            break;
        default:
            std::cout << "Unknown AddExpType" << std::endl;
    }
    std::cout << "}" ;
}
void MulExpAST::Dump() const {
    std::cout << "MulExpAST {" << std::endl;
    switch(mul_exp_type){
        case MulExpType::Unary_EXP:
            unary_exp->Dump();
            break;
        case MulExpType::MUL_EXP_Op_Unary_EXP:
            mul_exp->Dump();
            std::cout << op;
            unary_exp->Dump();
            break;
        default:
            std::cout << "Unknown MulExpType" << std::endl;
    }
    std::cout << "}" ;
}
void UnaryExpAST::Dump() const {
    std::cout << "UnaryExpAST {" << std::endl;
    switch(unary_exp_type){
        case UnaryExpType::Primary_EXP:
            primary_exp->Dump();
            break;
        case UnaryExpType::UnaryOp_EXP_Unary_EXP:
            unary_op->Dump();
            unary_exp->Dump();
            break;
        default:
            std::cout << "Unknown UnaryExpType" << std::endl;
    }
    std::cout << "}" ;
}
void UnaryOpAST::Dump() const {
    std::cout << "UnaryOpAST {" << std::endl;
    switch(unary_op_type){
        case UnaryOpType::PLUS:
            std::cout << "PLUS" << std::endl;
            break;
        case UnaryOpType::MINUS:
            std::cout << "MINUS" << std::endl;
            break;
        case UnaryOpType::NOT:
            std::cout << "NOT" << std::endl;
            break;
        default:
            std::cout << "Unknown UnaryOpType" << std::endl;
    }
    std::cout << "}" ;
}
void PrimaryExpAST::Dump() const {
    std::cout << "PrimaryExpAST {" << std::endl;
    switch(primary_exp_type){
        case PrimaryExpType::LVal:
            l_val->Dump();
            break;
        case PrimaryExpType::Number:
            number->Dump();
            break;
        case PrimaryExpType::Exp:
            std::cout<<"(";
            exp->Dump();
            std::cout<<")";
            break;
        default:
            std::cout << "Unknown PrimaryExpType" << std::endl;
    }
    std::cout << "}" ;
}
void NumberAST::Dump() const {
    std::cout << "NumberAST {" << std::endl;
    std::cout << "number: " << number << std::endl;
    std::cout << "}" ;
}

