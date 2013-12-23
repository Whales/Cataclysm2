#include <sstream>
#include "window.h"
#include "stringfunc.h"
#include "monster_type.h"

Monster_type::Monster_type()
{
  name = "Unknown";
  uid = -1;
  sym = glyph();
  for (int i = 0; i < SENSE_MAX; i++) {
    senses.push_back(false);
  }
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

    if (!ident.empty() && ident[0] == '#') {
// It's a comment
      std::getline(data, junk);

    } else if (ident == "name:") {
      std::getline(data, name);
      name = trim(name);

    } else if (ident == "glyph:") {
      sym.load_data_text(data);
      std::getline(data, junk);

    } else if (ident == "speed:") {
      data >> speed;
      std::getline(data, junk);

    } else if (ident == "senses:") {
      std::string sense_line;
      std::getline(data, sense_line);
      std::istringstream sense_data(sense_line);

      std::string sense_name;
      while (sense_data >> sense_name) {
        senses[ lookup_sense_type(sense_name) ] = true;
      }

    } else if (ident != "done") {
      debugmsg("Unknown monster property '%s' (%s)",
               ident.c_str(), name.c_str());
    }
  }
  return true;
}

bool Monster_type::has_sense(Sense_type sense)
{
  return senses[sense];
}
