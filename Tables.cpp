#include <list>
#include <string>
#include <cassert>
#include "hw3_output.hpp"
#include "Tables.h"
#include "Utilities.h"


SymbolTableEntry::SymbolTableEntry(std::string name, int offset, std::string type, bool is_func,
                                  std::vector<std::string> arguments) : name(name),
                                                                        offset(offset),
                                                                        type(type),
                                                                        is_func(is_func),
                                                                        arguments(arguments) {}


std::string& SymbolTableEntry::get_name() {
  return name;
}


int SymbolTableEntry::get_offset() {
  return offset;
}


std::string& SymbolTableEntry::get_type() {
  return type;
}


bool SymbolTableEntry::get_is_func() {
  return is_func;
}


std::vector<std::string>& SymbolTableEntry::get_arguments() {
  return arguments;
}


void SymbolTable::push_entry(std::string name, int offset, std::string type, bool is_func,
                            std::vector<std::string> arguments) {
  SymbolTableEntry new_entry(name, offset, type, is_func, arguments);
  symbol_table_entries.push_back(new_entry);
}


void SymbolTable::pop_entry() {
  symbol_table_entries.pop_back();
}

SymbolTableEntry* SymbolTable::get_entry_by_name(std::string name) {
  for (std::list<SymbolTableEntry>::iterator it = symbol_table_entries.begin(); it != symbol_table_entries.end(); ++it) {
    if (it->get_name() == name) {
      return &(*it);
    }
  }
  return nullptr;
}


void SymbolTable::print() {
  for (std::list<SymbolTableEntry>::iterator it = symbol_table_entries.begin(); it != symbol_table_entries.end(); ++it) {
    SymbolTableEntry& current_entry = *it;
    std::string current_entry_name = current_entry.get_name();
    std::string current_entry_type = strip_const(current_entry.get_type());
    int current_entry_offset = current_entry.get_offset();
    if (current_entry.get_is_func()) {
      std::vector<std::string> current_entry_args = strip_const(current_entry.get_arguments());
      current_entry_type = output::makeFunctionType(current_entry_type, current_entry_args);
    }
    output::printID(current_entry_name, current_entry_offset, current_entry_type);
  }
}


SymbolTableEntry& SymbolTable::top_symbol_table_entry() {
	return symbol_table_entries.back();
}


SymbolTableStack::SymbolTableStack() : symbol_tables(std::list<SymbolTable>()) {
  SymbolTable new_symbol_table;
  std::vector<std::string> print_func_args;
  print_func_args.push_back("STRING");
  new_symbol_table.push_entry("print", 0, "VOID", true, print_func_args);
  std::vector<std::string> printi_func_args;
  printi_func_args.push_back("INT");
  new_symbol_table.push_entry("printi", 0, "VOID", true, printi_func_args);
  symbol_tables.push_back(new_symbol_table);
}


void SymbolTableStack::push_symbol_table() {
  SymbolTable new_symbol_table;
  symbol_tables.push_back(new_symbol_table);
}


void SymbolTableStack::pop_symbol_table() {
  symbol_tables.pop_back();
}


SymbolTable& SymbolTableStack::top_symbol_table() {
  return symbol_tables.back();
}


SymbolTableEntry* SymbolTableStack::get_entry_by_name(std::string name) {
  for (std::list<SymbolTable>::iterator it = symbol_tables.begin(); it != symbol_tables.end(); ++it) {
    SymbolTableEntry* entry = it->get_entry_by_name(name);
	if(entry){
		return entry;
	}
  }
  return nullptr;
}


void SymbolTableStack::print_top_symbol_table() {
  SymbolTable& symbol_table = top_symbol_table();
  symbol_table.print();
}


SymbolTable& SymbolTableStack::first_symbol_table() {
	return symbol_tables.front();
}


OffsetTableStack::OffsetTableStack() : offsets(std::list<int>()) {
    offsets.push_back(0);
}


void OffsetTableStack::push_offset() {
  int copy_top_offset = offsets.back();
  offsets.push_back(copy_top_offset);
}


void OffsetTableStack::pop_offset() {
  offsets.pop_back();

}


int OffsetTableStack::top_offset() {
  return offsets.back();
}


void OffsetTableStack::inc_top_offset() {
  (offsets.back())++;
}


void ConstTable::remove(std::string id) {
    for (std::list<pair<std::string, int>>::iterator it = const_values.begin(); it != const_values.end(); ++it) {
        if (it->first == id) {
            const_values.erase(it);
            break;
        }
    }
}


void ConstTable::update(std::string id, int value) {
    for (std::list<pair<std::string, int>>::iterator it = const_values.begin(); it != const_values.end(); ++it) {
        if (it->first == id) {
            it->second = value;
            return;
        }
    }
    pair<std::string, int> new_pair;
    new_pair.first = id;
    new_pair.second = value;
    const_values.push_front(new_pair);
}


int ConstTable::get_value(std::string id) {
    for (std::list<pair<std::string, int>>::iterator it = const_values.begin(); it != const_values.end(); ++it) {
        if (it->first == id) {
            return it->second;
        }
    }
    return -1;
}
