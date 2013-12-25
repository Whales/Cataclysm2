#include <sstream>
#include "window.h"
#include "stringfunc.h"
#include "monster_type.h"

Monster_attack::Monster_attack()
{
  verb = "hits";
  speed = 100;
  to_hit = 0;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    damage[i] = 0;
  }
}

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

    } else if (ident == "attack:") {
      std::getline(data, junk);
      std::string attack_ident;
      Monster_attack tmpattack;

      while ( data >> attack_ident && attack_ident != "done" ) {
        if (attack_ident == "verb:") {
          std::getline(data, tmpattack.verb);
        } else if (attack_ident == "speed:") {
          data >> tmpattack.speed;
          std::getline(data, junk);
        } else if (attack_ident == "to_hit:") {
          data >> tmpattack.to_hit;
          std::getline(data, junk);
        } else if (attack_ident == "bash:"    || attack_ident == "bashing:") {
          data >> tmpattack.damage[DAMAGE_BASH];
        } else if (attack_ident == "cut:"     || attack_ident == "cutting:") {
          data >> tmpattack.damage[DAMAGE_CUT];
        } else if (attack_ident == "pierce:"  || attack_ident == "piercing:") {
          data >> tmpattack.damage[DAMAGE_PIERCE];
        } else if (attack_ident != "done") {
          debugmsg("Unknown attack_ident '%s' (%s)", attack_ident.c_str(),
                   name.c_str());
        }
      }
      attacks.push_back(tmpattack);

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
