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
               OPERATION_TYPE op,
               AbsCls* cls1,
               AbsCls* cls2,
               AbsCls* cls3) : type(type),
                               value(value),
                               reg(Register()),
                               truelist(std::vector<pair<int,BranchLabelIndex>>()),
                               falselist(std::vector<pair<int,BranchLabelIndex>>()),
                               nextlist(std::vector<pair<int,BranchLabelIndex>>()) {
    std::string code;
    std::string global_code;
    int emit_result;
    pair<int, BranchLabelIndex> list_item;
    switch (op) {
        case EXP_TO_LPAREN_EXP_RPAREN:
            code = reg.get_name() + " = add " + size_by_type(type) + " " + cls1->get_reg() + ", 0";
            code_buffer.emit(code);
            truelist = cls1->get_truelist();
            falselist = cls1->get_falselist();
            nextlist = cls1->get_nextlist();
            break;
        case EXP_TO_EXP_BINOP_MUL_EXP:
            if (cls3->get_value() == "*") {
                code = reg.get_name() + " = mul " + size_by_type(type) + " " + cls1->get_reg() + ", " + cls2->get_reg();
            }
            else {
                code = reg.get_name() + " = sdiv " + size_by_type(type) + " " + cls1->get_reg() + ", " + cls2->get_reg();
            }
            code_buffer.emit(code);
            truelist = CodeBuffer::merge(cls1->get_truelist(), cls2->get_truelist());
            falselist = CodeBuffer::merge(cls1->get_falselist(), cls2->get_falselist());
            nextlist = CodeBuffer::merge(cls1->get_nextlist(), cls2->get_nextlist());
            break;
        case EXP_TO_EXP_BINOP_ADD_EXP:
            if (cls3->get_value() == "+") {
                code = reg.get_name() + " = add " + size_by_type(type) + " " + cls1->get_reg() + ", " + cls2->get_reg();
            }
            else {
                code = reg.get_name() + " = sub " + size_by_type(type) + " " + cls1->get_reg() + ", " + cls2->get_reg();
            }
            code_buffer.emit(code);
            truelist = CodeBuffer::merge(cls1->get_truelist(), cls2->get_truelist());
            falselist = CodeBuffer::merge(cls1->get_falselist(), cls2->get_falselist());
            nextlist = CodeBuffer::merge(cls1->get_nextlist(), cls2->get_nextlist());
            break;
        case EXP_TO_NUM:
            code = reg.get_name() + " = add " + size_by_type(type) + " " + value + ", 0";
            code_buffer.emit(code);
            break;
        case EXP_TO_NUM_B:
            code = reg.get_name() + " = add " + size_by_type(type) + " " + value + ", 0";
            code_buffer.emit(code);
            break;
        case EXP_TO_STRING:
            global_code = "@." + cls1->get_str_gen_name() + " = internal constant [" + cls1->get_size() + " x i8] c\"" + cls1->get_value() + "\\00\"";
            code = reg.get_name() + " = getelementptr ([" + cls1->get_size() + " x i8], [" + cls1->get_size() + " x i8]* @." + cls1->get_str_gen_name() + ", i32 0, i32 0)";
            code_buffer.emitGlobal(global_code);
            code_buffer.emit(code);
            break;
        case EXP_TO_TRUE:
            code = reg.get_name() + " = add " + size_by_type(type) + " " + "1" + ", 0";
            code_buffer.emit(code);
            emit_result = code_buffer.emit("br labal @");
            list_item.first = emit_result;
            list_item.second = FIRST;
            truelist = CodeBuffer::makelist(list_item);
            break;
        case EXP_TO_FALSE:
            code = reg.get_name() + " = add " + size_by_type(type) + " " + "0" + ", 0";
            code_buffer.emit(code);
            emit_result = code_buffer.emit("br labal @");
            list_item.first = emit_result;
            list_item.second = FIRST;
            falselist = CodeBuffer::makelist(list_item);
            break;
        case EXP_TO_NOT_EXP:
            truelist = cls1->get_falselist();
            falselist = cls1->get_truelist();
            break;
        case EXP_TO_EXP_AND_EXP:
            code_buffer.bpatch(cls1->get_truelist(), cls3->get_label());
            falselist = CodeBuffer::merge(cls1->get_falselist(), cls2->get_falselist());
            truelist = cls2->get_truelist();
            break;
        case EXP_TO_EXP_OR_EXP:
            code_buffer.bpatch(cls1->get_falselist(), cls3->get_label());
            truelist = CodeBuffer::merge(cls1->get_truelist(), cls2->get_truelist());
            falselist = cls2->get_falselist();
            break;
        case EXP_TO_EXP_RELOP_COMPARE_EXP:
            if (cls3->get_value() == "<") {
                code = reg.get_name() + " = icmp "
            }
            else if (cls3->get_value() == "<=") {

            }
            else if (cls3->get_value() == ">") {

            }
            else {  // (cls3->get_value() == ">=")

            }
            code_buffer.emit(code);
        default:
            std::cerr << "OPERATION_TYPE ERROR!" << std::endl;
            break;
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
