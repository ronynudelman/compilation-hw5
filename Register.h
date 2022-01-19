#ifndef _REGISTER_H_
#define _REGISTER_H_


#include <string>


class Register {
private:
    std::string name;
public:
    Register();
    std::string get_name();
    static unsigned int counter;
};

class LocalVarsReg {
private:
    static unsigned int counter;
    static std::string name;
public:
    LocalVarsReg() = default;
    std::string update();
    std::string get_name();
};


#endif // _REGISTER_H_
