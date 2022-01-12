#ifndef TYPES_H_
#define TYPES_H_


#include <string>
#include <vector>
#include <iostream>


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
public:
	ExpCls(std::string type, std::string value = std::string("0"));
	std::string get_type() override { return type; }
  std::string get_value() override { return value; }
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


#endif // TYPES_H_
