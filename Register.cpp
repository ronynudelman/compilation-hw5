#include "Register.h"


unsigned int Register::counter = 0;
unsigned int LocalVarsReg::counter = 0;
std::string LocalVarsReg::name = std::string();


Register::Register() : name(std::string()) {
    name = std::string("%t") + std::to_string(counter);
    counter++;
}


std::string Register::get_name() {
    return name;
}


std::string LocalVarsReg::update() {
    name = std::string("%loc_var") + std::to_string(counter);
    counter++;
    return name;
}


std::string LocalVarsReg::get_name() {
    return name;
}
