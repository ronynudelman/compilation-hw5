#ifndef TABLES_H_
#define  TABLES_H_


#include <list>
#include <vector>
#include <string>
#include "Types.h"


class SymbolTableEntry {
private:
  std::string name;
  int offset;
  std::string type; // for functions, it's the return type
  bool is_func;
  std::vector<std::string> arguments;
public:
  SymbolTableEntry(std::string name, int offset, std::string type, bool is_func = false,
                   std::vector<std::string> arguments = std::vector<std::string>());
  std::string& get_name();
  int get_offset();
  std::string& get_type();
  bool get_is_func();
  std::vector<std::string>& get_arguments();
};


class SymbolTable {
private:
  std::list<SymbolTableEntry> symbol_table_entries;
public:
  SymbolTable() = default;
  void push_entry(std::string name, int offset, std::string type, bool is_func = false,
                  std::vector<std::string> arguments = std::vector<std::string>());
  void pop_entry();
  SymbolTableEntry* get_entry_by_name(std::string name);
  SymbolTableEntry& top_symbol_table_entry();
  void print();
};


class SymbolTableStack {
private:
  std::list<SymbolTable> symbol_tables;
public:
  SymbolTableStack();
  void push_symbol_table();
  void pop_symbol_table();
  SymbolTable& top_symbol_table();
  SymbolTable& first_symbol_table();
  SymbolTableEntry* get_entry_by_name(std::string name);
  void print_top_symbol_table();
};


class OffsetTableStack {
private:
  std::list<int> offsets;
public:
  OffsetTableStack();
  void push_offset();
  void pop_offset();
  int top_offset();
  void inc_top_offset();
};


class ConstTable {
private:
    std::list<pair<std::string, std::string>> const_values;
public:
    ConstTable() = default;
    void remove(std::string id);
    void update(std::string id, std::string value);
    std::string get_value(std::string id);
    void print() {
        std::cout << "*** Printing ConstTable ***" << std::endl;
        for (std::list<pair<std::string, std::string>>::iterator it = const_values.begin(); it != const_values.end(); ++it) {
            std::cout << "id: " << it->first << ", value: " << it->second << std::endl;
        }
        std::cout << "***************************" << std::endl;
    }
};


#endif // TABLES_H_
