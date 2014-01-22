#include "item_type.h"
#include "stringfunc.h"
#include "window.h"
#include <sstream>

Item_type::Item_type()
{
  uid = -1;
  name = "bug";
  sym = glyph();
  weight = 0;
  volume = 0;
  for (int i = 0; i < DAMAGE_MAX; i++) {
    damage[i] = 0;
  }
  to_hit = 0;
  attack_speed = 0;
  ranged_variance = 0;
  ranged_dmg_bonus = 5;
  ranged_speed = 0;
}

Item_type::~Item_type()
{
}

Item_type_clothing::Item_type_clothing()
{
  carry_capacity = 0;
  armor_bash = 0;
  armor_cut = 0;
  armor_pierce = 0;
}

Item_type_ammo::Item_type_ammo()
{
  damage = 0;
  armor_pierce = 1;
  range = 0;
  accuracy = 0;
  count = 100;
}

Item_type_launcher::Item_type_launcher()
{
  damage = 0;
  accuracy = 0;
  recoil = 0;
  durability = 100;
  capacity = 15;
  reload_ap = 300;
}
void Item_type::assign_uid(int id)
{
  uid = id;
}

std::string Item_type::get_name()
{
  return name;
}

bool Item_type::load_data(std::istream &data)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident)) {
      return false;
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
      while (no_caps(desc) != "done") {
        std::getline(data, desc);
        desc = trim(desc);
        if (no_caps(desc) != "done") {
          description = description + " " + desc;
        }
      }

    } else if (ident == "glyph:") {
      sym.load_data_text(data);
      std::getline(data, junk);

    } else if (ident == "weight:") {
      data >> weight;
      std::getline(data, junk);

    } else if (ident == "volume:") {
      data >> volume;
      std::getline(data, junk);

    } else if (ident == "bash:") {
      data >> damage[DAMAGE_BASH];
      std::getline(data, junk);

    } else if (ident == "cut:") {
      data >> damage[DAMAGE_CUT];
      std::getline(data, junk);

    } else if (ident == "pierce:") {
      data >> damage[DAMAGE_PIERCE];
      std::getline(data, junk);

    } else if (ident == "to_hit:") {
      data >> to_hit;
      std::getline(data, junk);

    } else if (ident == "speed:" || ident == "attack_speed:") {
      data >> attack_speed;
      std::getline(data, junk);

    } else if (ident == "ranged_variance:") {
      data >> ranged_variance;
      std::getline(data, junk);

    } else if (ident == "ranged_dmg_bonus:") {
      data >> ranged_dmg_bonus;
      std::getline(data, junk);

    } else if (ident == "ranged_speed:") {
      data >> ranged_speed;
      std::getline(data, junk);

    } else if (!handle_data(ident, data)) {
      debugmsg("Unknown item_type flag '%s' (%s)", ident.c_str(), name.c_str());
      return false;

    }
  }
// TODO: Flag loading.
  return true;
}

bool Item_type::handle_data(std::string ident, std::istream &data)
{
  if (ident == "done") {
    return true;
  }
  return false;
}

bool Item_type_clothing::handle_data(std::string ident, std::istream &data)
{
  std::string junk;
  if (ident == "carries:") {
    data >> carry_capacity;
    std::getline(data, junk);

  } else if (ident == "armor_bash:") {
    data >> armor_bash;
    std::getline(data, junk);

  } else if (ident == "armor_cut:") {
    data >> armor_cut;
    std::getline(data, junk);

  } else if (ident == "armor_pierce:") {
    data >> armor_pierce;
    std::getline(data, junk);

  } else if (ident == "encumbrance:") {
    data >> encumbrance;
    std::getline(data, junk);

  } else if (ident == "covers:") {
    std::string line;
    std::getline(data, line);
    std::istringstream cover_data(line);
    std::string body_part_name;
    while (cover_data >> body_part_name) {
      if (body_part_name == "arms") {
        covers[BODYPART_LEFT_ARM]  = true;
        covers[BODYPART_RIGHT_ARM] = true;
      } else if (body_part_name == "legs") {
        covers[BODYPART_LEFT_LEG]  = true;
        covers[BODYPART_RIGHT_LEG] = true;
      } else {
        covers[ lookup_body_part( body_part_name ) ] = true;
      }
    }

  } else if (ident != "done") {
    return false;
  }
  return true;
}

bool Item_type_ammo::handle_data(std::string ident, std::istream &data)
{
  std::string junk;
  if (ident == "type:") {
    std::getline(data, ammo_type);

  } else if (ident == "damage:") {
    data >> damage;
    std::getline(data, junk);

  } else if (ident == "armor_pierce:" || ident == "pierce:") {
    data >> armor_pierce;
    if (armor_pierce <= 0) {
      armor_pierce = 1;
    }
    std::getline(data, junk);

  } else if (ident == "range:") {
    data >> range;
    std::getline(data, junk);

  } else if (ident == "accuracy:") {
    data >> accuracy;
    std::getline(data, junk);

  } else if (ident == "count:") {
    data >> count;
    std::getline(data, junk);

  } else if (ident != "done") {
    return false;
  }
  return true;
}

bool Item_type_launcher::handle_data(std::string ident, std::istream &data)
{
  std::string junk;
  if (ident == "type:" || ident == "ammo_type:") {
    std::getline(data, ammo_type);

  } else if (ident == "damage:") {
    data >> damage;
    std::getline(data, junk);

  } else if (ident == "accuracy:") {
    data >> accuracy;
    std::getline(data, junk);

  } else if (ident == "recoil:") {
    data >> recoil;
    std::getline(data, junk);

  } else if (ident == "durability:") {
    data >> durability;
    std::getline(data, junk);

// "Clip" and "magazine" to satisfy the gun nerds.  what up gun nerds
  } else if (ident == "clip:" || ident == "magazine:" || ident == "capacity:") {
    data >> capacity;
    std::getline(data, junk);

  } else if (ident == "reload_time:" || ident == "reload_ap:") {
    data >> reload_ap;
    std::getline(data, junk);

  } else if (ident == "fire_time:" || ident == "fire_ap:") {
    data >> fire_ap;
    std::getline(data, junk);

  } else if (ident == "modes:" || ident == "mode:") {
    std::string mode_line;
    std::getline(data, mode_line);
    std::istringstream mode_ss(mode_line);
    int mode;
    while (mode_ss >> mode) {
      modes.push_back(mode);
    }

  } else if (ident != "done") {
    return false;
  }
  return true;
}

bool Item_type_food::handle_data(std::string ident, std::istream &data)
{
  std::string junk;

  if (ident == "food:") {
    data >> food;
    std::getline(data, junk);

  } else if (ident == "water:") {
    data >> water;
    std::getline(data, junk);

  } else if (ident != "done") {
    return false;
  }
  return true;
}

Item_class lookup_item_class(std::string name)
{
  name = no_caps(name);
  for (int i = 0; i < ITEM_CLASS_MAX; i++) {
    Item_class ret = Item_class(i);
    if ( no_caps( item_class_name(ret) ) == name ) {
      return ret;
    }
  }
  return ITEM_CLASS_MISC;
}

std::string item_class_name(Item_class iclass)
{
  switch (iclass) {
    case ITEM_CLASS_MISC:     return "Misc";
    case ITEM_CLASS_CLOTHING: return "Clothing";
    case ITEM_CLASS_AMMO:     return "Ammo";
    case ITEM_CLASS_LAUNCHER: return "Launcher";
    case ITEM_CLASS_MAX:      return "BUG - ITEM_CLASS_MAX";
    default:                  return "BUG - Unnamed Item_class";
  }
  return "BUG - Escaped Switch";
}
