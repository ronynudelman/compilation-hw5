#ifndef UTILITIES_H_
#define UTILITIES_H_


#include <string>
#include <vector>
#include "Tables.h"


extern SymbolTableStack symbol_table_stack;
extern OffsetTableStack offset_table_stack;
extern int inside_while_counter;
extern CodeBuffer& code_buffer;
extern LocalVarsReg local_vars_reg;


std::string size_by_type(std::string type);
std::string strip_const(std::string type);
std::vector<std::string> strip_const(std::vector<std::string> types);
void check_main_exist();
void open_scope();
void close_scope();
void add_func_to_symbol_table(std::string name, std::string ret_type, std::vector<std::string> arguments, int line_num);
void add_func_args_to_symbol_table(std::vector<std::string> args_types, std::vector<std::string> args_names);
void add_var_to_symbol_table(bool is_const, std::string type, std::string name);
void check_matching_types(std::string type_a, std::string type_b);
void check_var_valid_for_assign(std::string name);
std::string& get_id_type(std::string name);
void check_assign_to_const(bool is_const);
std::string handle_binop_exp(std::string type_a, std::string type_b);
void handle_relop_exp(std::string type_a, std::string type_b);
void handle_cast_exp(std::string type_a, std::string type_b, std::string value);
void check_valid_types_for_assign(std::string left_type, std::string right_type);
std::string& check_valid_func_call(std::string func_name, std::vector<std::string> args_types = std::vector<std::string>());
void check_valid_ret_type(std::string ret_type);
void assert_not_void(std::string ret_type);
void check_legal_break();
void check_legal_continue();
void check_byte_range(std::string value);


#endif // UTILITIES_H_
