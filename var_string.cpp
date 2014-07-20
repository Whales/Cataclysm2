#include "var_string.h"
#include "stringfunc.h"
#include "rng.h"

Variable_string::Variable_string()
{
  total_chance = 0;
}

Variable_string& Variable_string::operator=(const Variable_string& rhs)
{
  strings = rhs.strings;
  total_chance = rhs.total_chance;
  return *this;
}

bool Variable_string::empty()
{
  return strings.empty();
}

std::string Variable_string::pick()
{
  if (strings.empty()) {
    return "";
  }
  int roll = rng(1, total_chance);
  int pick = -1;
  while (roll > 0 && pick < strings.size() - 1) {
    pick++;
    roll -= strings[pick].chance;
  }
  return strings[pick].string;
}

void Variable_string::add_string(int chance, std::string string)
{
  add_string( String_chance(chance, string) );
}

void Variable_string::add_string(String_chance string)
{
  total_chance += string.chance;
  strings.push_back(string);
}

bool Variable_string::load_data(std::istream& data, std::string owner)
{
  std::string ident;
  String_chance tmp_chance;
  while (data >> ident) {
    if (no_caps( ident.substr(0, 2) ) == "w:") { // it's a weight, i.e. a chance
      tmp_chance.chance = atoi( ident.substr(2).c_str() );

    } else if (ident == "/") {  // End of an option
      add_string(tmp_chance);

    } else { 
      tmp_chance.string += ident;
    }
  }
// Add the last one in the line to our list
  add_string(tmp_chance);
  return true;
}

