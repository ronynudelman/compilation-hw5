#include "StringGen.h"


unsigned int StringGen::counter = 0;


StringGen::StringGen() : name(std::string()) {
    name = std::string("s") + std::to_string(counter);
    counter++;
}


std::string StringGen::get_name() {
    return name;
}
