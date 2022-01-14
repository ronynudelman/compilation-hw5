#include <string>
#include "Types.h"
#include "Utilities.h"
#include "Register.h"
#include "StringGen.h"


FormalDeclCls::FormalDeclCls(bool is_const, std::string type, std::string name) : is_const(is_const),
                                                                                  type(type),
                                                                                  name(name)
                                                                                  {}

FormalsListCls::FormalsListCls(std::vector<std::string> args_types, std::vector<std::string> args_names) : args_types(args_types), args_names(args_names){}

void FormalsListCls::add_new_func_arg(AbsCls* new_decl) {
  std::string new_type;
  if (new_decl->get_is_const()) {
    new_type = std::string("CONST ") + new_decl->get_type();
  } else {
    new_type = new_decl->get_type();
  }
  std::string new_name(new_decl->get_name());
  args_types.push_back(new_type);
  args_names.push_back(new_name);
}


FormalsCls::FormalsCls(std::vector<std::string> args_types,
                       std::vector<std::string> args_names) : args_types(args_types),
                                                              args_names(args_names)
                                                              {}


RetTypeCls::RetTypeCls(std::string name) : name(name) {}


TypeCls::TypeCls(std::string name) : name(name) {}


IDCls::IDCls(std::string name, int line_num) : name(name), line_num(line_num) {}


TypeAnnotationCls::TypeAnnotationCls(bool is_const) : is_const(is_const) {}


ExpCls::ExpCls(std::string type,
               std::string value,
               ExpCase exp_case,
               OPERATION_TYPE op,
               AbsCls* cls1,
               AbsCls* cls2,
               AbsCls* cls3) : type(type),
                               value(value),
                               exp_case(exp_case),
                               reg(Register()),
                               truelist(std::vector<pair<int,BranchLabelIndex>>()),
                               falselist(std::vector<pair<int,BranchLabelIndex>>()),
                               nextlist(std::vector<pair<int,BranchLabelIndex>>()) {
    std::string code;
    std::string global_code;
    if (op == EXP_TO_LPAREN_EXP_RPAREN) {
        if (cls1->get_exp_case() == CONST_ID) {
            code = reg.get_name() + " = add " + size_by_type(type) + " " + cls1->get_value() + ", 0";
        }
        else {
            code = reg.get_name() + " = add " + size_by_type(type) + " " + cls1->get_reg() + ", 0";
        }
        code_buffer.emit(code);
        truelist = cls1->get_truelist();
        falselist = cls1->get_falselist();
        nextlist = cls1->get_nextlist();
    }
    else if (op == EXP_TO_EXP_BINOP_MUL_EXP) {
        code = reg.get_name() + " = ";
        if (cls3->get_value() == "*") {
            code += "mul ";
        }
        else {
            code += "sdiv ";
        }
        code += size_by_type(type) + " ";
        if (cls1->get_exp_case() == CONST_ID) {
            code += cls1->get_value();
        }
        else {
            code += cls1->get_reg();
        }
        code += ", ";
        if (cls2->get_exp_case() == CONST_ID) {
            code += cls2->get_value();
        }
        else {
            code += cls2->get_reg();
        }
        code_buffer.emit(code);
        truelist = CodeBuffer::merge(cls1->get_truelist(), cls2->get_truelist());
        falselist = CodeBuffer::merge(cls1->get_falselist(), cls2->get_falselist());
        nextlist = CodeBuffer::merge(cls1->get_nextlist(), cls2->get_nextlist());
    }
    else if (op == EXP_TO_EXP_BINOP_ADD_EXP) {
        code = reg.get_name() + " = ";
        if (cls3->get_value() == "+") {
            code += "add ";
        }
        else {
            code += "sub ";
        }
        code += size_by_type(type) + " ";
        if (cls1->get_exp_case() == CONST_ID) {
            code += cls1->get_value();
        }
        else {
            code += cls1->get_reg();
        }
        code += ", ";
        if (cls2->get_exp_case() == CONST_ID) {
            code += cls2->get_value();
        }
        else {
            code += cls2->get_reg();
        }
        code_buffer.emit(code);
        truelist = CodeBuffer::merge(cls1->get_truelist(), cls2->get_truelist());
        falselist = CodeBuffer::merge(cls1->get_falselist(), cls2->get_falselist());
        nextlist = CodeBuffer::merge(cls1->get_nextlist(), cls2->get_nextlist());
    }
    else if (op == EXP_TO_ID) {
        std::string id_value = const_table.get_value(cls1->get_name());
        if (!id_value.empty()) {
            this->exp_case = CONST_ID;
            this->value = id_value;
        }
        else {
            Register temp_reg;
            std::string id_offset = std::to_string((symbol_table_stack.get_entry_by_name(cls1->get_name()))->get_offset());
            code = temp_reg.get_name() + " = add i32 " + id_offset + ", " + local_vars_reg.get_name();
            code_buffer.emit(code);
            code = reg.get_name() + " = load i32, i32* " + temp_reg.get_name();
            code_buffer.emit(code);
        }
    }
    else if (op == EXP_TO_NUM) {
        code = reg.get_name() + " = add i32 " + value + ", 0";
        code_buffer.emit(code);
    }
    else if (op == EXP_TO_NUM_B) {
        Register temp_reg;
        code = temp_reg.get_name() + " = zext i8 " + value + " to i32";
        code_buffer.emit(code);
        code = reg.get_name() + " = add i32 " + temp_reg.get_name() + ", 0";
        code_buffer.emit(code);
    }
    else if (op == EXP_TO_STRING) {
        global_code = "@." + cls1->get_str_gen_name() + " = internal constant [" + cls1->get_size() + " x i8] c\"" + cls1->get_value() + "\\00\"";
        code = reg.get_name() + " = getelementptr ([" + cls1->get_size() + " x i8], [" + cls1->get_size() + " x i8]* @." + cls1->get_str_gen_name() + ", i32 0, i32 0)";
        code_buffer.emitGlobal(global_code);
        code_buffer.emit(code);
    }
    else if (op == EXP_TO_TRUE) {
        pair<int, BranchLabelIndex> list_item;
        code = reg.get_name() + " = add i32 1, 0";
        code_buffer.emit(code);
        int emit_result = code_buffer.emit("br label @");
        list_item.first = emit_result;
        list_item.second = FIRST;
        truelist = CodeBuffer::makelist(list_item);
    }
    else if (op == EXP_TO_FALSE) {
        pair<int, BranchLabelIndex> list_item;
        code = reg.get_name() + " = add i32 0, 0";
        code_buffer.emit(code);
        int emit_result = code_buffer.emit("br label @");
        list_item.first = emit_result;
        list_item.second = FIRST;
        falselist = CodeBuffer::makelist(list_item);
    }
    else if (op == EXP_TO_NOT_EXP) {
        truelist = cls1->get_falselist();
        falselist = cls1->get_truelist();
    }
    else if (op == EXP_TO_EXP_AND_EXP) {
        code_buffer.bpatch(cls1->get_truelist(), cls3->get_label());
        falselist = CodeBuffer::merge(cls1->get_falselist(), cls2->get_falselist());
        truelist = cls2->get_truelist();
    }
    else if (op == EXP_TO_EXP_OR_EXP) {
        code_buffer.bpatch(cls1->get_falselist(), cls3->get_label());
        truelist = CodeBuffer::merge(cls1->get_truelist(), cls2->get_truelist());
        falselist = cls2->get_falselist();
    }
    else if (op == EXP_TO_EXP_RELOP_COMPARE_EXP) {
        code = reg.get_name() + " = icmp ";
        if (cls3->get_value() == "<") {
            code += "slt ";
        }
        else if (cls3->get_value() == "<=") {
            code += "sle ";
        }
        else if (cls3->get_value() == ">") {
            code += "sgt ";
        }
        else { // (cls3->get_value() == ">=")
            code += "sge ";
        }
        code += size_by_type(handle_binop_exp(cls1->get_type(),cls2->get_type())) + " ";
        if (cls1->get_exp_case() == CONST_ID) {
            code += cls1->get_value();
        }
        else {
            code += cls1->get_reg();
        }
        code += ", ";
        if (cls2->get_exp_case() == CONST_ID) {
            code += cls2->get_value();
        }
        else {
            code += cls2->get_reg();
        }
        code_buffer.emit(code);
        code = "br i1 " + reg.get_name() + ", label @, label @";
        int emit_result = code_buffer.emit(code);
        pair<int, BranchLabelIndex> list_item;
        list_item.first = emit_result;
        list_item.second = FIRST;
        truelist = CodeBuffer::makelist(list_item);
        list_item.second = SECOND;
        falselist = CodeBuffer::makelist(list_item);
    }
    else if (op == EXP_TO_EXP_RELOP_EQUAL_EXP) {
        code = reg.get_name() + " = icmp ";
        if (cls3->get_value() == "==") {
            code += "eq ";
        }
        else { // (cls3->get_value() == "!=")
            code += "ne ";
        }
        code += size_by_type(handle_binop_exp(cls1->get_type(),cls2->get_type())) + " ";
        if (cls1->get_exp_case() == CONST_ID) {
            code += cls1->get_value();
        }
        else {
            code += cls1->get_reg();
        }
        code += ", ";
        if (cls2->get_exp_case() == CONST_ID) {
            code += cls2->get_value();
        }
        else {
            code += cls2->get_reg();
        }
        code_buffer.emit(code);
        code = "br i1 " + reg.get_name() + ", label @, label @";
        int emit_result = code_buffer.emit(code);
        pair<int, BranchLabelIndex> list_item;
        list_item.first = emit_result;
        list_item.second = FIRST;
        truelist = CodeBuffer::makelist(list_item);
        list_item.second = SECOND;
        falselist = CodeBuffer::makelist(list_item);
    }
    else if (op == EXP_TO_CAST) {
        if (cls1->get_exp_case() == CONST_ID) {
            code = reg.get_name() + " = add " + size_by_type(type) + " " + cls1->get_value() + ", 0";
        }
        else {
            code = reg.get_name() + " = add " + size_by_type(type) + " " + cls1->get_reg() + ", 0";
        }
        code_buffer.emit(code);
    }
    else {
        std::cerr << "EXP OPERATION_TYPE ERROR!" << std::endl;
    }
}


CallCls::CallCls(std::string type) : type(type) {}


ExpListCls::ExpListCls(std::vector<std::string> args_types) : args_types(args_types) {}


void ExpListCls::add_new_func_arg(AbsCls* exp_new_type) {
  args_types.push_back(exp_new_type->get_type());
}


NumCls::NumCls(std::string value) : value(value) {}


BinopMulCls::BinopMulCls(std::string value) : value(value) {}


BinopAddCls::BinopAddCls(std::string value) : value(value) {}


StringCls::StringCls(std::string value) : str_gen(StringGen()), value(value), size(std::to_string(value.size())) {
    this->value = this->value.substr(1, this->value.size() - 2);
    this->size = std::to_string(this->value.size());
}


RelopCompareCls::RelopCompareCls(std::string value) : value(value) {}


RelopEqualCls::RelopEqualCls(std::string value) : value(value) {}


MCls::MCls() : label(code_buffer.genLabel()) {}


StatementCls::StatementCls(OPERATION_TYPE op, AbsCls* cls1, AbsCls* cls2,  AbsCls* cls3) : nextlist(std::vector<pair<int,BranchLabelIndex>>()){
    std::string code;
    std::string global_code;
    Register reg;
    if (op == STATEMENT_TO_TYPE_ID) {
        const_table.remove(cls1->get_name());
        code = reg.get_name() + " = alloca i32";
        code_buffer.emit(code);
        code = "store i32 0, i32* " + reg.get_name();
        code_buffer.emit(code);
    }
    else if (op == STATEMENT_TO_TYPE_ID_EXP) { //TODO - deal with case CONST
        if (cls1->get_is_const() && cls3->get_exp_case() == SIMPLE_NUM) {
            const_table.update(cls2->get_name(), cls3->get_value());
        }
        else {
            const_table.remove(cls2->get_name());
            code = reg.get_name() + " = alloca i32";
            code_buffer.emit(code);
            if (cls3->get_exp_case() == CONST_ID) {
                code = "store i32 " + cls3->get_value() + ", i32* " + reg.get_name();
            }
            else {
                code = "store i32 " + cls3->get_reg() + ", i32* " + reg.get_name();
            }
            code_buffer.emit(code);
        }
    }
    else if (op == STATEMETN_TO_IF) {
        code_buffer.bpatch(cls1->get_truelist(), cls3->get_label());
        nextlist = CodeBuffer::merge(cls1->get_falselist(), cls2->get_nextlist());
    }
    else {
        std::cerr << "STATEMENT OPERATION_TYPE ERROR!" << std::endl;
    }
}
