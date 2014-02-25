#include "explosion.h"
#include "window.h" // For debugmsg
#include "stringfunc.h" // For no_caps() and trim()

Explosion::Explosion()
{
  radius = Dice(0, 0, 1);
  force  = Dice(0, 0, 1);

  field_chance = 0;
}

Explosion::~Explosion()
{
}

bool Explosion::load_data(std::istream& data, std::string owner_name)
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

    } else if (ident == "radius:") {
      if (!radius.load_data(data, owner_name + " radius")) {
        return false;
      }

    } else if (ident == "force:") {
      if (!force.load_data(data, owner_name + " force")) {
        return false;
      }

    } else if (ident == "shrapnel_count:") {
      if (!shrapnel_count.load_data(data, owner_name + " shrapnel count")) {
        return false;
      }

    } else if (ident == "shrapnel_damage:") {
      if (!shrapnel_damage.load_data(data, owner_name + " shrapnel damage")) {
        return false;
      }

    } else if (ident == "field:" || ident == "field_name:") {
      std::getline(data, field_name);
      field_name = no_caps(field_name);
      field_name = trim(field_name);
      if (field_name.empty()) {
        debugmsg("Empty explosion field name (%s)", owner_name.c_str());
        return false;
      }

    } else if (ident == "field_chance:") {
      data >> field_chance;
      if (field_chance < 1) {
        debugmsg("Correcting field_chance of %d to 1. (%s)", field_chance,
                 owner_name.c_str());
        field_chance = 1;
      } else if (field_chance > 100) {
        debugmsg("Correcting field_chance of %d to 100. (%s)", field_chance,
                 owner_name.c_str());
        field_chance = 100;
      }

    } else if (ident == "field_duration:") {
      if (!field_duration.load_data(data, owner_name + " field duration")) {
        return false;
      }

    } else if (ident != "done") {
      debugmsg("Unknown Explosion property '%s' (%s)", ident.c_str(),
               owner_name.c_str());
      return false;
    }
  }
  return true;
}

void Explosion::explode(Tripoint epicenter)
{
}

