#include "AST.h"
#include "Symbol.h"
#include <memory>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include "utils.h"
using namespace std;

KoopaString koopa_string;
SymbolTableStack symbol_table_stack;
BlockController block_controller;
WhileStack while_stack;

/**
 * helper functions
*/

/**
 * 完成对Local数组初始化的IR生成
 * @param array_name: 数组在Koopa IR中的名字 
 * @param content_ptr: 指向数组的内容，例如{"1", "%2"}
 * @param dimensions: 描述数组类型，i.e. 各个维度的长
*/
void initializeArray(std::string array_name, std::string *content_ptr, const std::vector<int> &dimensions){    
    int dimension_size = dimensions[0];
    if(dimensions.size() == 1){
        for(int i = 0; i < dimension_size; ++i){
            string temp_ptr = symbol_table_stack.getTmpName();
            koopa_string.getelemptr(temp_ptr, array_name, i);
            koopa_string.store(content_ptr[i], temp_ptr);
        }
    } else {
        vector<int> sub_dimensions(dimensions.begin() + 1, dimensions.end());
        int element_width = 1;
        for(auto dim : sub_dimensions)  element_width *= dim;
        for(int i = 0; i < dimension_size; ++i){
            string temp_ptr = symbol_table_stack.getTmpName();
            koopa_string.getelemptr(temp_ptr, array_name, i);
            initializeArray(temp_ptr, content_ptr + i * element_width, sub_dimensions);
        }
    }
}

/**
 * 返回数组中某个元素的指针
 * @param array_name: 数组在Koopa IR中的名字 
 * @param indices: 元素在数组中的下标
*/
std::string getElementPointer(const std::string &array_name, const std::vector<std::string>& indices){
    if(indices.size() == 1){
        string temp_ptr = symbol_table_stack.getTmpName();
        koopa_string.getelemptr(temp_ptr, array_name, indices[0]);
        return temp_ptr;
    } else {
        string temp_ptr = symbol_table_stack.getTmpName();
        koopa_string.getelemptr(temp_ptr, array_name, indices[0]);
        return getElementPointer(
            temp_ptr,
            vector<string>(indices.begin() + 1, indices.end())
        );
    }
}

void CompilationUnitAST::Dump()const {
    symbol_table_stack.alloc(); // 全局作用域
    this->DumpGlobalVariables();
    // 库函数声明
    koopa_string.declLibFunc();
    symbol_table_stack.insertFUNC("getint", SysYType::SYSY_FUNC_INT);
    symbol_table_stack.insertFUNC("getch", SysYType::SYSY_FUNC_INT);
    symbol_table_stack.insertFUNC("getarray", SysYType::SYSY_FUNC_INT);
    symbol_table_stack.insertFUNC("putint", SysYType::SYSY_FUNC_VOID);
    symbol_table_stack.insertFUNC("putch", SysYType::SYSY_FUNC_VOID);
    symbol_table_stack.insertFUNC("putarray", SysYType::SYSY_FUNC_VOID);
    symbol_table_stack.insertFUNC("starttime", SysYType::SYSY_FUNC_VOID);
    symbol_table_stack.insertFUNC("stoptime", SysYType::SYSY_FUNC_VOID);

    int function_count = function_definitions.size();
    for(int i = 0; i < function_count; ++i)
        function_definitions[i]->Dump();
    symbol_table_stack.quit();
}

void CompilationUnitAST::DumpGlobalVariables() const{
     // 全局变量
    for(auto &declaration : declarations){
        if(declaration->tag == DeclarationAST::CONST_DECL){
            for(auto &const_def : declaration->const_decl->const_defs){
                const_def->Dump(true);
            }
        } else {
           for(auto &var_def: declaration->var_decl->var_defs){
                var_def->Dump(true);
           }
        }
    }
    koopa_string.append("\n");
}

void FunctionDefinitionAST::Dump() const {
    symbol_table_stack.resetNameManager();
    
    // 函数名加到符号表
    symbol_table_stack.insertFUNC(function_name, return_type->tag == BasicTypeAST::INT ? SysYType::SYSY_FUNC_INT : SysYType::SYSY_FUNC_VOID);

    // fun @main(): i32 {
    koopa_string.append("fun " + symbol_table_stack.getName(function_name) + "(");

    // 提前进入到函数内的block，之后把参数load到变量中
    symbol_table_stack.alloc();
    vector<string> parameter_names;   //KoopaIR参数列表的名字
    
    // 打印函数变量名，并保存到parameter_names中
    if(parameters != nullptr){
        auto &param_list = parameters->parameter_list;
        int param_count = param_list.size();
        parameter_names.push_back(symbol_table_stack.getVarName(param_list[0]->parameter_name));
        koopa_string.append(parameter_names.back() + ": ");
        koopa_string.append(param_list[0]->Dump());
        for(int i = 1; i < param_count; ++i){
            koopa_string.append(", ");
            parameter_names.push_back(symbol_table_stack.getVarName(param_list[i]->parameter_name));
            koopa_string.append(parameter_names.back() + ": ");
            koopa_string.append(param_list[i]->Dump());
        }
    }
    koopa_string.append(")");
    if(return_type->tag == BasicTypeAST::INT){
        koopa_string.append(": i32");
    }
    koopa_string.append(" {\n");

    // 进入Block
    block_controller.set();
    koopa_string.label("%entry");

    // 提前把虚参加载到变量中
    if(parameters != nullptr){
        int i = 0;
        for(auto &param : parameters->parameter_list){
            string var = parameter_names[i++];

            if(param->param_type == FunctionParameterAST::SINGLE_VARIABLE){
                symbol_table_stack.insertINT(param->parameter_name);
                string name = symbol_table_stack.getName(param->parameter_name);

                koopa_string.alloc(name);
                koopa_string.store(var, name);
            }else{
                
                vector<int> dimensions;
                vector<int> padded_dimensions;
                padded_dimensions.push_back(-1);

                param->getArrayDimensions(dimensions);
                for(int dim : dimensions) padded_dimensions.push_back(dim);
                
                symbol_table_stack.insertArray(param->parameter_name, padded_dimensions, SysYType::SYSY_ARRAY);
                string name = symbol_table_stack.getName(param->parameter_name);

                koopa_string.alloc(name, "*" + koopa_string.getArrayType(dimensions));
                koopa_string.store(var, name);
            }
        }
    }

    // 具体内容交给block
    if(parameters != nullptr){
        function_body->Dump(false);
    }else{
        function_body->Dump();
    }
    // 特判空块
    if(block_controller.alive()){
        if(return_type->tag == BasicTypeAST::INT)
            koopa_string.ret("0");
        else
            koopa_string.ret("");
    }
    koopa_string.append("}\n\n");
}

string FunctionParameterAST::Dump() const{
    if(param_type == SINGLE_VARIABLE){
        return "i32";
    }
    string type_string = "i32";
    for(auto &dim_expr: array_dimensions){
        type_string = "[" + type_string + ", " + to_string(dim_expr->getValue()) + "]";
    }
    return "*" + type_string;
}

void FunctionParameterAST::getArrayDimensions(std::vector<int> &dimensions){
    dimensions.clear();
    for(auto &dim_expr: array_dimensions){
        dimensions.push_back(dim_expr->getValue());
    }
    return;
}

void CodeBlockAST::Dump(bool new_symbol_tb) const {
    // into this Block
    if(new_symbol_tb)
        symbol_table_stack.alloc();
   
    int item_count = block_items.size();

    for(int i = 0; i < item_count; ++i){
        block_items[i]->Dump();
    }
    // out of this block
    symbol_table_stack.quit();
}

void BlockElementAST::Dump() const{
    if(!block_controller.alive()) return;
    if(tag == DECL){
        decl->Dump();
    } else{
        stmt->Dump();
    }
}

void DeclarationAST::Dump() const{
    if(tag == VAR_DECL)
        var_decl->Dump();
    else
        const_decl->Dump();
}

void StatementAST::Dump() const {
    if(!block_controller.alive()) return;
    if(tag == RETURN){
        // block_controller.finish()写在这里不对！
        if(exp){
            string return_value = exp->Dump();
            koopa_string.ret(return_value);
        } else{
            koopa_string.ret("");
        }
        block_controller.finish();
    } else if(tag == ASSIGN){
        string value = exp->Dump();
        string to = lval->Dump(true);
        koopa_string.store(value, to);
    } else if(tag == BLOCK){
        block->Dump();
    } else if(tag == EXP){
        if(exp){
            exp->Dump();
        }
    } else if(tag == WHILE){
        string while_entry = symbol_table_stack.getLabelName("while_entry");
        string while_body = symbol_table_stack.getLabelName("while_body");
        string while_end = symbol_table_stack.getLabelName("while_end");
        
        while_stack.append(while_entry, while_body, while_end);

        koopa_string.jump(while_entry);

        block_controller.set();
        koopa_string.label(while_entry);
        string cond = exp->Dump();
        koopa_string.br(cond, while_body, while_end);

        block_controller.set();
        koopa_string.label(while_body);
        stmt->Dump();
        if(block_controller.alive())
            koopa_string.jump(while_entry);

        block_controller.set();
        koopa_string.label(while_end);
        while_stack.quit(); // 该while处理已结束，退栈
    } else if(tag == BREAK){
        koopa_string.jump(while_stack.getEndName());  // 跳转到while_end
        block_controller.finish();                // 当前IR的block设为不活跃
    } else if(tag == CONTINUE){
        koopa_string.jump(while_stack.getEntryName());// 跳转到while_entry
        block_controller.finish();                // 当前IR的block设为不活跃
    } else if(tag == IF){
        string s = exp->Dump();
        string t = symbol_table_stack.getLabelName("then");
        string e = symbol_table_stack.getLabelName("else");
        string j = symbol_table_stack.getLabelName("end");
        koopa_string.br(s, t, else_stmt == nullptr ? j : e);

        // IF Stmt
        block_controller.set();
        koopa_string.label(t);
        if_stmt->Dump();
        if(block_controller.alive())
            koopa_string.jump(j);

        // else stmt
        if(else_stmt != nullptr){
            block_controller.set();
            koopa_string.label(e);
            else_stmt->Dump();
            if(block_controller.alive())
                koopa_string.jump(j);

        }
        // end
        block_controller.set();
        koopa_string.label(j);
    }
    return;
}

void ConstDeclAST::Dump() const{
    int n = const_defs.size();
    for(int i = 0; i < n; ++i){
        const_defs[i]->Dump();
    }
}

void VarDeclAST::Dump() const {
    int n = var_defs.size();
    for(int i = 0; i < n; ++i){
        var_defs[i]->Dump();
    }
}

void BTypeAST::Dump() const{
    if(tag == BTypeAST::INT){
        koopa_string.append("i32");
    }
}

void ConstDefAST::Dump(bool is_global) const{
    if(tag == ARRAY){
        DumpArray(is_global);
        return;
    }
    int v = const_init_val->getValue();
    symbol_table_stack.insertINTCONST(ident, v);
}

void ConstDefAST::DumpArray(bool is_global) const{
    

    vector<int> len;
    for(auto &ce : const_exps){
        len.push_back(ce->getValue());
    }
    symbol_table_stack.insertArray(ident, len,SysYType::SYSY_ARRAY_CONST);
    string name = symbol_table_stack.getName(ident);
    string array_type = koopa_string.getArrayType(len);

    int tot_len = 1;
    for(auto i : len) tot_len *= i;
    string *init = new string[tot_len];
    for(int i = 0; i < tot_len; ++i)
        init[i] = "0";
    
    const_init_val->getInitVal(init, len);

    if(is_global){
        // Global Const Array
        koopa_string.globalAllocArray(name, array_type, koopa_string.getInitList(init, len));
    } else {
        // Local Const Array
        koopa_string.alloc(name, array_type);
        initializeArray(name, init, len);
    }
    return;
}

void VarDefAST::Dump(bool is_global) const{
    if(tag == ARRAY){
        DumpArray(is_global);
        return;
    }
    symbol_table_stack.insertINT(ident);
    string name = symbol_table_stack.getName(ident);
    if(is_global){
        if(init_val == nullptr){
            koopa_string.globalAllocINT(name);
        } else {
            int v = init_val->exp->getValue();
            koopa_string.globalAllocINT(name, to_string(v));
        }
    } else {
        koopa_string.alloc(name);
        if(init_val != nullptr){
            string s = init_val->Dump();
            koopa_string.store(s, name);
        }
    }
    return;
}

void VarDefAST::DumpArray(bool is_global) const {
    vector<int> len;
    for(auto &ce : const_exps){
        len.push_back(ce->getValue());
    }

    symbol_table_stack.insertArray(ident, len, SysYType::SYSY_ARRAY);

    string name = symbol_table_stack.getName(ident);
    string array_type = koopa_string.getArrayType(len);
    
    int tot_len = 1;
    for(auto i : len) tot_len *= i;
    string *init = new string[tot_len];
    for(int i = 0; i < tot_len; ++i)
        init[i] = "0";

    if(is_global){
        if(init_val != nullptr){
            init_val->getInitVal(init, len, true);
        }
        koopa_string.globalAllocArray(name, array_type, koopa_string.getInitList(init, len));
    } else {
        koopa_string.alloc(name, array_type);
        if(init_val == nullptr) 
            return;
        init_val->getInitVal(init, len, false);

        initializeArray(name, init, len);
    }
    return;
}

string InitValAST::Dump() const{
    return exp->Dump();
}

void InitValAST::getInitVal(std::string *ptr, const std::vector<int> &len, bool is_global) const{
    int n = len.size();
    vector<int> width(n);
    width[n - 1] = len[n - 1];
    for(int i = n - 2; i >= 0; --i){
        width[i] = width[i + 1] * len[i];
    }
    int i = 0;  // 指向下一步要填写的内存位置
    for(auto &init_val : inits){
        if(init_val->tag == EXP){
            if(is_global){
                ptr[i++] = to_string(init_val->exp->getValue());
            } else{
                ptr[i++] = init_val->Dump();
            }
        } else {
            assert(n > 1);  // 对一维数组初始化不可能再套一个Aggregate{{}}
            int j = n - 1;
            if(i == 0){
                j = 1;
            } else{
                j = n - 1;
                for(; j >= 0; --j){
                    if(i % width[j] != 0)
                        break;
                }
                assert(j < n - 1); // 保证整除最后一维
                ++j;    // j 指向最大的可除的维度
            }
            init_val->getInitVal(
                ptr + i, 
                vector<int>(len.begin() + j, len.end())
                );
            i += width[j];
        }
        if(i >= width[0]) break;
    }
}

int ConstInitValAST::getValue(){
    return const_exp->getValue();
}

// 对ptr指向的区域初始化，所指区域的数组类型由len规定
void ConstInitValAST::getInitVal(std::string *ptr, const std::vector<int> &len) const{
    int n = len.size();
    vector<int> width(n);
    width[n - 1] = len[n - 1];
    for(int i = n - 2; i >= 0; --i){
        width[i] = width[i + 1] * len[i];
    }
    int i = 0;  // 指向下一步要填写的内存位置
    for(auto &init_val : inits){
        if(init_val->tag == CONST_EXP){
            ptr[i++] = to_string(init_val->getValue());
        } else {
            assert(n > 1);  // 对一维数组初始化不可能再套一个Aggregate{{}}
            int j = n - 1;
            if(i == 0){
                j = 1;
            } else{
                j = n - 1;
                for(; j >= 0; --j){
                    if(i % width[j] != 0)
                        break;
                }
                assert(j < n - 1); // 保证整除最后一维
                ++j;    // j 指向最大的可除的维度
            }
            init_val->getInitVal(
                ptr + i, 
                vector<int>(len.begin() + j, len.end())
                );
            i += width[j];
        }
        if(i >= width[0]) break;
    }
}

string LValAST::Dump(bool dump_ptr)const{
    if(tag == VARIABLE){
        // Hint: a single a ident be a array address
        SysYType *ty = symbol_table_stack.getType(ident);
        if(ty->ty == SysYType::SYSY_INT_CONST)
            return to_string(symbol_table_stack.getValue(ident));
        else if(ty->ty == SysYType::SYSY_INT){
            if(dump_ptr == false){
                string tmp = symbol_table_stack.getTmpName();
                koopa_string.load(tmp, symbol_table_stack.getName(ident));
                return tmp;
            } else {
                return symbol_table_stack.getName(ident);
            }
        } else {
            // func(ident)
            if(ty->value == -1){
                string tmp = symbol_table_stack.getTmpName();
                koopa_string.load(tmp, symbol_table_stack.getName(ident));
                return tmp;
            }
            string tmp = symbol_table_stack.getTmpName();
            koopa_string.getelemptr(tmp, symbol_table_stack.getName(ident), "0");
            return tmp;
        }
    } else {
        vector<string> index;
        vector<int> len;

        for(auto &e: exps){
            index.push_back(e->Dump());
        }

        SysYType *ty = symbol_table_stack.getType(ident);
        ty->getArrayType(len);

        // hint: len可以是-1开头的，说明这个数组是函数中使用的参数
        // 如 a[-1][3][2],表明a是参数 a[][3][2], 即 *[3][2].
        // 此时第一步不能用getelemptr，而应该getptr

        string name = symbol_table_stack.getName(ident);
        string tmp;
        if(len.size() != 0 && len[0] == -1){
            vector<int> sublen(len.begin() + 1, len.end());
            string tmp_val = symbol_table_stack.getTmpName();
            koopa_string.load(tmp_val, name);
            string first_indexed = symbol_table_stack.getTmpName();
            koopa_string.getptr(first_indexed, tmp_val, index[0]);
            if(index.size() > 1){
                tmp = getElementPointer(
                    first_indexed,
                    vector<string>(
                        index.begin() + 1, index.end()
                    )
                );
            } else {
                tmp = first_indexed;
            }
            
        } else {
            tmp = getElementPointer(name, index);
        }   
        

        if(index.size() < len.size()){
            // 一定是作为函数参数即实参使用，因为下标不完整
            string real_param = symbol_table_stack.getTmpName();
            koopa_string.getelemptr(real_param, tmp, "0");
            return real_param;
        }
        if(dump_ptr) return tmp;
        string tmp2 = symbol_table_stack.getTmpName();
        koopa_string.load(tmp2, tmp);
        return tmp2;
    }
}

int LValAST::getValue(){
    return symbol_table_stack.getValue(ident);
}


int ConstExpAST::getValue(){
    return exp->getValue();
}

string ExpAST::Dump() const {
    return l_or_exp->Dump();
}

int ExpAST::getValue(){
    return l_or_exp->getValue();
}


string PrimaryExpAST::Dump() const{
    switch (tag)
    {
        case PARENTHESES:
            return exp->Dump();
        case NUMBER:
            return to_string(number);
        case LVAL:
            return lval->Dump();
    }
    return "";
}

int PrimaryExpAST::getValue(){
    switch (tag)
    {
        case PARENTHESES:
            return exp->getValue();
        case NUMBER:
            return number;
        case LVAL:
            return lval->getValue();
    }
    return -1;  // make g++ happy
}

string UnaryExpAST::Dump() const{
    if(tag == PRIMARY_EXP)return primary_exp->Dump();
    else if(tag == OP_UNITARY_EXP){
        string b = unary_exp->Dump();
        if(unary_op == '+') return b;

        string op = unary_op == '-' ? "sub" : "eq";
        string c = symbol_table_stack.getTmpName();
        koopa_string.binary(op, c, "0", b);
        return c;
    }else{
        // Func_Call
        string name = symbol_table_stack.getName(ident);
        string tmp = "";
        vector<string> par;
        if(st.getType(ident)->ty == SysYType::SYSY_FUNC_INT){
            tmp = symbol_table_stack.getTmpName();
        }
        if(func_params){
            int n = func_params->exps.size();
            for(int i = 0; i < n; ++i){
                par.push_back(func_params->exps[i]->Dump());
            }
        }
        koopa_string.call(tmp, symbol_table_stack.getName(ident), par);
        return tmp;
    }
}

int UnaryExpAST::getValue(){
    if(tag == PRIMARY_EXP) return primary_exp->getValue();

    int v =unary_exp->getValue();
    if(unary_op == '+') return v;
    return unary_op == '-' ? -v : !v;
}

string MulExpAST::Dump() const{
    if(tag == UNARY_EXP)return unary_exp->Dump();
    string a, b, c;
    
    a = mul_exp_1->Dump();
    b = unary_exp_2->Dump();

    string op = mul_op == '*' ? "mul":(mul_op == '/' ?"div" : "mod");
    
    c = symbol_table_stack.getTmpName();
    koopa_string.binary(op, c, a, b);
    return c;
}

int MulExpAST::getValue(){
    if(tag == UNARY_EXP) return unary_exp->getValue();

    int a = mul_exp_1->getValue(), b = unary_exp_2->getValue();

    return mul_op == '*' ? a * b : (mul_op == '/' ? a / b : a % b);
}

string AddExpAST::Dump() const{
    if(tag == MUL_EXP)return mul_exp->Dump();
    string a, b, c;
    
    a = add_exp_1->Dump();
    b = mul_exp_2->Dump();

    string op = add_op == '+' ? "add" : "sub";
    
    c = symbol_table_stack.getTmpName();
    koopa_string.binary(op, c, a, b);
    return c;
}

int AddExpAST::getValue(){
    if(tag == MUL_EXP) return mul_exp->getValue();

    int a = add_exp_1->getValue(), b = mul_exp_2->getValue();
    return add_op == '+' ? a + b : a - b;
}

string RelExpAST::Dump() const {
    if(tag == ADD_EXP) return add_exp->Dump();
    string a = rel_exp_1->Dump(), b = add_exp_2->Dump();
    string op = rel_op[1] == '=' ? (rel_op[0] == '<' ? "le" : "ge") : (rel_op[0] == '<' ? "lt" : "gt");
    string dest = symbol_table_stack.getTmpName();
    koopa_string.binary(op, dest, a, b);
    return dest;
}

int RelExpAST::getValue(){
    if(tag == ADD_EXP) return add_exp->getValue();

    int a = rel_exp_1->getValue(), b = add_exp_2->getValue();
    if(rel_op[1] == '='){
        return rel_op[0] == '>' ? (a >= b) : (a <= b);
    }
    return rel_op[0] == '>' ? (a > b) : (a < b);
}

string EqExpAST::Dump() const {
    if(tag == REL_EXP) return rel_exp->Dump();
    string a = eq_exp_1->Dump(), b =rel_exp_2->Dump();
    string op = eq_op == '=' ? "eq" : "ne";
    string dest = symbol_table_stack.getTmpName();
    koopa_string.binary(op, dest, a, b);
    return dest;
}

int EqExpAST::getValue(){
    if(tag == REL_EXP) return rel_exp->getValue();
    int a = eq_exp_1->getValue(), b = rel_exp_2->getValue();
    return eq_op == '=' ? (a == b) : (a != b);
}

string LAndExpAST::Dump() const {
    if(tag == EQ_EXP) return eq_exp->Dump();
    
    // 修改支持短路逻辑
    string result = symbol_table_stack.getVarName("SCRES");
    koopa_string.alloc(result);
    koopa_string.store("0", result);

    string lhs = l_and_exp_1->Dump();
    string then_s = symbol_table_stack.getLabelName("then_sc");
    string end_s = symbol_table_stack.getLabelName("end_sc");

    koopa_string.br(lhs, then_s, end_s);

    block_controller.set();
    koopa_string.label(then_s);
    string rhs = eq_exp_2->Dump();
    string tmp = symbol_table_stack.getTmpName();
    koopa_string.binary("ne", tmp, rhs, "0");
    koopa_string.store(tmp, result);
    koopa_string.jump(end_s);

    block_controller.set();
    koopa_string.label(end_s);
    string ret = symbol_table_stack.getTmpName();
    koopa_string.load(ret, result);
    return ret;
}

int LAndExpAST::getValue(){
    if(tag == EQ_EXP) return eq_exp->getValue();
    int a = l_and_exp_1->getValue(), b = eq_exp_2->getValue();
    return a && b;  // 注意是逻辑与
}

string LOrExpAST::Dump() const {
    if(tag == L_AND_EXP) return l_and_exp->Dump();

    // 修改支持短路逻辑
    string result = symbol_table_stack.getVarName("SCRES");
    koopa_string.alloc(result);
    koopa_string.store("1", result);

    string lhs = l_or_exp_1->Dump();

    string then_s = symbol_table_stack.getLabelName("then_sc");
    string end_s = symbol_table_stack.getLabelName("end_sc");

    koopa_string.br(lhs, end_s, then_s);

    block_controller.set();
    koopa_string.label(then_s);
    string rhs = l_and_exp_2->Dump();
    string tmp = symbol_table_stack.getTmpName();
    koopa_string.binary("ne", tmp, rhs, "0");
    koopa_string.store(tmp, result);
    koopa_string.jump(end_s);

    block_controller.set();
    koopa_string.label(end_s);
    string ret = symbol_table_stack.getTmpName();
    koopa_string.load(ret, result);
    return ret;
}

int LOrExpAST::getValue() {
    if(tag == L_AND_EXP) return l_and_exp->getValue();
    int a = l_or_exp_1->getValue(), b = l_and_exp_2->getValue();
    return a || b;
}