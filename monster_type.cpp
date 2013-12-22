#include "window.h"
#include "stringfunc.h"
#include "monster_type.h"

Monster_type::Monster_type()
{
  name = "Unknown";
  uid = -1;
  sym = glyph();
}

Monster_type::~Monster_type()
{
}

void Monster_type::assign_uid(int id)
{
  uid = id;
}

std::string Monster_type::get_name()
{
  return name;
}

bool Monster_type::load_data(std::istream &data)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident) ) {
      return false;
    }
    ident = no_caps(ident);
    if (ident == "name:") {
      std::getline(data, name);
      name = trim(name);
    } else if (ident == "glyph:") {
      sym.load_data_text(data);
      std::getline(data, junk);
    } else if (ident == "speed:") {
      data >> speed;
      std::getline(data, junk);
    } else if (ident != "done") {
      debugmsg("Unknown monster property '%s' (%s)",
               ident.c_str(), name.c_str());
    }
  }
  return true;
}

