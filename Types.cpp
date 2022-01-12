#include <string>
#include "Types.h"
#include "Utilities.h"


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


ExpCls::ExpCls(std::string type, std::string value, OPERATION_TYPE op, AbsCls* cls1, AbsCls* cls2) : type(type),
                                                                                                     value(value),
                                                                                                     reg(Register()),
                                                                                                     truelist(std::vector<pair<int,BranchLabelIndex>>()),
                                                                                                     falselist(std::vector<pair<int,BranchLabelIndex>>()),
                                                                                                     nextlist(std::vector<pair<int,BranchLabelIndex>>()) {
    std::string code;
    switch (op) {
        case LPAREN_EXP_RPAREN:
            code = reg.get_name() + " = add " + size_by_type(type) + " " + cls1->get_reg() + ", 0";
            code_buffer.emit(code);
            truelist = cls1->get_truelist();
            falselist = cls1->get_falselist();
            nextlist = cls1->get_nextlist();
            break;
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
