#include "profession.h"
#include "stringfunc.h"
#include "window.h" // For debugmsg()
#include <string>
#include <istream>

Profession::Profession()
{
  uid = -1;
}

Profession::~Profession()
{
}

void Profession::assign_uid(int id)
{
  uid = id;
}

std::string Profession::get_data_name()
{
  return name;
}

bool Profession::load_data(std::istream& data)
{
  std::string ident, junk;

  while (ident != "done" && !data.eof()) {

    if ( ! (data >> ident) ) {
      return false; // Unexpected EOF
    }

    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment
      std::getline(data, junk);

    } else if (ident == "name:") {
      std::getline(data, name);
      name = trim(name);

    } else if (ident == "description:") {
      std::string desc;
      description = "";
      while (no_caps(desc) != "done") {
        std::getline(data, desc);
        desc = trim(desc);
        if (no_caps(desc) != "done") {
          description = description + " " + desc;
        }
      }
      description = trim(description);  // Get rid of extra " "

    } else if (ident == "items:") {
      if (!items.load_item_data(data, name)) {
        return false;
      }

    } else if (ident == "skills:") {
      std::string skill_name;
      while (no_caps(skill_name) != "done") {
        data >> skill_name;
        skill_name = trim(skill_name);
        if (no_caps(skill_name) != "done") {
          Skill_type skill = lookup_skill_type(skill_name);
          if (skill == SKILL_NULL) {
            debugmsg("Unknown skill '%s' (Profession %s)",
                     skill_name.c_str(), name.c_str());
            return false;
          }
          int level;
          data >> level;
          std::getline(data, junk); // chomp
          skills.set_level(skill, level);
        }
      }

    } else if (ident != "done") {
      debugmsg("Unknown Profession property '%s' (%s)",
               ident.c_str(), name.c_str());
    }
  }
  return true;
}
