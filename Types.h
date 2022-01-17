#ifndef TYPES_H_
#define TYPES_H_


#include <string>
#include <vector>
#include <iostream>
#include "Register.h"
#include "StringGen.h"
#include "bp.hpp"


enum OPERATION_TYPE {
    NONE,
    EXP_TO_LPAREN_EXP_RPAREN,
    EXP_TO_EXP_BINOP_MUL_EXP,
    EXP_TO_EXP_BINOP_ADD_EXP,
    EXP_TO_ID,
    EXP_TO_NUM,
    EXP_TO_NUM_B,
    EXP_TO_STRING,
    EXP_TO_TRUE,
    EXP_TO_FALSE,
    EXP_TO_NOT_EXP,
    EXP_TO_EXP_AND_EXP,
    EXP_TO_EXP_OR_EXP,
    EXP_TO_EXP_RELOP_COMPARE_EXP,
    EXP_TO_EXP_RELOP_EQUAL_EXP,
    EXP_TO_CAST,
    STATEMETN_TO_IF,
    STATEMENT_TO_TYPE_ID,
    STATEMENT_TO_TYPE_ID_EXP,
    STATEMENT_TO_ID_EXP,
    STATEMENTS_TO_STATEMENT,
    STATEMENTS_TO_STATEMENTS_STATEMENT
};


enum ExpCase {
    REGULAR,
    SIMPLE_NUM,
    CONST_ID
};


class AbsCls {
public:
  virtual std::string get_name() { std::cerr << "1 Unexpected error" << std::endl; exit(1); return std::string(); }
  virtual std::string get_type() { std::cerr << "2 Unexpected error" << std::endl; exit(1); return std::string(); }
  virtual std::vector<std::string> get_args_types() { std::cerr << "3 Unexpected error" << std::endl; exit(1); return std::vector<std::string>(); }
  virtual std::vector<std::string> get_args_names() { std::cerr << "4 Unexpected error" << std::endl; exit(1); return std::vector<std::string>(); }
  virtual void add_new_func_arg(AbsCls*) { std::cerr << "5 Unexpected error" << std::endl; exit(1); }
  virtual bool get_is_const() { std::cerr << "6 Unexpected error" << std::endl; exit(1); return true; }
  virtual int get_yylineno() { std::cerr << "7 Unexpected error" << std::endl; exit(1); return 0; }
  virtual std::string get_value() { std::cerr << "8 Unexpected error" << std::endl; exit(1); return std::string(); }
  virtual std::string get_reg() { std::cerr << "9 Unexpected error" << std::endl; exit(1); return std::string(); }
  virtual std::vector<pair<int,BranchLabelIndex>> get_truelist() { std::cerr << "10 Unexpected error" << std::endl; exit(1); return std::vector<pair<int,BranchLabelIndex>>(); }
  virtual std::vector<pair<int,BranchLabelIndex>> get_falselist() { std::cerr << "11 Unexpected error" << std::endl; exit(1); return std::vector<pair<int,BranchLabelIndex>>(); }
  virtual std::vector<pair<int,BranchLabelIndex>> get_nextlist() { std::cerr << "12 Unexpected error" << std::endl; exit(1); return std::vector<pair<int,BranchLabelIndex>>(); }
  virtual std::string get_str_gen_name() { std::cerr << "13 Unexpected error" << std::endl; exit(1); return std::string(); }
  virtual std::string get_size() { std::cerr << "14 Unexpected error" << std::endl; exit(1); return std::string(); }
  virtual std::string get_label() { std::cerr << "15 Unexpected error" << std::endl; exit(1); return std::string(); }
  virtual ExpCase get_exp_case() { std::cerr << "16 Unexpected error" << std::endl; exit(1); return REGULAR; }
  virtual bool get_is_empty() { std::cerr << "17 Unexpected error" << std::endl; exit(1); return true; }
  virtual ~AbsCls() = default;
};


class FormalDeclCls : public AbsCls {
private:
  bool is_const;
  std::string type;
  std::string name;
public:
  FormalDeclCls(bool is_const, std::string type, std::string name);
  std::string get_name() override { return name; }
  std::string get_type() override { return type; }
  bool get_is_const() override { return is_const; }
};


class FormalsListCls : public AbsCls {
private:
  std::vector<std::string> args_types;
  std::vector<std::string> args_names;
public:
  std::vector<std::string> get_args_types() override { return args_types; }
  std::vector<std::string> get_args_names() override { return args_names; }
  FormalsListCls() = default;
  FormalsListCls(std::vector<std::string> args_types, std::vector<std::string> args_names);
  void add_new_func_arg(AbsCls*) override;
};


class FormalsCls : public AbsCls {
private:
  std::vector<std::string> args_types;
  std::vector<std::string> args_names;
public:
  std::vector<std::string> get_args_types() override { return args_types; }
  std::vector<std::string> get_args_names() override { return args_names; }
  FormalsCls(std::vector<std::string> args_types = std::vector<std::string>(),
             std::vector<std::string> args_names = std::vector<std::string>());
};


class RetTypeCls : public AbsCls {
private:
  std::string name;
public:
  RetTypeCls(std::string name);
  std::string get_name() override { return name; }
};


class TypeCls : public AbsCls {
private:
  std::string name;
public:
  TypeCls(std::string name);
  std::string get_name() override { return name; }
};


class TypeAnnotationCls : public AbsCls {
private:
  bool is_const;
public:
  TypeAnnotationCls(bool is_const);
  bool get_is_const() override { return is_const; }
};


class IDCls : public AbsCls {
private:
  std::string name;
  int line_num;
public:
  IDCls(std::string name, int line_num);
  std::string get_name() override { return name; }
  int get_yylineno() override { return line_num; }
};

class ExpCls : public AbsCls {
private:
	std::string type;
    std::string value;
    bool is_simple_num;
    ExpCase exp_case;
    Register reg;
    std::vector<pair<int,BranchLabelIndex>> truelist;
    std::vector<pair<int,BranchLabelIndex>> falselist;
    std::vector<pair<int,BranchLabelIndex>> nextlist;
public:
    ExpCls(std::string type,
           std::string value = std::string("0"),
           ExpCase exp_case = REGULAR,
           OPERATION_TYPE op = NONE,
           AbsCls* cls1 = nullptr,
           AbsCls* cls2 = nullptr,
           AbsCls* cls3 = nullptr);
	std::string get_type() override { return type; }
    std::string get_value() override { return value; }
    ExpCase get_exp_case() override { return exp_case; }
    std::string get_reg() override { return reg.get_name(); }
    std::vector<pair<int,BranchLabelIndex>> get_truelist() override { return truelist; }
    std::vector<pair<int,BranchLabelIndex>> get_falselist() override { return falselist; }
    std::vector<pair<int,BranchLabelIndex>> get_nextlist() override { return nextlist; }
};


class CallCls : public AbsCls {
private:
	std::string type;
public:
	CallCls(std::string type);
	std::string get_type() override { return type; }
};


class ExpListCls : public AbsCls {
private:
  std::vector<std::string> args_types;
public:
  ExpListCls() = default;
  ExpListCls(std::vector<std::string> args_types);
  std::vector<std::string> get_args_types() override { return args_types; }
  void add_new_func_arg(AbsCls*) override;
};


class NumCls : public AbsCls {
private:
  std::string value;
public:
  NumCls(std::string value);
  std::string get_value() override { return value; }
};


class BinopMulCls : public AbsCls {
private:
  std::string value;
public:
  BinopMulCls(std::string value);
  std::string get_value() override { return value; }
};


class BinopAddCls : public AbsCls {
private:
  std::string value;
public:
  BinopAddCls(std::string value);
  std::string get_value() override { return value; }
};


class StringCls : public AbsCls {
private:
    StringGen str_gen;
    std::string value;
    std::string size;
public:
  StringCls(std::string value);
  std::string get_str_gen_name() override { return str_gen.get_name(); }
  std::string get_value() override { return value; }
  std::string get_size() override { return size; }
};


class RelopCompareCls : public AbsCls {
private:
  std::string value;
public:
  RelopCompareCls(std::string value);
  std::string get_value() override { return value; }
};


class RelopEqualCls : public AbsCls {
private:
  std::string value;
public:
  RelopEqualCls(std::string value);
  std::string get_value() override { return value; }
};


class MCls : public AbsCls {
private:
    std::string label;
public:
    MCls();
    std::string get_label() override { return label; }
};


class NCls : public AbsCls {
private:
    std::vector<pair<int,BranchLabelIndex>> nextlist;
public:
    NCls();
    std::vector<pair<int,BranchLabelIndex>> get_nextlist() override { return nextlist; }
};


class StatementCls : public AbsCls {
private:
    std::vector<pair<int,BranchLabelIndex>> nextlist;
public:
    StatementCls(OPERATION_TYPE op = NONE,
                 AbsCls* cls1 = nullptr,
                 AbsCls* cls2 = nullptr,
                 AbsCls* cls3 = nullptr,
                 AbsCls* cls4 = nullptr,
                 AbsCls* cls5 = nullptr);
    std::vector<pair<int,BranchLabelIndex>> get_nextlist() override { return nextlist; }
};


class StatementsCls : public AbsCls {
private:
    std::vector<pair<int,BranchLabelIndex>> nextlist;
public:
    StatementsCls(OPERATION_TYPE op = NONE,
                 AbsCls* cls1 = nullptr,
                 AbsCls* cls2 = nullptr,
                 AbsCls* cls3 = nullptr);
    std::vector<pair<int,BranchLabelIndex>> get_nextlist() override { return nextlist; }
};



class IfElseCls : public AbsCls {
private:
    std::vector<pair<int,BranchLabelIndex>> nextlist;
    std::string label;
    bool is_empty;
public:
    IfElseCls(AbsCls* cls1 = nullptr, AbsCls* cls2 = nullptr, bool is_empty = true);
    std::vector<pair<int,BranchLabelIndex>> get_nextlist() override { return nextlist; }
    std::string get_label() override { return label; }
    bool get_is_empty() override {return is_empty; }
};


#endif // TYPES_H_
