#include "item_type.h"
#include "stringfunc.h"
#include "window.h"

Item_type::Item_type()
{
  uid = -1;
  name = "bug";
  weight = 0;
  volume = 0;
  sym = glyph();
  bash = 0;
  cut = 0;
  pierce = 0;
  to_hit = 0;
  attack_speed = 100;
}

Item_type::~Item_type()
{
}

Item_type_clothing::Item_type_clothing()
{
  debugmsg("weight %d", weight);
  carry_capacity = 0;
  armor_bash = 0;
  armor_cut = 0;
  armor_pierce = 0;
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
      data >> bash;
      std::getline(data, junk);

    } else if (ident == "cut:") {
      data >> cut;
      std::getline(data, junk);

    } else if (ident == "pierce:") {
      data >> pierce;
      std::getline(data, junk);

    } else if (ident == "to_hit:") {
      data >> to_hit;
      std::getline(data, junk);

    } else if (ident == "speed:" || ident == "attack_speed:") {
      data >> attack_speed;
      std::getline(data, junk);

    } else if (ident != "done") {
      debugmsg("Unknown item_type flag '%s' (%s)", ident.c_str(), name.c_str());
    }
  }
// TODO: Flag loading.
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
    case ITEM_CLASS_MAX:      return "BUG - ITEM_CLASS_MAX";
    default:                  return "BUG - Unnamed Item_class";
  }
  return "BUG - Escaped Switch";
}
