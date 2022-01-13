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
    std::string name;
public:
    LocalVarsReg();
    std::string get_name();
    static unsigned int counter;
};

#endif // _REGISTER_H_
