#ifndef _STRING_GEN_H_
#define _STRING_GEN_H_


#include <string>


class StringGen {
private:
    std::string name;
public:
    StringGen();
    std::string get_name();
    static unsigned int counter;
};


#endif // _STRING_GEN_H_
