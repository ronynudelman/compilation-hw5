#include "Register.h"


unsigned int Register::counter = 0;


Register::Register() : name(std::string()) {
    name = std::string("%t") + std::to_string(counter);
    counter++;
}


std::string Register::get_name() {
    return name;
}
