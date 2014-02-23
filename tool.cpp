#include "tool.h"
#include "stringfunc.h" // For trim() and no_caps()
#include "window.h" // For debugmsg()
#include "enum.h" // For HP_part etc
#include "entity.h"
#include "player.h"
#include "field.h"
#include "game.h"
#include <sstream>

Tool_special_explosion::Tool_special_explosion()
{
  radius = Dice(0, 0,  8);
  damage = Dice(0, 0, 20);
}

bool Tool_special_explosion::load_data(std::istream& data,
                                       std::string owner_name)
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

    } else if (ident == "radius:") {
      if (!radius.load_data(data, owner_name + " explosion")) {
        return false;
      }

    } else if (ident == "damage:") {
      if (!damage.load_data(data, owner_name + " explosion")) {
        return false;
      }

    } else if (ident != "done") {
      debugmsg("Unknown Tool_special_explosion flag '%s' (%s)",
               ident.c_str(), owner_name.c_str());
      return false;
    }
  }
  return true;
}

Tool_special_light::Tool_special_light()
{
  light = 8;
}

bool Tool_special_light::load_data(std::istream& data, std::string owner_name)
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

    } else if (ident == "light:") {
      data >> light;
      std::getline(data, junk);

    } else if (ident != "done") {
      debugmsg("Unknown Tool_special_light flag '%s' (%s)",
               ident.c_str(), owner_name.c_str());
      return false;
    }
  }
  return true;
}

bool Tool_special_light::effect(Entity* user)
{
  GAME.set_temp_light_level( light );
  return true;
}

Tool_special_heal::Tool_special_heal()
{
  min_amount = 0;
  max_amount = 0;
}

bool Tool_special_heal::load_data(std::istream& data, std::string owner_name)
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

    } else if (ident == "amount:") {
      data >> min_amount;
      max_amount = min_amount;
      std::getline(data, junk);

    } else if (ident == "min:" || ident == "min_amount:") {
      data >> min_amount;
      std::getline(data, junk);

    } else if (ident == "max:" || ident == "max_amount:") {
      data >> max_amount;
      std::getline(data, junk);

    } else if (ident != "done") {
      debugmsg("Unknown Tool_special_heal flag '%s' (%s)",
               ident.c_str(), owner_name.c_str());
      return false;
    }
  }
  if (max_amount < min_amount) {
    max_amount = min_amount;
  }
  return true;
}

bool Tool_special_heal::effect(Entity* user)
{
  if (!user) {
    return false;
  }

// TODO: Increase amount as our First Aid skill increases.
  int amount_healed = min_amount;
// TODO: If user is an NPC, auto-choose
// TODO: Allow us to target things (monsters/NPCs) other than ourselves
  Player* player = NULL;
  if (user->is_you()) {
    player = static_cast<Player*>(user);
  }
  std::vector<std::string> options;
  options.push_back("Cancel");
  for (int i = 1; i < HP_PART_MAX; i++) {
    std::stringstream option_ss;
    option_ss << HP_part_name( HP_part(i) );
    if (player) {
      option_ss << ": " << player->hp_text( HP_part(i) );
      int result = player->current_hp[i] + amount_healed;
      if (result > player->max_hp[i]) {
        result = player->max_hp[i];
      }
      option_ss << " => " << result;
    }
    options.push_back( option_ss.str() );
  }

  std::stringstream menu_title;
  menu_title << "Healing " << amount_healed << ":";
  int selection = menu_vec(menu_title.str().c_str(), options);
  if (selection == 0) {
    return false; // We canceled
  }
  user->heal_damage(amount_healed, HP_part(selection));
  return true;
}

Tool_action::Tool_action()
{
  signal = "";
  special = NULL;
  target  = TOOL_TARGET_NULL;
  ap_cost = 100;
  charge_cost = 1;
  range = 0;
  destroy_if_chargeless = false;
  real = false;
}

Tool_action::~Tool_action()
{
  if (special) {
    delete special;
  }
}

bool Tool_action::load_data(std::istream& data, std::string owner_name)
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

    } else if (ident == "signal:") {
      std::getline(data, signal);
      signal = no_caps(signal);
      signal = trim(signal);
      if (signal.empty()) {
        debugmsg("Empty signal (%s)", owner_name.c_str());
        return false;
      }

    } else if (ident == "special:") {
      std::string special_name;
      std::getline(data, special_name);
      special_name = no_caps(special_name);
      special_name = trim(special_name);
      if (special_name == "explosion") {
        special = new Tool_special_explosion;
      } else if (special_name == "light") {
        special = new Tool_special_light;
      } else if (special_name == "heal") {
        special = new Tool_special_heal;
      } else {
        debugmsg("Unknown Tool_special '%s' (%s)", special_name.c_str(),
                 owner_name.c_str());
        return false;
      }
      if (!special->load_data(data, owner_name + " tool_action")) {
        delete special;
        special = NULL;
        return false;
      }

    } else if (ident == "target:") {
      std::string target_name;
      std::getline(data, target_name);
      target_name = trim(target_name);
      target = lookup_tool_target(target_name);
      if (target == TOOL_TARGET_NULL) {
        debugmsg("Unknown tool target '%s' (%s)", target_name.c_str(),
                 owner_name.c_str());
        return false;
      }

    } else if (ident == "field:") {
      if (!field.load_data(data, owner_name + " action field")) {
        debugmsg("Failed to load field (%s)", owner_name.c_str());
        return false;
      }

    } else if (ident == "ap_cost:") {
      data >> ap_cost;
      std::getline(data, junk);

    } else if (ident == "charge_cost:") {
      data >> charge_cost;
      std::getline(data, junk);

    } else if (ident == "range:") {
      data >> range;
      std::getline(data, junk);

    } else if (ident == "destroy" || ident == "destroy_if_chargeless") {
      destroy_if_chargeless = true;

    } else if (ident != "done") {
      debugmsg("Unknown Tool_action property '%s' (%s)", ident.c_str(),
               owner_name.c_str());
      return false;
    }
  }
  real = true;
  return true;
}

bool Tool_action::activate(Item* it)
{
  if (!it) {
    return false;
  }
  return activate(it, NULL);
}

bool Tool_action::activate(Item* it, Entity* user)
{
  if (!it) {
    return false;
  }
  Tripoint pos = GAME.find_item_uid(it->get_uid());
  if (pos.x == -1) {  // Couldn't find item!
    debugmsg("Couldn't find item.");
    return false;
  }
  return activate(it, user, pos);
}

bool Tool_action::activate(Item* it, Entity* user, Tripoint pos)
{
  bool seen_by_player = GAME.player->can_see(GAME.map, pos);
  if (!seen_by_player && user && user->is_you()) {
    seen_by_player = true;
  }
  bool had_effect = false;  // return value; true if we did anything

// Pick a name for whoever's activating us
  std::string user_name = "Something";  // TODO: "Someone"?
  if (user) {
    user_name = user->get_name_to_player();
  }

  if (!signal.empty()) {
// Pick a verb; TODO - be smarter about this?
    std::string verb = signal;
    if (user) {
      verb = user->conjugate(verb);
    }

    std::string old_name = GAME.map->get_name(pos);
    if (GAME.map->apply_tool_action(signal, pos)) {
      had_effect = true;
      if (seen_by_player) {
        GAME.add_msg("%s %s the %s.", user_name.c_str(), verb.c_str(),
                     old_name.c_str());
      }
    } else if (seen_by_player) {
      GAME.add_msg("%s can't %s there.", user_name.c_str(), signal.c_str());
    }
// TODO: Send signal to monsters and items.
  }

// Generate the field, if any
  if (field.exists()) {
    field.drop(pos, user_name);
    had_effect = true;
  }

// Apply the special, if any
  if (special) {
    had_effect = special->effect(user);
  }

  return had_effect;
}

Tool_target lookup_tool_target(std::string name)
{
  name = no_caps(name);
  name = trim(name);
  for (int i = 0; i < TOOL_TARGET_MAX; i++) {
    Tool_target ret = Tool_target(i);
    if (name == no_caps( tool_target_name(ret) )) {
      return ret;
    }
  }
  return TOOL_TARGET_NULL;
}
    
std::string tool_target_name(Tool_target action)
{
  switch (action) {
    case TOOL_TARGET_NULL:      return "NULL";
    case TOOL_TARGET_ADJACENT:  return "adjacent";
    case TOOL_TARGET_RANGED:    return "ranged";
    case TOOL_TARGET_ALL:       return "all";
    case TOOL_TARGET_MAX:       return "BUG - TOOL_TARGET_MAX";
    default:                    return "BUG - Unnamed Tool_target";
  }
  return "BUG - Escaped tool_target_name switch";
}
