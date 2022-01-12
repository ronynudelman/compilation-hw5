#ifndef TYPES_H_
#define TYPES_H_


#include <string>
#include <vector>
#include <iostream>
#include "Register.h"
#include "bp.hpp"


enum OPERATION_TYPE {
    NONE,
    EXP_TO_LPAREN_EXP_RPAREN,
    EXP_TO_EXP_BINOP_MUL_EXP,
    EXP_TO_EXP_BINOP_ADD_EXP,
    EXP_TO_NUM
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
    Register reg;
    std::vector<pair<int,BranchLabelIndex>> truelist;
    std::vector<pair<int,BranchLabelIndex>> falselist;
    std::vector<pair<int,BranchLabelIndex>> nextlist;
public:
    ExpCls(std::string type,
           std::string value = std::string("0"),
           OPERATION_TYPE op = NONE,
           AbsCls* cls1 = nullptr,
           AbsCls* cls2 = nullptr,
           AbsCls* cls3 = nullptr);
	std::string get_type() override { return type; }
    std::string get_value() override { return value; }
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


#endif // TYPES_H_
