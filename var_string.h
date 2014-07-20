#ifndef _VAR_STRING_H_
#define _VAR_STRING_H_

#include <string>
#include <vector>

struct String_chance
{
  String_chance(int C = 10, std::string N = "") : chance (C), string (N) {}
  int chance;
  std::string string;
};

struct Variable_string
{
public:
  Variable_string();
  ~Variable_string(){}

  Variable_string& operator=(const Variable_string& rhs);

  bool empty();
  std::string pick();

  void add_string(int chance, std::string string);
  void add_string(String_chance string);
  bool load_data(std::istream &data, std::string owner = "unknown");

  std::vector<String_chance> strings;
  int total_chance;
};

#endif
