#include <cassert>
#include <string>
#include <vector>
#include "Utilities.h"
#include "Tables.h"
#include "hw3_output.hpp"
#include "Register.h"
#include "bp.hpp"


SymbolTableStack symbol_table_stack;
OffsetTableStack offset_table_stack;
ConstTable const_table;
int inside_while_counter;
CodeBuffer& code_buffer = CodeBuffer::instance();
LocalVarsReg local_vars_reg;


static bool is_type_starts_with_const(const std::string& type);
static SymbolTableEntry& get_last_func();
static void check_func_args(std::vector<std::string>& args_names);


static bool is_type_starts_with_const(const std::string& type){
	return std::string::npos != type.find("CONST");
}

static SymbolTableEntry& get_last_func() {
	return symbol_table_stack.first_symbol_table().top_symbol_table_entry();
}


static void check_func_args(std::vector<std::string>& args_names) {
	for (size_t i = 0; i < args_names.size(); i++) {
		for (size_t j = i + 1; j < args_names.size(); j++) {
			if (args_names[i] == args_names[j]) {
				output::errorDef(yylineno, args_names[i]);
				exit(1);
			}
		}
	}
	for (std::vector<std::string>::iterator it = args_names.begin(); it != args_names.end(); ++it) {
		if (symbol_table_stack.get_entry_by_name(*it)) {
			output::errorDef(yylineno, *it);
			exit(1);
		}
	}
}


std::string size_by_type(std::string type) {
	if (type.find("INT") != std::string::npos) {
		return "i32";
	}
	else if (type.find("BOOL") != std::string::npos) {
		return "i1";
	}
	else if (type.find("BYTE") != std::string::npos) {
		return "i8";
	}
	else {
		return "void";
	}
}


std::string strip_const(std::string type){
	if(std::string::npos  == type.find("CONST")){
		return type;
	}
	std::string stripped_type = type.replace(0, 6, "");
	return stripped_type;
}


std::vector<std::string> strip_const(std::vector<std::string> types){
	std::vector<std::string> stripped_types;
	for (std::vector<std::string>::iterator it = types.begin(); it != types.end(); ++it) {
		stripped_types.push_back(strip_const(*it));
	}
	return stripped_types;
}


void check_main_exist() {
  SymbolTableEntry* main_entry = symbol_table_stack.get_entry_by_name("main");
  if (!main_entry || !(main_entry->get_type() == "VOID" && main_entry->get_is_func() && main_entry->get_arguments().empty())) {
    output::errorMainMissing();
    exit(1);
  }
}


void open_scope() {
  symbol_table_stack.push_symbol_table();
  offset_table_stack.push_offset();
}


void close_scope() {
  //output::endScope();
  //symbol_table_stack.print_top_symbol_table();
  symbol_table_stack.pop_symbol_table();
  offset_table_stack.pop_offset();
}


void add_func_to_symbol_table(std::string name, std::string ret_type, std::vector<std::string> arguments, int line_num) {
  if (symbol_table_stack.get_entry_by_name(name)) {
  	output::errorDef(line_num, name);
  	exit(1);
  }
  std::vector<std::string> reverse_args;
  for (std::vector<std::string>::reverse_iterator it = arguments.rbegin(); it != arguments.rend(); ++it) {
		reverse_args.push_back(*it);
  }
  SymbolTable& top_symbol_table = symbol_table_stack.top_symbol_table();
  top_symbol_table.push_entry(name, 0, ret_type, true, reverse_args);
}


void add_func_args_to_symbol_table(std::vector<std::string> args_types, std::vector<std::string> args_names) {
  assert(args_types.size() == args_names.size());
  check_func_args(args_names);
  if (!args_types.empty()){
	 SymbolTable& top_symbol_table = symbol_table_stack.top_symbol_table();
	 int arg_offset = -1;
	 for (size_t i = (args_types.size() - 1); i >= 0; i--) {
		top_symbol_table.push_entry(args_names[i], arg_offset, args_types[i]);
		arg_offset--;
		if (i == 0){
			break;
		}
	 }
  }
}


void add_var_to_symbol_table(bool is_const, std::string type, std::string name){
	if (symbol_table_stack.get_entry_by_name(name)) {
		output::errorDef(yylineno, name);
		exit(1);
	}
	SymbolTable& top_symbol_table = symbol_table_stack.top_symbol_table();
	type = is_const ? "CONST " + type : type;
	top_symbol_table.push_entry(name, offset_table_stack.top_offset(), type);
	offset_table_stack.inc_top_offset();
}

void check_matching_types(std::string type_a, std::string type_b){
	if (strip_const(type_a) != strip_const(type_b)){
		output::errorMismatch(yylineno);
		exit(1);
	}
}


void check_var_valid_for_assign(std::string name){
	SymbolTableEntry* var_entry = symbol_table_stack.get_entry_by_name(name);
	if (!var_entry || var_entry->get_is_func()) {
		output::errorUndef(yylineno, name);
		exit(1);
	}
	else if(is_type_starts_with_const(var_entry->get_type())){
		output::errorConstMismatch(yylineno);
		exit(1);
	}
}


std::string& get_id_type(std::string name){
	SymbolTableEntry* id_entry = symbol_table_stack.get_entry_by_name(name);
	if (id_entry == nullptr){
		output::errorUndef(yylineno, name);
		exit(1);
	}
	return id_entry->get_type();
}


void check_assign_to_const(bool is_const){
	if(is_const){
		output::errorConstDef(yylineno);
		exit(1);
	}
}


std::string handle_binop_exp(std::string type_a, std::string type_b){
	if (std::string::npos == type_a.find("INT") && std::string::npos == type_a.find("BYTE")){
		output::errorMismatch(yylineno);
		exit(1);
	}
	if (std::string::npos == type_b.find("INT") && std::string::npos == type_b.find("BYTE")){
		output::errorMismatch(yylineno);
		exit(1);
	}
	if (std::string::npos != type_a.find("INT") || std::string::npos != type_b.find("INT")){
		return "INT";
	}
	return "BYTE";
}


void handle_relop_exp(std::string type_a, std::string type_b){
	if (std::string::npos == type_a.find("INT") && std::string::npos == type_a.find("BYTE")){
		output::errorMismatch(yylineno);
		exit(1);
	}
	if (std::string::npos == type_b.find("INT") && std::string::npos == type_b.find("BYTE")){
		output::errorMismatch(yylineno);
		exit(1);
	}
}

void handle_cast_exp(std::string type_a, std::string type_b, std::string value){
	if (std::string::npos == type_a.find("INT") && std::string::npos == type_a.find("BYTE")){
		output::errorMismatch(yylineno);
		exit(1);
	}
	if (std::string::npos == type_b.find("INT") && std::string::npos == type_b.find("BYTE")){
		output::errorMismatch(yylineno);
		exit(1);
	}
	if (std::string::npos != type_a.find("BYTE")) {
		check_byte_range(value);
	}
}


void check_valid_types_for_assign(std::string left_type, std::string right_type){
	if (!(std::string::npos != left_type.find("INT") && std::string::npos != right_type.find("BYTE"))){
		check_matching_types(left_type, right_type);
	}
}


std::string& check_valid_func_call(std::string func_name, std::vector<std::string> args_types) {
	SymbolTableEntry* func_entry = symbol_table_stack.get_entry_by_name(func_name);
	if(!func_entry || func_entry->get_is_func() == false) {
		 output::errorUndefFunc(yylineno, func_name);
		 exit(1);
	}

	std::vector<std::string>& func_entry_args = func_entry->get_arguments();
	std::vector<std::string> reverse_args_types;
	for (std::vector<std::string>::reverse_iterator it = args_types.rbegin(); it != args_types.rend(); ++it) {
		reverse_args_types.push_back(*it);
	}

	if (func_entry_args.size() != reverse_args_types.size()) {
		output::errorPrototypeMismatch(yylineno, func_name, func_entry_args);
		exit(1);
	}

	for (size_t i = 0; i < func_entry_args.size(); i++) {
		std::string stripped_type_a = strip_const(func_entry_args[i]);
		std::string stripped_type_b = strip_const(reverse_args_types[i]);
		if (!(stripped_type_a == "INT" && stripped_type_b == "BYTE")) {
			if (stripped_type_a != stripped_type_b){
				output::errorPrototypeMismatch(yylineno, func_name, func_entry_args);
				exit(1);
			}
		}
	}
	return func_entry->get_type();
}


void check_valid_ret_type(std::string ret_type) {
	std::string& last_func_ret_type = get_last_func().get_type();
	if (!(std::string::npos != last_func_ret_type.find("INT") && std::string::npos != ret_type.find("BYTE"))) {
		check_matching_types(last_func_ret_type, ret_type);
	}
}


void assert_not_void(std::string ret_type) {
	if (ret_type == "VOID") {
		output::errorMismatch(yylineno);
		exit(1);
	}
}


void check_legal_break(){
	if(!inside_while_counter){
		output::errorUnexpectedBreak(yylineno);
		exit(1);
	}
}


void check_legal_continue(){
	if(!inside_while_counter){
		output::errorUnexpectedContinue(yylineno);
		exit(1);
	}
}


void check_byte_range(std::string value) {
	int numeric_value = stoi(value);
	if (numeric_value < 0 || numeric_value > 255) {
		output::errorByteTooLarge(yylineno, value);
		exit(1);
	}
}


void declare_standart_functions() {
	code_buffer.emitGlobal("declare i32 @printf(i8*, ...)");
	code_buffer.emitGlobal("declare void @exit(i32)");
}

void declare_divion_error_str() {
	code_buffer.emitGlobal("@.div_error = internal constant [23 x i8] c\"Error division by zero\\00\"");
}


void emit_default_return(std::string ret_type) {
	if (ret_type == "VOID") {
		code_buffer.emit(DOUBLE_TAB + "ret void");
	}
	else {
		code_buffer.emit(DOUBLE_TAB + "ret " + size_by_type(ret_type) + " 0");
	}
	code_buffer.emit("}");
}


void emit_define_function(std::string ret_type, std::string func_name, std::vector<std::string> args_types) {
	//define i32 @main() {   ; i32()*
	std::string code;
	code = "define " + size_by_type(ret_type) + " @" + func_name + "(";
	for (std::vector<std::string>::iterator it = args_types.begin(); it != args_types.end(); ++it) {
		code += size_by_type((*it)) + ", ";
	}
	if (!args_types.empty()) {
		code = code.substr(0, code.size() - 2);
	}
	code += ") {";
	code_buffer.emit(code);
}
