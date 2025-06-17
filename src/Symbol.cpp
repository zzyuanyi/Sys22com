#include "Symbol.h"
#include <iostream>
using namespace std;

void IdentifierManager::reset(){
    counter = 0;
}

std::string IdentifierManager::getTemporaryName(){
    return "%" + std::to_string(counter++);
}

std::string IdentifierManager::getIdentifierName(const std::string &identifier){
    auto iter = identifier_map.find(identifier);
    if(iter == identifier_map.end()){
        identifier_map.insert(make_pair(identifier, 1));
        return "@" + identifier + "_0";
    }
    return "@" + identifier + "_"  + std::to_string(iter->second++);
}

std::string IdentifierManager::getLabelIdentifier(const std::string &label){
    auto iter = identifier_map.find(label);
    if(iter == identifier_map.end()){
        identifier_map.insert(make_pair(label, 1));
        return "%" + label + "_0";
    }
    return "%" + label + "_"  + std::to_string(iter->second++);
}

SysYType::SysYType():type(INTEGER), value(-1), next(nullptr){}

SysYType::SysYType(TypeCategory _type):type(_type), value(-1), next(nullptr){}

SysYType::SysYType(TypeCategory _type, int _value): type(_type), value(_value), next(nullptr){}

SysYType::SysYType(TypeCategory _type, const std::vector<int> &dimensions):type(_type), value(-1), next(nullptr){
    buildFromArrayType(dimensions, _type == CONSTANT_ARRAY);
}

SysYType::~SysYType() { 
    auto current = next;
    if(current != nullptr){
        auto previous = current;
        current = current->next;
        delete previous;
    }
}

void SysYType::buildFromArrayType(const std::vector<int> &dimensions, bool is_constant){
    TypeCategory type = is_constant ? CONSTANT_ARRAY : VARIABLE_ARRAY;
    SysYType *current = this;
    for(int dimension : dimensions){
        current->type = type;
        current->value = dimension;
        current->next = new SysYType();
        current = current->next;
    }
    current->next = new SysYType();
    current = current->next;
    current->type = is_constant ? INTEGER_CONSTANT : INTEGER;
}

void SysYType::getArrayType(std::vector<int> &dimensions){
    dimensions.clear();
    SysYType *current = this;
    while(current->next != nullptr && (current->type == CONSTANT_ARRAY || current->type == VARIABLE_ARRAY)){
        dimensions.push_back(value);
        current = current->next;
    }
    return;
}

SymbolEntry::SymbolEntry(const std::string &_identifier, const std::string &_ir_name, SysYType *_type): 
    identifier(_identifier), ir_name(_ir_name), type_info(_type){
}

SymbolEntry::~SymbolEntry(){
    if(type_info) delete type_info;
}

SymbolTable::~SymbolTable(){
    for(auto &entry : symbol_map){
        delete entry.second;
    }
}

void SymbolTable::insert(SymbolEntry *symbol){
    symbol_map.insert({symbol->identifier, symbol});
}

void SymbolTable::insert(const std::string &identifier, const std::string &ir_name, SysYType::TypeCategory _type, int value){
    SysYType *type_info = new SysYType(_type, value);
    SymbolEntry *symbol = new SymbolEntry(identifier, ir_name, type_info);
    insert(symbol);
}

void SymbolTable::insertInteger(const std::string &identifier, const std::string &ir_name){
    insert(identifier, ir_name, SysYType::INTEGER, UNKNOWN);
}

void SymbolTable::insertIntegerConstant(const std::string &identifier, const std::string &ir_name, int value){
    insert(identifier, ir_name, SysYType::INTEGER_CONSTANT, value);
}

void SymbolTable::insertFunction(const std::string &identifier, const std::string &ir_name, SysYType::TypeCategory _type){
    insert(identifier, ir_name, _type, UNKNOWN);
}

void SymbolTable::insertArray(const std::string &identifier, const std::string &ir_name, const std::vector<int> &dimensions, SysYType::TypeCategory _type){
    SysYType *type_info = new SysYType(_type, dimensions);
    SymbolEntry *symbol = new SymbolEntry(identifier, ir_name, type_info);
    insert(symbol);
}

bool SymbolTable::exists(const std::string &identifier){
    return symbol_map.find(identifier) != symbol_map.end();
}

SymbolEntry *SymbolTable::search(const std::string &identifier){
    return symbol_map.find(identifier)->second;
}

int SymbolTable::getValue(const std::string &identifier){
    return symbol_map.find(identifier)->second->type_info->value;
}

SysYType *SymbolTable::getType(const std::string &identifier){
    return symbol_map.find(identifier)->second->type_info;
}

std::string SymbolTable::getName(const std::string &identifier){
    return symbol_map.find(identifier)->second->ir_name;
}

void SymbolTableStack::allocate(){
    symbol_table_stack.emplace_back(new SymbolTable());
}

void SymbolTableStack::deallocate(){
    symbol_table_stack.pop_back();
}

void SymbolTableStack::resetIdentifierManager(){
    identifier_manager.reset();
}

void SymbolTableStack::insert(SymbolEntry *symbol){
    symbol_table_stack.back()->insert(symbol);
}

void SymbolTableStack::insert(const std::string &identifier, SysYType::TypeCategory _type, int value){
    string ir_name = identifier_manager.getIdentifierName(identifier);
    symbol_table_stack.back()->insert(identifier, ir_name, _type, value);
}

void SymbolTableStack::insertInteger(const std::string &identifier){
    string ir_name = identifier_manager.getIdentifierName(identifier);
    symbol_table_stack.back()->insertInteger(identifier, ir_name);
}

void SymbolTableStack::insertIntegerConstant(const std::string &identifier, int value){
    string ir_name = identifier_manager.getIdentifierName(identifier);
    symbol_table_stack.back()->insertIntegerConstant(identifier, ir_name, value);
}

void SymbolTableStack::insertFunction(const std::string &identifier, SysYType::TypeCategory _type){
    string ir_name = "@" + identifier;
    symbol_table_stack.back()->insertFunction(identifier, ir_name, _type);
}

void SymbolTableStack::insertArray(const std::string &identifier, const std::vector<int> &dimensions, SysYType::TypeCategory _type){
    string ir_name = identifier_manager.getIdentifierName(identifier);
    symbol_table_stack.back()->insertArray(identifier, ir_name, dimensions, _type);
}

bool SymbolTableStack::exists(const std::string &identifier){
    for(int i = (int)symbol_table_stack.size() - 1; i >= 0; --i){
        if(symbol_table_stack[i]->exists(identifier))
            return true;
    }
    return false;
}

int SymbolTableStack::getValue(const std::string &identifier){
    int i = (int)symbol_table_stack.size() - 1;
    for(; i >= 0; --i){
        if(symbol_table_stack[i]->exists(identifier))
            break;
    }
    return symbol_table_stack[i]->getValue(identifier);
}

SysYType *SymbolTableStack::getType(const std::string &identifier){
    int i = (int)symbol_table_stack.size() - 1;
    for(; i >= 0; --i){
        if(symbol_table_stack[i]->exists(identifier))
            break;
    }
    return symbol_table_stack[i]->getType(identifier);
}

std::string SymbolTableStack::getName(const std::string &identifier){
    int i = (int)symbol_table_stack.size() - 1;
    for(; i >= 0; --i){
        if(symbol_table_stack[i]->exists(identifier))
            break;
    }
    return symbol_table_stack[i]->getName(identifier);
}

std::string SymbolTableStack::getTemporaryName(){
    return identifier_manager.getTemporaryName();
}

std::string SymbolTableStack::getLabelIdentifier(const std::string &label){
    return identifier_manager.getLabelIdentifier(label);
}

std::string SymbolTableStack::getVariableName(const std::string& variable){
    return identifier_manager.getIdentifierName(variable);
}