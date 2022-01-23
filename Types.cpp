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
        if (cls1->get_type().find("BOOL") == std::string::npos) {
            if (cls1->get_exp_case() == CONST_ID) {
                code = DOUBLE_TAB + reg.get_name() + " = add " + size_by_type(type) + " " + cls1->get_value() + ", 0";
            }
            else {
                code = DOUBLE_TAB + reg.get_name() + " = add " + size_by_type(type) + " " + cls1->get_reg() + ", 0";
            }
            code_buffer.emit(code);
        }
        truelist = cls1->get_truelist();
        falselist = cls1->get_falselist();
        nextlist = cls1->get_nextlist();
    }
    else if (op == EXP_TO_EXP_BINOP_MUL_EXP) {
        // the operands will be the actual strings in the mul/div command
        std::string operand_1 = cls1->get_exp_case() == CONST_ID ? cls1->get_value() : cls1->get_reg();
        std::string operand_2 = cls2->get_exp_case() == CONST_ID ? cls2->get_value() : cls2->get_reg();
        std::string operand_2_size = size_by_type(cls2->get_type());
        // check if we need to extent one of the registers
        if (size_by_type(cls1->get_type()) != size_by_type(cls2->get_type())) {
            if (size_by_type(cls1->get_type()) == "i8" && cls1->get_exp_case() != CONST_ID) {
                Register ext_reg;
                code_buffer.emit(DOUBLE_TAB + ext_reg.get_name() + " = zext i8 " + operand_1 + " to i32");
                operand_1 = ext_reg.get_name();
            }
            else if (size_by_type(cls2->get_type()) == "i8" && cls2->get_exp_case() != CONST_ID) {
                Register ext_reg;
                code_buffer.emit(DOUBLE_TAB + ext_reg.get_name() + " = zext i8 " + operand_2 + " to i32");
                operand_2 = ext_reg.get_name();
                operand_2_size = "i32";
            }
        }
        // prepare the mul/div commands
        if (cls3->get_value() == "*") {
            code = DOUBLE_TAB + reg.get_name() + " = mul ";
        }
        else { // cls3->get_value() == "/"
            // checking division by 0.
            Register temp_reg;
            code_buffer.emit(DOUBLE_TAB + temp_reg.get_name() + " = icmp eq " + operand_2_size + " " + operand_2 + ", 0");
            int br_addr = code_buffer.emit(DOUBLE_TAB + "br i1 " + temp_reg.get_name() + ", label @, label @");
            std::string true_label = code_buffer.genLabel();
            Register div_error_reg;
            code_buffer.emit(DOUBLE_TAB + div_error_reg.get_name() + " = getelementptr [23 x i8], [23 x i8]* @.div_error, i32 0, i32 0");
            code_buffer.emit(DOUBLE_TAB + "call void @print(i8* " + div_error_reg.get_name() + ")"); //TODO: should we insert lables after these calls?
            code_buffer.emit(DOUBLE_TAB + "call void @exit(i32 0)"); //TODO: which return value should we send to exit.
            int div_err_br_addr = code_buffer.emit(DOUBLE_TAB + "br label @");
            std::string false_label = code_buffer.genLabel();
            pair<int,BranchLabelIndex> new_pair;
            new_pair.first = br_addr;
            new_pair.second = FIRST;
            code_buffer.bpatch(CodeBuffer::makelist(new_pair), true_label);
            new_pair.second = SECOND;
            code_buffer.bpatch(CodeBuffer::makelist(new_pair), false_label);
            //bpatching the branch after call exit
            new_pair.first = div_err_br_addr;
            new_pair.second = FIRST;
            code_buffer.bpatch(CodeBuffer::makelist(new_pair), false_label);
            //the actual division.
            code = DOUBLE_TAB + reg.get_name() + " = ";
            if (type.find("BYTE") != std::string::npos) {
                code += "udiv ";
            }
            else {
                code += "sdiv ";
            }
        }
        code += size_by_type(type) + " " + operand_1 + ", " + operand_2;
        code_buffer.emit(code);
        truelist = CodeBuffer::merge(cls1->get_truelist(), cls2->get_truelist());
        falselist = CodeBuffer::merge(cls1->get_falselist(), cls2->get_falselist());
        nextlist = CodeBuffer::merge(cls1->get_nextlist(), cls2->get_nextlist());
    }
    else if (op == EXP_TO_EXP_BINOP_ADD_EXP) {
        // the operands will be the actual strings in the add/sub command
        std::string operand_1 = cls1->get_exp_case() == CONST_ID ? cls1->get_value() : cls1->get_reg();
        std::string operand_2 = cls2->get_exp_case() == CONST_ID ? cls2->get_value() : cls2->get_reg();
        // check if we need to extent one of the registers
        if (size_by_type(cls1->get_type()) != size_by_type(cls2->get_type())) {
            if (size_by_type(cls1->get_type()) == "i8" && cls1->get_exp_case() != CONST_ID) {
                Register ext_reg;
                code_buffer.emit(DOUBLE_TAB + ext_reg.get_name() + " = zext i8 " + operand_1 + " to i32");
                operand_1 = ext_reg.get_name();
            }
            else if (size_by_type(cls2->get_type()) == "i8" && cls2->get_exp_case() != CONST_ID) {
                Register ext_reg;
                code_buffer.emit(DOUBLE_TAB + ext_reg.get_name() + " = zext i8 " + operand_2 + " to i32");
                operand_2 = ext_reg.get_name();
            }
        }
        // prepare the add/sub commands
        if (cls3->get_value() == "+") {
            code = DOUBLE_TAB + reg.get_name() + " = add ";
        }
        else { // cls3->get_value() == "-"
            code = DOUBLE_TAB + reg.get_name() + " = sub ";
        }
        code += size_by_type(type) + " " + operand_1 + ", " + operand_2;
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
            if (type.find("BOOL") != std::string::npos) {
                Register temp_reg;
                code_buffer.emit(DOUBLE_TAB + temp_reg.get_name() + " = icmp eq i1 " + id_value + ", 1");
                int emit_result = code_buffer.emit(DOUBLE_TAB + "br i1 " + temp_reg.get_name() + ", label @, label @");
                pair<int, BranchLabelIndex> list_item;
                list_item.first = emit_result;
                list_item.second = FIRST;
                truelist = CodeBuffer::makelist(list_item);
                list_item.second = SECOND;
                falselist = CodeBuffer::makelist(list_item);
            }
        }
        else {
            Register stack_value_reg;
            Register addr_calc_reg;
            int id_offset = (symbol_table_stack.get_entry_by_name(cls1->get_name()))->get_offset();
            if (id_offset >= 0) { // local val case
                std::string id_offset_str = std::to_string(id_offset);
                code_buffer.emit(DOUBLE_TAB + addr_calc_reg.get_name() + " = getelementptr [50 x i32], [50 x i32]* " + local_vars_reg.get_name() + ", i32 0, i32 " + id_offset_str);
                if (size_by_type(type) != "i32") {
                    Register ext_reg;
                    code_buffer.emit(DOUBLE_TAB + ext_reg.get_name() + " = load i32, i32* " + addr_calc_reg.get_name());
                    code_buffer.emit(DOUBLE_TAB + stack_value_reg.get_name() + " = trunc i32 " + ext_reg.get_name() + " to " + size_by_type(type));
                }
                else {
                    code_buffer.emit(DOUBLE_TAB + stack_value_reg.get_name() + " = load i32, i32* " + addr_calc_reg.get_name());
                }
            }
            else { // argument case
                std::string arg_index = std::to_string((-1 * id_offset) - 1);
                code_buffer.emit(DOUBLE_TAB + stack_value_reg.get_name() + " = add " + size_by_type(type) + " %" + arg_index + ", 0");
            }
            if (type.find("BOOL") != std::string::npos) {
                Register temp_reg;
                code_buffer.emit(DOUBLE_TAB + temp_reg.get_name() + " = icmp eq i1 " + stack_value_reg.get_name() + ", 1");
                int emit_result = code_buffer.emit(DOUBLE_TAB + "br i1 " + temp_reg.get_name() + ", label @, label @");
                pair<int, BranchLabelIndex> list_item;
                list_item.first = emit_result;
                list_item.second = FIRST;
                truelist = CodeBuffer::makelist(list_item);
                list_item.second = SECOND;
                falselist = CodeBuffer::makelist(list_item);
            }
            else {
                code_buffer.emit(DOUBLE_TAB + reg.get_name() + " = add " + size_by_type(type) + " " + stack_value_reg.get_name() + ", 0");
            }
        }
    }
    else if (op == EXP_TO_CALL) {
        if (type.find("BOOL") == std::string::npos) {
            code_buffer.emit(DOUBLE_TAB + reg.get_name() + " = add " + size_by_type(cls1->get_type()) + " " + cls1->get_reg() + ", 0");
        }
        else {
            Register temp_reg;
            code_buffer.emit(DOUBLE_TAB + temp_reg.get_name() + " = icmp eq i1 " + cls1->get_reg() + ", 1");
            int emit_result = code_buffer.emit(DOUBLE_TAB + "br i1 " + temp_reg.get_name() + ", label @, label @");
            pair<int, BranchLabelIndex> list_item;
            list_item.first = emit_result;
            list_item.second = FIRST;
            truelist = CodeBuffer::makelist(list_item);
            list_item.second = SECOND;
            falselist = CodeBuffer::makelist(list_item);
        }
    }
    else if (op == EXP_TO_NUM) {
        code_buffer.emit(DOUBLE_TAB + reg.get_name() + " = add i32 " + value + ", 0");
    }
    else if (op == EXP_TO_NUM_B) {
        code_buffer.emit(DOUBLE_TAB + reg.get_name() + " = add i8 " + value + ", 0");
    }
    else if (op == EXP_TO_STRING) {
        global_code = "@." + cls1->get_str_gen_name() + " = internal constant [" + cls1->get_size() + " x i8] c\"" + cls1->get_value() + "\\00\"";
        code = DOUBLE_TAB + reg.get_name() + " = getelementptr [" + cls1->get_size() + " x i8], [" + cls1->get_size() + " x i8]* @." + cls1->get_str_gen_name() + ", i32 0, i32 0";
        code_buffer.emitGlobal(global_code);
        code_buffer.emit(code);
    }
    else if (op == EXP_TO_TRUE) {
        pair<int, BranchLabelIndex> list_item;
        int emit_result = code_buffer.emit(DOUBLE_TAB + "br label @");
        list_item.first = emit_result;
        list_item.second = FIRST;
        truelist = CodeBuffer::makelist(list_item);
    }
    else if (op == EXP_TO_FALSE) {
        pair<int, BranchLabelIndex> list_item;
        int emit_result = code_buffer.emit(DOUBLE_TAB + "br label @");
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
        // the operands will be the actual strings in the compare command
        std::string operand_1 = cls1->get_exp_case() == CONST_ID ? cls1->get_value() : cls1->get_reg();
        std::string operand_2 = cls2->get_exp_case() == CONST_ID ? cls2->get_value() : cls2->get_reg();
        // check if we need to extent one of the registers
        if (size_by_type(cls1->get_type()) != size_by_type(cls2->get_type())) {
            if (size_by_type(cls1->get_type()) == "i8" && cls1->get_exp_case() != CONST_ID) {
                Register ext_reg;
                code_buffer.emit(DOUBLE_TAB + ext_reg.get_name() + " = zext i8 " + operand_1 + " to i32");
                operand_1 = ext_reg.get_name();
            }
            else if (size_by_type(cls2->get_type()) == "i8" && cls2->get_exp_case() != CONST_ID) {
                Register ext_reg;
                code_buffer.emit(DOUBLE_TAB + ext_reg.get_name() + " = zext i8 " + operand_2 + " to i32");
                operand_2 = ext_reg.get_name();
            }
        }
        Register cmp_result_reg;
        code = DOUBLE_TAB + cmp_result_reg.get_name() + " = icmp ";
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
        code += size_by_type(handle_binop_exp(cls1->get_type(), cls2->get_type())) + " " + operand_1 + ", " + operand_2;
        code_buffer.emit(code);
        int emit_result = code_buffer.emit(DOUBLE_TAB + "br i1 " + cmp_result_reg.get_name() + ", label @, label @");
        pair<int, BranchLabelIndex> list_item;
        list_item.first = emit_result;
        list_item.second = FIRST;
        truelist = CodeBuffer::makelist(list_item);
        list_item.second = SECOND;
        falselist = CodeBuffer::makelist(list_item);
    }
    else if (op == EXP_TO_EXP_RELOP_EQUAL_EXP) {
        // the operands will be the actual strings in the compare command
        std::string operand_1 = cls1->get_exp_case() == CONST_ID ? cls1->get_value() : cls1->get_reg();
        std::string operand_2 = cls2->get_exp_case() == CONST_ID ? cls2->get_value() : cls2->get_reg();
        // check if we need to extent one of the registers
        if (size_by_type(cls1->get_type()) != size_by_type(cls2->get_type())) {
            if (size_by_type(cls1->get_type()) == "i8" && cls1->get_exp_case() != CONST_ID) {
                Register ext_reg;
                code_buffer.emit(DOUBLE_TAB + ext_reg.get_name() + " = zext i8 " + operand_1 + " to i32");
                operand_1 = ext_reg.get_name();
            }
            else if (size_by_type(cls2->get_type()) == "i8" && cls2->get_exp_case() != CONST_ID) {
                Register ext_reg;
                code_buffer.emit(DOUBLE_TAB + ext_reg.get_name() + " = zext i8 " + operand_2 + " to i32");
                operand_2 = ext_reg.get_name();
            }
        }
        Register cmp_result_reg;
        code = DOUBLE_TAB + cmp_result_reg.get_name() + " = icmp ";
        if (cls3->get_value() == "==") {
            code += "eq ";
        }
        else { // (cls3->get_value() == "!=")
            code += "ne ";
        }
        code += size_by_type(handle_binop_exp(cls1->get_type(), cls2->get_type())) + " " + operand_1 + ", " + operand_2;
        code_buffer.emit(code);
        int emit_result = code_buffer.emit(DOUBLE_TAB + "br i1 " + cmp_result_reg.get_name() + ", label @, label @");
        pair<int, BranchLabelIndex> list_item;
        list_item.first = emit_result;
        list_item.second = FIRST;
        truelist = CodeBuffer::makelist(list_item);
        list_item.second = SECOND;
        falselist = CodeBuffer::makelist(list_item);
    }
    else if (op == EXP_TO_CAST) {
        if (size_by_type(type) == size_by_type(cls1->get_type())) {
            std::string src_operand = cls1->get_exp_case() == CONST_ID ? cls1->get_value() : cls1->get_reg();
            code_buffer.emit(DOUBLE_TAB + reg.get_name() + " = add " + size_by_type(type) + " " + src_operand + ", 0");
        }
        else if (size_by_type(type) == "i8" && size_by_type(cls1->get_type()) == "i32") {
            std::string src_operand = cls1->get_exp_case() == CONST_ID ? cls1->get_value() : cls1->get_reg();
            code_buffer.emit(DOUBLE_TAB + reg.get_name() + " = trunc i32 " + src_operand + " to i8");
        }
         else if (size_by_type(type) == "i32" && size_by_type(cls1->get_type()) == "i8") {
             std::string src_operand = cls1->get_exp_case() == CONST_ID ? cls1->get_value() : cls1->get_reg();
             code_buffer.emit(DOUBLE_TAB + reg.get_name() + " = zext i8 " + src_operand + " to i32");
         }
    }
    else {
        std::cerr << "EXP OPERATION_TYPE ERROR!" << std::endl;
    }
}


CallCls::CallCls(std::string type, OPERATION_TYPE op, AbsCls* cls1, AbsCls* cls2) : type(type), reg(Register()) {
    if (op == CALL_TO_ID) {
        if (type == "VOID") {
            code_buffer.emit(DOUBLE_TAB + "call " + size_by_type(type) + " @" + cls1->get_name() + "()");
        }
        else {
            code_buffer.emit(DOUBLE_TAB + reg.get_name() + " = call " + size_by_type(type) + " @" + cls1->get_name() + "()");
        }
    }
    else if (op == CALL_TO_ID_EXPLIST) {
        std::vector<std::string> func_args_sizes_reverse = symbol_table_stack.get_entry_by_name(cls1->get_name())->get_arguments();
        std::vector<std::string> func_args_sizes;
        for (std::vector<std::string>::reverse_iterator it = func_args_sizes_reverse.rbegin(); it != func_args_sizes_reverse.rend(); ++it) {
    		func_args_sizes.push_back(*it);
        }
        std::string code;
        if (type == "VOID") {
            code = DOUBLE_TAB + "call " + size_by_type(type) + " @" + cls1->get_name() + "(";
        }
        else {
            code = DOUBLE_TAB + reg.get_name() + " = call " + size_by_type(type) + " @" + cls1->get_name() + "(";
        }
        size_t i = cls2->get_vals().size() - 1;
        while (true) {
            if (size_by_type(cls2->get_args_types()[i]) != size_by_type(func_args_sizes[i])) {
                Register ext_reg;
                code_buffer.emit(DOUBLE_TAB + ext_reg.get_name() + " = zext i8 " + cls2->get_vals()[i] + " to i32");
                code += size_by_type(func_args_sizes[i]) + " " + ext_reg.get_name() + ", ";
            }
            else {
                code += size_by_type(func_args_sizes[i]) + " " + cls2->get_vals()[i] + ", ";
            }
            if (i == 0) {
                break;
            }
            i--;
        }
        code_buffer.emit(code.substr(0, code.size() - 2) + ")");
    }
    else {
        std::cerr << "CALL OPERATION_TYPE ERROR!" << std::endl;
    }
}


ExpListCls::ExpListCls(std::vector<std::string> args_types,
                       std::vector<std::string> vals) : args_types(args_types),
                                                        vals(vals) {}


void ExpListCls::add_new_func_arg(AbsCls* exp) {
  args_types.push_back(exp->get_type());
  if (exp->get_exp_case() == CONST_ID) {
      vals.push_back(exp->get_value());
  }
  else {
      vals.push_back(exp->get_reg());
  }
}


NumCls::NumCls(std::string value) : value(value) {}


BinopMulCls::BinopMulCls(std::string value) : value(value) {}


BinopAddCls::BinopAddCls(std::string value) : value(value) {}


StringCls::StringCls(std::string value) : str_gen(StringGen()), value(value), size(std::to_string(value.size())) {
    this->value = this->value.substr(1, this->value.size() - 2);
    this->size = std::to_string(this->value.size() + 1); // the +1 is to include the '/00' at the end of the string.
}


RelopCompareCls::RelopCompareCls(std::string value) : value(value) {}


RelopEqualCls::RelopEqualCls(std::string value) : value(value) {}


MCls::MCls() : label(code_buffer.genLabel()) {}


NCls::NCls() : nextlist(std::vector<pair<int,BranchLabelIndex>>()) {
    pair<int,BranchLabelIndex> new_pair;
    new_pair.first = code_buffer.emit(DOUBLE_TAB + "br label @");
    new_pair.second = FIRST;
    nextlist = CodeBuffer::makelist(new_pair);
}


StatementCls::StatementCls(OPERATION_TYPE op, AbsCls* cls1, AbsCls* cls2,  AbsCls* cls3, AbsCls* cls4, AbsCls* cls5, AbsCls* cls6) :
                           nextlist(std::vector<pair<int,BranchLabelIndex>>()),
                           continue_list(std::vector<pair<int,BranchLabelIndex>>()),
                           break_list(std::vector<pair<int,BranchLabelIndex>>()) {
    std::string code;
    std::string global_code;
    Register reg1;
    Register reg2;
    if (op == STATEMENT_TO_STATEMENTS) {
        continue_list = cls1->get_continue_list();
        break_list = cls1->get_break_list();
    }
    else if (op == STATEMENT_TO_TYPE_ID) {
        const_table.remove(cls1->get_name());
        std::string id_offset = std::to_string((symbol_table_stack.get_entry_by_name(cls1->get_name()))->get_offset());
        code_buffer.emit(DOUBLE_TAB + reg1.get_name() + " = getelementptr [50 x i32], [50 x i32]* " + local_vars_reg.get_name() + ", i32 0, i32 " + id_offset);
        code_buffer.emit(DOUBLE_TAB + "store i32 0, i32* " + reg1.get_name());
    }
    else if (op == STATEMENT_TO_TYPE_ID_EXP) {
        if (cls1->get_is_const() && cls3->get_exp_case() == SIMPLE_NUM) {
            const_table.update(cls2->get_name(), cls3->get_value());
            if (cls3->get_type().find("BOOL") != std::string::npos) {
                std::string true_label = code_buffer.genLabel();
                code_buffer.bpatch(cls3->get_truelist(), true_label);
                int true_label_addr = code_buffer.emit(DOUBLE_TAB + "br label @");
                pair<int,BranchLabelIndex> true_label_pair;
                true_label_pair.first = true_label_addr;
                true_label_pair.second = FIRST;

                std::string false_label = code_buffer.genLabel();
                code_buffer.bpatch(cls3->get_falselist(), false_label);
                int false_label_addr = code_buffer.emit(DOUBLE_TAB + "br label @");
                pair<int,BranchLabelIndex> false_label_pair;
                false_label_pair.first = false_label_addr;
                false_label_pair.second = FIRST;

                vector<pair<int,BranchLabelIndex>> branches_to_patch;
                branches_to_patch.push_back(true_label_pair);
                branches_to_patch.push_back(false_label_pair);
                std::string final_label = code_buffer.genLabel();
                code_buffer.bpatch(branches_to_patch, final_label);

                code_buffer.emit(DOUBLE_TAB + reg1.get_name() + " = phi i32 [1, %" + true_label + "], [0, %" + false_label + "]");
                std::string id_offset = std::to_string((symbol_table_stack.get_entry_by_name(cls2->get_name()))->get_offset());
                code_buffer.emit(DOUBLE_TAB + reg2.get_name() + " = getelementptr [50 x i32], [50 x i32]* " + local_vars_reg.get_name() + ", i32 0, i32 " + id_offset);
                // code_buffer.emit(DOUBLE_TAB + "store i32 " + reg1.get_name() + ", i32* " + reg2.get_name());
            }
        }
        else {
            const_table.remove(cls2->get_name());
            if (cls3->get_type().find("BOOL") != std::string::npos) {
                std::string true_label = code_buffer.genLabel();
                code_buffer.bpatch(cls3->get_truelist(), true_label);
                int true_label_addr = code_buffer.emit(DOUBLE_TAB + "br label @");
                pair<int,BranchLabelIndex> true_label_pair;
                true_label_pair.first = true_label_addr;
                true_label_pair.second = FIRST;

                std::string false_label = code_buffer.genLabel();
                code_buffer.bpatch(cls3->get_falselist(), false_label);
                int false_label_addr = code_buffer.emit(DOUBLE_TAB + "br label @");
                pair<int,BranchLabelIndex> false_label_pair;
                false_label_pair.first = false_label_addr;
                false_label_pair.second = FIRST;

                vector<pair<int,BranchLabelIndex>> branches_to_patch;
                branches_to_patch.push_back(true_label_pair);
                branches_to_patch.push_back(false_label_pair);
                std::string final_label = code_buffer.genLabel();
                code_buffer.bpatch(branches_to_patch, final_label);

                code_buffer.emit(DOUBLE_TAB + reg1.get_name() + " = phi i32 [1, %" + true_label + "], [0, %" + false_label + "]");
                std::string id_offset = std::to_string((symbol_table_stack.get_entry_by_name(cls2->get_name()))->get_offset());
                code_buffer.emit(DOUBLE_TAB + reg2.get_name() + " = getelementptr [50 x i32], [50 x i32]* " + local_vars_reg.get_name() + ", i32 0, i32 " + id_offset);
                code_buffer.emit(DOUBLE_TAB + "store i32 " + reg1.get_name() + ", i32* " + reg2.get_name());
            }
            else {
                std::string id_offset = std::to_string((symbol_table_stack.get_entry_by_name(cls2->get_name()))->get_offset());
                code_buffer.emit(DOUBLE_TAB + reg1.get_name() + " = getelementptr [50 x i32], [50 x i32]* " + local_vars_reg.get_name() + ", i32 0, i32 " + id_offset);
                std::string operand_val = cls3->get_exp_case() == CONST_ID ? cls3->get_value() : cls3->get_reg();
                // check if we need to extent the register
                if (size_by_type(cls3->get_type()) != "i32" && cls3->get_exp_case() != CONST_ID) {
                    Register ext_reg;
                    code_buffer.emit(DOUBLE_TAB + ext_reg.get_name() + " = zext " + size_by_type(cls3->get_type()) + " " + operand_val + " to i32");
                    operand_val = ext_reg.get_name();
                }
                code_buffer.emit(DOUBLE_TAB + "store i32 " + operand_val + ", i32* " + reg1.get_name());
            }
        }
    }
    else if (op == STATEMENT_TO_ID_EXP) {
        if (cls2->get_type().find("BOOL") != std::string::npos) {
            std::string true_label = code_buffer.genLabel();
            code_buffer.bpatch(cls2->get_truelist(), true_label);
            code = DOUBLE_TAB + "br label @";
            int true_label_addr = code_buffer.emit(code);
            pair<int,BranchLabelIndex> true_label_pair;
            true_label_pair.first = true_label_addr;
            true_label_pair.second = FIRST;

            std::string false_label = code_buffer.genLabel();
            code_buffer.bpatch(cls2->get_falselist(), false_label);
            code = DOUBLE_TAB + "br label @";
            int false_label_addr = code_buffer.emit(code);
            pair<int,BranchLabelIndex> false_label_pair;
            false_label_pair.first = false_label_addr;
            false_label_pair.second = FIRST;

            vector<pair<int,BranchLabelIndex>> branches_to_patch;
            branches_to_patch.push_back(true_label_pair);
            branches_to_patch.push_back(false_label_pair);
            std::string final_label = code_buffer.genLabel();
            code_buffer.bpatch(branches_to_patch, final_label);

            code_buffer.emit(DOUBLE_TAB + reg1.get_name() + " = phi i32 [1, %" + true_label + "], [0, %" + false_label + "]");
            std::string id_offset = std::to_string((symbol_table_stack.get_entry_by_name(cls1->get_name()))->get_offset());
            code_buffer.emit(DOUBLE_TAB + reg2.get_name() + " = getelementptr [50 x i32], [50 x i32]* " + local_vars_reg.get_name() + ", i32 0, i32 " + id_offset);
            code_buffer.emit(DOUBLE_TAB + "store i32 " + reg1.get_name() + ", i32* " + reg2.get_name());
        }
        else {
            std::string id_offset = std::to_string((symbol_table_stack.get_entry_by_name(cls1->get_name()))->get_offset());
            code_buffer.emit(DOUBLE_TAB + reg1.get_name() + " = getelementptr [50 x i32], [50 x i32]* " + local_vars_reg.get_name() + ", i32 0, i32 " + id_offset);

            std::string operand_val = cls2->get_exp_case() == CONST_ID ? cls2->get_value() : cls2->get_reg();
            // check if we need to extent the register
            if (size_by_type(cls2->get_type()) != "i32" && cls2->get_exp_case() != CONST_ID) {
                Register ext_reg;
                code_buffer.emit(DOUBLE_TAB + ext_reg.get_name() + " = zext " + size_by_type(cls2->get_type()) + " " + operand_val + " to i32");
                operand_val = ext_reg.get_name();
            }
            code_buffer.emit(DOUBLE_TAB + "store i32 " + operand_val + ", i32* " + reg1.get_name());
        }
    }
    else if (op == STATEMETN_TO_IF) {
        if (cls4->get_is_empty()) {
            code_buffer.bpatch(cls1->get_truelist(), cls2->get_label());
            nextlist = CodeBuffer::merge(cls1->get_falselist(), cls3->get_nextlist());
            // inserting branch and then label
            // back patching the nextlist with this label
            int last_branch_addr = code_buffer.emit(DOUBLE_TAB + "br label @");
            pair<int,BranchLabelIndex> last_branch_fix;
            last_branch_fix.first = last_branch_addr;
            last_branch_fix.second = FIRST;
            nextlist = CodeBuffer::merge(nextlist, CodeBuffer::makelist(last_branch_fix));
            std::string final_label = code_buffer.genLabel();
            code_buffer.bpatch(nextlist, final_label);
            continue_list = cls3->get_continue_list();
            break_list = cls3->get_break_list();
        }
        else {
            code_buffer.bpatch(cls1->get_truelist(), cls2->get_label());
            code_buffer.bpatch(cls1->get_falselist(), cls4->get_label());
            nextlist = CodeBuffer::merge(cls3->get_nextlist(), cls4->get_nextlist());
            // inserting branch and then label
            // back patching the nextlist with this label
            int last_branch_addr = code_buffer.emit(DOUBLE_TAB + "br label @");
            pair<int,BranchLabelIndex> last_branch_fix;
            last_branch_fix.first = last_branch_addr;
            last_branch_fix.second = FIRST;
            nextlist = CodeBuffer::merge(nextlist, CodeBuffer::makelist(last_branch_fix));
            std::string final_label = code_buffer.genLabel();
            code_buffer.bpatch(nextlist, final_label);
            continue_list = CodeBuffer::merge(cls3->get_continue_list(), cls4->get_continue_list());
            break_list = CodeBuffer::merge(cls3->get_break_list(), cls4->get_break_list());
        }
    }
    else if (op == STATEMENT_TO_WHILE) {
        code_buffer.bpatch(cls1->get_nextlist(), cls2->get_label());
        code_buffer.bpatch(cls3->get_truelist(), cls4->get_label());
        nextlist = cls3->get_falselist();
        code_buffer.emit(DOUBLE_TAB + "br label %" + cls2->get_label());
        std::string final_label = code_buffer.genLabel();
        code_buffer.bpatch(nextlist, final_label);
        code_buffer.bpatch(cls5->get_continue_list(), cls2->get_label());
        code_buffer.bpatch(cls5->get_break_list(), final_label);
    }
    else if (op == STATEMENT_TO_CONTINUE) {
        int br_addr = code_buffer.emit(DOUBLE_TAB + "br label @");
        code_buffer.genLabel();
        pair<int,BranchLabelIndex> continue_branch;
        continue_branch.first = br_addr;
        continue_branch.second = FIRST;
        continue_list = CodeBuffer::makelist(continue_branch);
    }
    else if (op == STATEMENT_TO_BREAK) {
        int br_addr = code_buffer.emit(DOUBLE_TAB + "br label @");
        code_buffer.genLabel();
        pair<int,BranchLabelIndex> break_branch;
        break_branch.first = br_addr;
        break_branch.second = FIRST;
        break_list = CodeBuffer::makelist(break_branch);
    }
    else if (op == STATEMENT_TO_RET) {
        // Do nothing
    }
    else if (op == STATEMENT_TO_RET_EXP) {
        if (cls1->get_type().find("BOOL") != std::string::npos) {
            std::string true_label = code_buffer.genLabel();
            code_buffer.bpatch(cls1->get_truelist(), true_label);
            int true_label_addr = code_buffer.emit(DOUBLE_TAB + "br label @");
            pair<int,BranchLabelIndex> true_label_pair;
            true_label_pair.first = true_label_addr;
            true_label_pair.second = FIRST;

            std::string false_label = code_buffer.genLabel();
            code_buffer.bpatch(cls1->get_falselist(), false_label);
            int false_label_addr = code_buffer.emit(DOUBLE_TAB + "br label @");
            pair<int,BranchLabelIndex> false_label_pair;
            false_label_pair.first = false_label_addr;
            false_label_pair.second = FIRST;

            vector<pair<int,BranchLabelIndex>> branches_to_patch;
            branches_to_patch.push_back(true_label_pair);
            branches_to_patch.push_back(false_label_pair);
            std::string final_label = code_buffer.genLabel();
            code_buffer.bpatch(branches_to_patch, final_label);

            code_buffer.emit(DOUBLE_TAB + cls1->get_reg() + " = phi i1 [1, %" + true_label + "], [0, %" + false_label + "]");
        }
        emit_explicit_return(cls1);
    }
    else if (op == STATEMENT_TO_CALL) {
        // Do nothing
    }
    else {
        std::cerr << "STATEMENT OPERATION_TYPE ERROR!" << std::endl;
    }
}


StatementsCls::StatementsCls(OPERATION_TYPE op,
                             AbsCls* cls1,
                             AbsCls* cls2,
                             AbsCls* cls3) : continue_list(std::vector<pair<int,BranchLabelIndex>>()),  break_list(std::vector<pair<int,BranchLabelIndex>>()) {
    if (op == STATEMENTS_TO_STATEMENT) {
        continue_list = cls1->get_continue_list();
        break_list = cls1->get_break_list();
    }
    else if (op == STATEMENTS_TO_STATEMENTS_STATEMENT) {
        continue_list = CodeBuffer::merge(cls1->get_continue_list(), cls2->get_continue_list());
        break_list = CodeBuffer::merge(cls1->get_break_list(), cls2->get_break_list());
    }
}


IfElseCls::IfElseCls(AbsCls* cls1, AbsCls* cls2, AbsCls* cls3, bool is_empty) :
                     nextlist(std::vector<pair<int,BranchLabelIndex>>()),
                     continue_list(std::vector<pair<int,BranchLabelIndex>>()),
                     break_list(std::vector<pair<int,BranchLabelIndex>>()),
                     label(std::string()),
                     is_empty(is_empty) {
    if (cls1 && cls2 && cls3) {
        label = cls2->get_label();
        nextlist = CodeBuffer::merge(cls1->get_nextlist(), cls3->get_nextlist());
        continue_list = cls3->get_continue_list();
        break_list = cls3->get_break_list();
    }
}
