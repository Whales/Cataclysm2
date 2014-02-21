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
  thrown_variance = Dice(8, 20, 0);
  thrown_dmg_percent = 50;
  thrown_speed = 0;
  for (int i = 0; i < ITEM_FLAG_MAX; i++) {
    flags.push_back(false);
  }
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
  encumbrance = 0;
}

std::string Item_type_clothing::get_property_description()
{
  std::stringstream ret;
  ret << "<c=white>Covers:<c=/> ";
  bool printed_any = false;
  for (int i = 0; i < BODY_PART_MAX; i++) {
    if (covers[i]) {
      if (printed_any) {
        ret << ", ";
      } else {
        printed_any = true;
      }
      ret << body_part_name( Body_part(i) );
    }
  }
  ret << std::endl;

  ret << "<c=ltblue>Storage space: ";
  if (carry_capacity == 0) {
    ret << "<c=dkgray>";
  } else if (carry_capacity >= 25) {  // TODO: Change / don't hardcode this
    ret << "<c=white>";
  } else {
    ret << "<c=ltgray>";
  }
  ret << carry_capacity << "<c=/>\n";
  ret << "<c=magenta>Encumbrance: ";
  if (encumbrance == 0) {
    ret << "<c=white>";
  } else if (encumbrance < 3) {
    ret << "<c=ltgray>";
  } else if (encumbrance < 5) {
    ret << "<c=ltred>";
  } else {
    ret << "<c=red>";
  }
  ret << encumbrance << std::endl;
// Set up vectors for use in color_gradient() below
  std::vector<int> breakpoints;
  breakpoints.push_back(0);
  breakpoints.push_back(10);
  std::vector<nc_color> colors;
  colors.push_back( c_dkgray );
  colors.push_back( c_ltgray );
  colors.push_back( c_white  );

  ret << "<c=brown>Armor (bash): " <<
         color_gradient(armor_bash, breakpoints, colors) << armor_bash <<
         "<c=/>\n";
  ret << "<c=brown>Armor (cut): " <<
         color_gradient(armor_cut, breakpoints, colors) << armor_cut <<
         "<c=/>\n";
  ret << "<c=brown>Armor (pierce): " <<
         color_gradient(armor_pierce, breakpoints, colors) << armor_pierce <<
         "<c=/>\n";

  return ret.str();
}

Item_type_ammo::Item_type_ammo()
{
  damage = 0;
  armor_pierce = 10;
  range = 0;
  count = 100;
}

std::string Item_type_ammo::get_property_description()
{
  std::stringstream ret;
  ret << "<c=white>Type:<c=/> " << ammo_type << std::endl;
  ret << "<c=cyan>Accuracy:<c=/> " << accuracy.str() << std::endl;
  ret << "<c=red>Damage:<c=white> " << damage << "\n<c=ltblue>Armor Piercing: ";
  if (armor_pierce == 10) {
    ret << "<c=dkgray>None";
  } else {
    if (armor_pierce < 10) {
      ret << "<c=red>";
    } else {
      ret << "<c=white>";
    }
    ret << "x" << armor_pierce / 10 << "." << armor_pierce % 10;
  }
  ret << "<c=/>\n";
  ret << "<c=ltgray>Range: " << range << "<c=/>\n";

  return ret.str();
}

Item_type_launcher::Item_type_launcher()
{
  damage = 0;
  recoil = 0;
  durability = 100;
  capacity = 15;
  reload_ap = 300;
  fire_ap = 100;
}

std::string Item_type_launcher::get_property_description()
{
  std::stringstream ret;
  ret << "<c=white>Ammo:<c=/> " << ammo_type << std::endl;
  ret << "<c=ltred>Damage bonus:<c=/> " << (damage >= 0 ? "+" : "") <<
         damage << std::endl;
  ret << "<c=cyan>Accuracy:<c=/> " << accuracy.str() << std::endl;
  ret << "<c=magenta>Recoil:<c=/> " << (recoil >= 0 ? "+" : "") <<
         recoil << std::endl;
  ret << "<c=brown>Durability:<c=/> " << durability << std::endl;
  ret << "<c=ltblue>Capacity:<c=/> " << capacity << " rounds" << std::endl;
  ret << "<c=green>Reload time:<c=/> " << reload_ap / 100 << "." <<
         (reload_ap % 100) / 10 << reload_ap % 10 << " turns" << std::endl;
  ret << "<c=ltgreen>Fire time:<c=/> " << fire_ap / 100 << "." <<
         (fire_ap % 100) / 10 << fire_ap % 10 << " turns" << std::endl;
  ret << "<c=red>Fire modes:<c=/> ";
// TODO: If there's special mods, like "snipe," handle them specially
  for (int i = 0; i < modes.size(); i++) {
    ret << "[" << modes[i] << "] ";
  }
  return ret.str();
}


Item_type_food::Item_type_food()
{
  food = 0;
  water = 0;
  charges = 1;
  verb = "eat";
}

std::string Item_type_food::get_property_description()
{
  std::stringstream ret;
  ret << "<c=green>Nutrition: <c=/>" << food << std::endl;
  ret << "<c=ltblue>Water: <c=/>" << water << std::endl;
  return ret.str();
}

Item_type_tool::Item_type_tool()
{
  default_charges = 0;
  max_charges = 0;
  countdown_timer = 0;
}

// We won't try to handle the Tool_action here - that's better done by hand
std::string Item_type_tool::get_property_description()
{
  std::stringstream ret;
  ret << "<c=yellow>Fuel:<c=/> " << fuel << std::endl;
  ret << "<c=white>Max charges:<c=/> " << max_charges << std::endl;
  return ret.str();
}

void Item_type::assign_uid(int id)
{
  uid = id;
}

std::string Item_type::get_data_name()
{
  return name;
}

std::string Item_type::get_name()
{
  if (display_name.empty()) {
    return name;
  }
  return display_name;
}

bool Item_type::load_data(std::istream &data)
{
  std::string ident, junk;
  bool set_name = false, set_glyph = false;
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
      set_name = true;

    } else if (ident == "display_name:") {
      std::getline(data, display_name);
      display_name = trim(display_name);

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

    } else if (ident == "glyph:") {
      sym.load_data_text(data);
      std::getline(data, junk);
      set_glyph = true;

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

    } else if (ident == "thrown_variance:") {
      if (!thrown_variance.load_data(data, name)) {
        return false;
      }

    } else if (ident == "thrown_dmg_percent:") {
      data >> thrown_dmg_percent;
      std::getline(data, junk);

    } else if (ident == "thrown_speed:") {
      data >> thrown_speed;
      std::getline(data, junk);

    } else if (ident == "flags:") {
      std::string flag_line;
      std::getline(data, flag_line);
      std::istringstream flag_data(flag_line);
      std::string flag_name;
      while (flag_data >> flag_name) {
        Item_flag flag = lookup_item_flag(flag_name);
        if (flag == ITEM_FLAG_NULL) {
          debugmsg("Unknown item flag '%s' (%s)",
                   flag_name.c_str(), name.c_str());
          return false;
        }
        flags[flag] = true;
      }

    } else if (!handle_data(ident, data)) {
      debugmsg("Unknown item_type flag '%s' (%s)", ident.c_str(), name.c_str());
      return false;

    }
  }
// Ensure that we set a glyph and name!
  if (!set_name) {
    debugmsg("Item created without a name!");
    return false;
  }
  if (!set_glyph) {
    debugmsg("Item '%s' created without a glyph!", name.c_str());
    return false;
  }
  return true;
}

bool Item_type::handle_data(std::string ident, std::istream &data)
{
  if (ident == "done") {
    return true;
  }
  return false;
}

bool Item_type::has_flag(Item_flag flag)
{
  return flags[flag];
}

/* TODO:  Right now, armor{_bash,_cut,_pierce} is hard-coded here.  But what if
 *        we add a damage type and want to protect against it with armor?  We
 *        should generalize and look up damage type names instead.
 */
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
      std::vector<Body_part> parts = get_body_part_list( body_part_name );
      for (int i = 0; i < parts.size(); i++) {
        covers[ parts[i] ] = true;
      }
      if (parts.empty()) {
        debugmsg("Unknown body part '%s' (%s)", body_part_name.c_str(),
                 name.c_str());
        return false;
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
      armor_pierce = 10;
    }
    std::getline(data, junk);

  } else if (ident == "range:") {
    data >> range;
    std::getline(data, junk);

  } else if (ident == "accuracy:") {
    if (!accuracy.load_data(data, name)) {
      return false;
    }

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
    if (!accuracy.load_data(data, name)) {
      return false;
    }

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

  } else if (ident == "effect:") {
    if (!effect.load_data(data, name)) {
      return false;
    }

  } else if (ident == "charges:") {
    data >> charges;
    std::getline(data, junk);

  } else if (ident == "container:") {
    std::getline(data, container);
    container = no_caps(container);
    container = trim(container);
    if (container.empty()) {
      debugmsg("Empty container (%s)", name.c_str());
      return false;
    }

  } else if (ident == "verb:") {
    std::getline(data, verb);
    verb = no_caps(verb);
    verb = trim(verb);
    if (verb.empty()) {
      debugmsg("Empty verb (%s)", name.c_str());
      return false;
    }

  } else if (ident != "done") {
    return false;
  }
  return true;
}

bool Item_type_tool::handle_data(std::string ident, std::istream &data)
{
  std::string junk;

  if (ident == "applied:") {
    if (!applied_action.load_data(data, name)) {
      return false;
    }

  } else if (ident == "powered:") {
    if (!powered_action.load_data(data, name)) {
      return false;
    }

  } else if (ident == "countdown:") {
    if (!countdown_action.load_data(data, name)) {
      return false;
    }

  } else if (ident == "countdown_timer:") {
    data >> countdown_timer;
    std::getline(data, junk);

  } else if (ident == "default_charges:") {
    data >> default_charges;
    std::getline(data, junk);

  } else if (ident == "max_charges:") {
    data >> max_charges;
    std::getline(data, junk);

  } else if (ident == "fuel:") {
    std::getline(data, fuel);
    fuel = no_caps(fuel);
    fuel = trim(fuel);

  } else if (ident == "powered_text:") {
    std::getline(data, powered_text);
    powered_text = no_caps(powered_text);
    powered_text = trim(powered_text);



  } else if (ident != "done") {
    debugmsg("Unknown Tool property '%s' (%s)", ident.c_str(), name.c_str());
    return false;
  }
  return true;
}

bool Item_type_tool::uses_charges()
{
  return (max_charges > 0 && applied_action.charge_cost > 0);
}

Item_type_container::Item_type_container()
{
  capacity = 0;
}

std::string Item_type_container::get_property_description()
{
  std::stringstream ret;
  ret << "<c=white>Capacity:<c=/> " << capacity;
  return ret.str();
}

bool Item_type_container::handle_data(std::string ident, std::istream &data)
{
  std::string junk;
  if (ident == "capacity:") {
    data >> capacity;
    std::getline(data, junk);

  } else if (ident != "done") {
    debugmsg("Unknown Container property '%s' (%s)",
             ident.c_str(), name.c_str());
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

std::string item_class_name_plural(Item_class iclass)
{
  return item_class_name(iclass, true);
}

// plural defaults to false
std::string item_class_name(Item_class iclass, bool plural)
{
  switch (iclass) {
    case ITEM_CLASS_MISC:       return "Misc";
    case ITEM_CLASS_CLOTHING:   return "Clothing";
    case ITEM_CLASS_AMMO:       return "Ammo";
    case ITEM_CLASS_LAUNCHER:   return (plural ? "Launchers"  : "Launcher");
    case ITEM_CLASS_FOOD:       return "Food";
    case ITEM_CLASS_TOOL:       return (plural ? "Tools"      : "Tool");
    case ITEM_CLASS_CONTAINER:  return (plural ? "Containers" : "Container");

    case ITEM_CLASS_MAX:        return "BUG - ITEM_CLASS_MAX";
    default:                    return "BUG - Unnamed Item_class";
  }
  return "BUG - Escaped item_class_name switch";
}

Item_flag lookup_item_flag(std::string name)
{
  name = no_caps(name);
  for (int i = 0; i < ITEM_FLAG_MAX; i++) {
    Item_flag ret = Item_flag(i);
    if ( no_caps( item_flag_name(ret) ) == name ) {
      return ret;
    }
  }
  return ITEM_FLAG_NULL;
}

std::string item_flag_name(Item_flag flag)
{
  switch (flag) {
    case ITEM_FLAG_NULL:      return "NULL";
    case ITEM_FLAG_LIQUID:    return "liquid";
    case ITEM_FLAG_FLAMMABLE: return "flammable";
    case ITEM_FLAG_PLURAL:    return "plural";
    case ITEM_FLAG_CONSTANT:  return "constant_volume_weight";
    case ITEM_FLAG_MAX:       return "BUG - ITEM_FLAG_MAX";
    default:                  return "BUG - Unnamed Item_flag";
  }
  return "BUG - Escaped item_flag_name switch";
}
