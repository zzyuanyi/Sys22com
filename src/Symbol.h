#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <queue>
#include <memory>

class IdentifierManager{
private:
    int counter;
    std::unordered_map<std::string, int> identifier_map;
public:
    IdentifierManager():counter(0){}
    void reset();
    std::string getTemporaryName();
    std::string getIdentifierName(const std::string &identifier);
    std::string getLabelIdentifier(const std::string &label);
};


class SysYType{
    public:
        enum TypeCategory{
            INTEGER, INTEGER_CONSTANT, VOID_FUNCTION, INTEGER_FUNCTION,
            CONSTANT_ARRAY, VARIABLE_ARRAY
        };

        TypeCategory type;
        int value;
        SysYType *next;

        SysYType();
        SysYType(TypeCategory _type);
        SysYType(TypeCategory _type, int _value);
        SysYType(TypeCategory _type, const std::vector<int> &dimensions);

        ~SysYType();
        void buildFromArrayType(const std::vector<int> &dimensions, bool is_constant);
        void getArrayType(std::vector<int> &dimensions);
};

class SymbolEntry{
public:
    std::string identifier;   // SysY标识符，诸如x,y
    std::string ir_name;    // KoopaIR中的具名变量，诸如@x_1, @y_1, ..., @n_2
    SysYType *type_info;
    SymbolEntry(const std::string &_identifier, const std::string &_ir_name, SysYType *_type);
    ~SymbolEntry();
};

class SymbolTable{
public:
    const int UNKNOWN = -1;
    std::unordered_map<std::string, SymbolEntry *> symbol_map;  // identifier -> SymbolEntry *
    SymbolTable() = default;
    ~SymbolTable();
    void insert(SymbolEntry *symbol);

    void insert(const std::string &identifier, const std::string &ir_name, SysYType::TypeCategory _type, int value);

    void insertInteger(const std::string &identifier, const std::string &ir_name);

    void insertIntegerConstant(const std::string &identifier, const std::string &ir_name, int value);

    void insertFunction(const std::string &identifier, const std::string &ir_name, SysYType::TypeCategory _type);

    void insertArray(const std::string &identifier, const std::string &ir_name, const std::vector<int> &dimensions, SysYType::TypeCategory _type);

    bool exists(const std::string &identifier);

    SymbolEntry *search(const std::string &identifier);

    int getValue(const std::string &identifier);

    SysYType *getType(const std::string &identifier);

    std::string getName(const std::string &identifier);
};

class SymbolTableStack{
private:
    std::deque<std::unique_ptr<SymbolTable>> symbol_table_stack;
    IdentifierManager identifier_manager;
public:
    const int UNKNOWN = -1;
    void allocate();
    void deallocate();
    void resetIdentifierManager();
    void insert(SymbolEntry *symbol);
    void insert(const std::string &identifier, SysYType::TypeCategory _type, int value);
    void insertInteger(const std::string &identifier);
    void insertIntegerConstant(const std::string &identifier, int value);
    void insertFunction(const std::string &identifier, SysYType::TypeCategory _type);
    void insertArray(const std::string &identifier, const std::vector<int> &dimensions, SysYType::TypeCategory _type);
    bool exists(const std::string &identifier);
    int getValue(const std::string &identifier);
    SysYType *getType(const std::string &identifier);
    std::string getName(const std::string &identifier);

    std::string getTemporaryName();   // inherit from identifier manager
    std::string getLabelIdentifier(const std::string &label); // inherit from identifier manager
    std::string getVariableName(const std::string& variable);   // auxiliary variable name, such as @short_circuit_res, shouldn't insert it into Symbol table.
};