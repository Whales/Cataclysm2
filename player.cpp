#include "player.h"
#include "cuss.h"
#include "game.h"
#include "files.h"  // For CUSS_DIR
#include "rng.h"
#include "help.h"   // For help_skill_desc()
#include <sstream>

void populate_item_lists(Player* p, int offset_size,
                         std::vector<int>  item_indices[ITEM_CLASS_MAX],
                         std::vector<char> item_letters[ITEM_CLASS_MAX],
                         std::vector<bool> include_item,
                         std::vector<std::string> &item_name,
                         std::vector<std::string> &item_weight,
                         std::vector<std::string> &item_volume);

Player::Player()
{
  pos.x = 15;
  pos.y = 15;
  action_points = 100;
  name = "Whales";
  male = true;
  profession = NULL;
  for (int i = 0; i < HP_PART_MAX; i++) {
    current_hp[i] = 100;
    max_hp[i] = 100;
  }
}

Player::~Player()
{
}

std::string Player::get_name()
{
  return name;
}

std::string Player::get_name_to_player()
{
  return "you";
}

std::string Player::get_possessive()
{
  return "your";
}

glyph Player::get_glyph()
{
// TODO: Better
  return glyph('@', c_white, c_black);
}

int Player::get_genus_uid()
{
  return -3;
}

void Player::prep_new_character()
{
  Item_group* clothes_group = ITEM_GROUPS.lookup_name("items_starting_clothes");
  std::vector<Item_type*> clothes = clothes_group->get_all_item_types();
  for (int i = 0; i < clothes.size(); i++) {
    Item tmp( clothes[i] );
    items_worn.push_back(tmp);
  }
}

void Player::set_profession(Profession* prof)
{
  profession = prof;
}

Profession* Player::get_profession()
{
  return profession;
}

void Player::gain_experience(int amount)
{
  if (amount <= 0) {
    return;
  }

// Display a message when we unlock improvements
  std::vector<bool> could_improve;
  for (int i = 0; i < SKILL_MAX; i++) {
    Skill_type sk = Skill_type(i);
    int cost = skills.improve_cost(sk);
    if (skills.maxed_out(sk) && has_trait(TRAIT_AUTODIDACT)) {
      cost *= 2;
    }
    bool maxed = (skills.maxed_out(sk) && !has_trait(TRAIT_AUTODIDACT));
    if (experience >= cost && !maxed) {
      could_improve.push_back(true);
    } else {
      could_improve.push_back(false);
    }
  }

  experience += amount;
  GAME.add_msg("You gain %d experience!", amount);

  std::vector<Skill_type> unlocked_skills;

  for (int i = 0; i < SKILL_MAX; i++) {
    Skill_type sk = Skill_type(i);
    int cost = skills.improve_cost(sk);
    if (skills.maxed_out(sk) && has_trait(TRAIT_AUTODIDACT)) {
      cost *= 2;
    }
    bool maxed = (skills.maxed_out(sk) && !has_trait(TRAIT_AUTODIDACT));
    if (i > 0 && !could_improve[i] && experience >= cost && !maxed) {
      unlocked_skills.push_back(sk);
    }
  }

  if (unlocked_skills.size() > 3) {
    GAME.add_msg("You can improve %d new skills!", unlocked_skills.size());
  } else if (!unlocked_skills.empty()) {
    std::stringstream mes;
    mes << "You can now improve ";
    for (int i = 0; i < unlocked_skills.size(); i++) {
      mes << skill_type_user_name( unlocked_skills[i] );
      if (i == unlocked_skills.size() - 1) {
        mes << "!";
      } else if (i == unlocked_skills.size() - 2) {
        mes << " and ";
      } else {
        mes << ", ";
      }
    }
    GAME.add_msg(mes.str());
  }
}

bool Player::has_sense(Sense_type sense)
{
// TODO: Turn off senses if we're blinded, deafened, etc.
  switch (sense) {
    case SENSE_SIGHT:
    case SENSE_SOUND:
      return true;
    default:
      return false;
  }
  return false;
}

int Player::get_movement_cost()
{
// TODO: If we have the "Fleet-Footed" perk, then start ret at 90
  int ret = 100;

// Add delay if we're over our weight limit
  int wgt = current_weight(), wgt_max = maximum_weight();
  if (wgt > wgt_max) {
    int diff = wgt - wgt_max;
    ret += (diff * 2) / stats.strength;
  }

// Add delay for dragging furniture
  int furniture_weight = 0;
  for (int i = 0; i < dragged.size(); i++) {
    furniture_weight += dragged[i].furniture.get_weight();
  }

// We can drag 2 pounds per unit of strength, without penalty
  int min_drag = stats.strength * 20;
  if (furniture_weight > min_drag) {
// Every (strength^2 / 100) pounds above that results in an extra point of delay
    furniture_weight -= min_drag;
    ret += (10 * furniture_weight) / (stats.strength * stats.strength);
  }

// TODO: Other stuff that slows down our run speed?
  return ret;
}

// This function handles messages and yes/no prompts, too!
bool Player::add_item(Item item)
{
// TODO: Weight isn't a hard limit
  if (current_weight() + item.get_weight() > maximum_weight()) {
    GAME.add_msg("You cannot carry that much weight.");
    return false;
  }
/* TODO: add_item() gets called when taking off an item.  If there's no room in
 *       our inventory for the item... we'll ask the player if they want to put
 *       it on.  Not great!
 */
  if (current_volume() + item.get_volume() > maximum_volume()) {
    if (item.get_item_class() == ITEM_CLASS_CLOTHING) {
      if (query_yn("No room for that %s.  Put on now?",
                   item.get_name().c_str())) {
        inventory.push_back(item);
        wear_item_uid(item.get_uid());
        GAME.add_msg( wear_item_message(item) );
        return true;
      } else {
        return false;
      }
    } else {
// If it's not clothing, our only option is to wield it...
      std::stringstream wield_prompt;
      bool sheath_weap = false;
      if (weapon.is_real()) {
        if (current_volume() + weapon.get_volume() <= maximum_volume()) {
          sheath_weap = true;
        }
        wield_prompt << (sheath_weap ? "Put away " : "Drop ") << "your " <<
                        weapon.get_name() << " and w";
      } else {
        wield_prompt << "W";
      }
      wield_prompt << "ield that " << item.get_name() << "?";
      if (query_yn(wield_prompt.str().c_str())) {
        if (sheath_weap) {
          GAME.add_msg( sheath_weapon_message() );
          sheath_weapon();
        } else if (weapon.is_real()) {
          GAME.add_msg( drop_item_message(weapon) );
          GAME.map->add_item( weapon, pos );
          remove_item_uid( weapon.get_uid() );
        }
        inventory.push_back(item);
        GAME.add_msg( wield_item_message(item) );
        wield_item_uid( item.get_uid() );
        return true;
      } else {
        return false;
      }
    } // End of non-clothing handling block
  } // End of "too much volume" block
  GAME.add_msg("You pick up %s.", item.get_name_indefinite().c_str());
  if (item.combines()) {
    Item* added = ref_item_of_type(item.type);
    if (added) {
      return (*added).combine_with(item);
    }
  }
  inventory.push_back(item);
  return true;
}

int Player::current_weight()
{
  int ret = 0;
  ret += weapon.get_weight();
  for (int i = 0; i < inventory.size(); i++) {
    ret += inventory[i].get_weight();
  }
  for (int i = 0; i < items_worn.size(); i++) {
    ret += items_worn[i].get_weight();
  }
  return ret;
}

int Player::maximum_weight()
{
// 25 pounds, plus 10 per point of strength; 10 strength = 125 lbs
  int ret = 250 + 100 * stats.strength;
  if (has_trait(TRAIT_BAD_BACK)) {
    ret = ret * .8;
  }
  return ret;
}

int Player::current_volume()
{
  int ret = 0;
  for (int i = 0; i < inventory.size(); i++) {
    ret += inventory[i].get_volume();
  }
  return ret;
}

int Player::maximum_volume()
{
  int ret = 0;
  for (int i = 0; i < items_worn.size(); i++) {
    ret += items_worn[i].get_volume_capacity();
  }
  if (has_trait(TRAIT_PACKMULE)) {
    ret *= 1.2;
  }
  return ret;
}

Item Player::inventory_single()
{
  std::vector<Item> items = inventory_ui(true, false);
  if (items.empty()) {
    return Item();
  }
  return items[0];
}

std::vector<Item> Player::drop_items()
{
  std::vector<Item> ret = inventory_ui(false, true);
  return ret;
}

std::vector<Item> Player::inventory_ui(bool single, bool remove)
{
  Window w_inv(0, 0, 80, 24);
  cuss::interface i_inv;
  std::string inv_file = CUSS_DIR + "/i_inventory.cuss";
// Sanity checks
  if (!i_inv.load_from_file(inv_file)) {
    return std::vector<Item>();
  }
  cuss::element *ele_list_items = i_inv.find_by_name("list_items");
  if (ele_list_items == NULL) {
    debugmsg("No element 'list_items' in %s", inv_file.c_str());
    return std::vector<Item>();
  }
  int offset_size = ele_list_items->sizey;
// Set static text fields, which are different depending on single/remove
// So, we have a vector of indices for each item category.
  bool include_weapon = false;

// Set up letter for weapon, if any exists
  char letter = 'a';
  char weapon_letter = 0;
  if (weapon.type) {
    weapon_letter = 'a';
    letter = 'b';
    std::stringstream weapon_ss;
    weapon_ss << weapon_letter << " - " << weapon.get_name_full();
    i_inv.set_data("text_weapon", weapon_ss.str());
  } else {
    i_inv.set_data("text_weapon", "<c=dkgray>No weapon<c=/>");
  }
// Start with clothing - it's simple!
  std::vector<char> clothing_letters;
  std::vector<bool> include_clothing;
  std::vector<std::string> clothing_name, clothing_weight, clothing_volume;

  for (int i = 0; i < items_worn.size(); i++) {
    include_clothing.push_back(false);

    std::stringstream clothing_ss;
    Item_type_clothing *clothing =
      static_cast<Item_type_clothing*>(items_worn[i].type);

    clothing_ss << letter << " - " << items_worn[i].get_name_full();
    clothing_name.push_back(clothing_ss.str());

    clothing_weight.push_back( itos( items_worn[i].get_weight() ) );
      
    int capacity = clothing->carry_capacity;
    if (capacity == 0) {
      clothing_volume.push_back("<c=dkgray>+0<c=/>");
    } else {
      std::stringstream volume_ss;
      volume_ss << "<c=green>+" << capacity << "<c=/>";
      clothing_volume.push_back(volume_ss.str());
    }

    clothing_letters.push_back(letter);
    if (letter == 'z') {
      letter = 'A';
    } else if (letter == 'Z') {
      letter = '!';
    } else {
      letter++;
    }
  }
// Populate those vectors!
  std::vector<int>  item_indices[ITEM_CLASS_MAX];
  std::vector<char> item_letters[ITEM_CLASS_MAX];
  std::vector<bool> include_item;
  for (int i = 0; i < inventory.size(); i++) {
    include_item.push_back(false);
    Item_class iclass = inventory[i].get_item_class();
    item_indices[iclass].push_back(i);
    item_letters[iclass].push_back(letter);
// TODO: Better inventory letters.  This still isn't unlimited.
    if (letter == 'z') {
      letter = 'A';
    } else if (letter == 'Z') {
      letter = '!';
    } else {
      letter++;
    }
  }
// Now, populate the string lists
  std::vector<std::string> item_name, item_weight, item_volume;
  populate_item_lists(this, offset_size, item_indices, item_letters,
                      include_item, item_name, item_weight, item_volume);

// Set interface data
  i_inv.set_data("weight_current", current_weight());
  i_inv.set_data("weight_maximum", maximum_weight());
  i_inv.set_data("volume_current", current_volume());
  i_inv.set_data("volume_maximum", maximum_volume());
  if (single) {
    i_inv.set_data("text_instructions", "\
<c=magenta>Press Esc to cancel.\nPress - to select nothing.<c=/>");
  } else {
    i_inv.set_data("text_instructions", "\
<c=magenta>Press Esc to cancel.\nPress Enter to confirm selection.<c=/>");
    if (remove) {
      i_inv.set_data("text_after", "<c=brown>After:<c=/>");
      i_inv.set_data("text_after2", "<c=brown>After:<c=/>");
    }
  }
  for (int i = 0; i < offset_size && i < item_name.size(); i++) {
    i_inv.add_data("list_items",  item_name[i]);
    i_inv.add_data("list_weight", item_weight[i]);
    i_inv.add_data("list_volume", item_volume[i]);
  }
  for (int i = 0; i < offset_size && i < clothing_name.size(); i++) {
    i_inv.add_data("list_clothing", clothing_name[i]);
    i_inv.add_data("list_clothing_weight", clothing_weight[i]);
    i_inv.add_data("list_clothing_volume", clothing_volume[i]);
  }

  int offset = 0;
  int weight_after = current_weight(), volume_after = current_volume();
  bool done = false;

  while (!done) {
    if (!single && remove) {
      i_inv.set_data("weight_after", weight_after);
      i_inv.set_data("volume_after", volume_after);
    }
    i_inv.draw(&w_inv);
    long ch = input();
    if (single && ch == '-') {
      std::vector<Item> ret;
      ret.push_back(Item());
      return ret;
    } else if (ch == '<' && offset > 0) {
      offset--;
      i_inv.clear_data("list_items");
      i_inv.clear_data("list_weight");
      i_inv.clear_data("list_volume");
      for (int i = offset * offset_size;
           i < (offset + 1) * offset_size && i < item_name.size();
           i++) {
        i_inv.add_data("list_items",  item_name[i]);
        i_inv.add_data("list_weight", item_weight[i]);
        i_inv.add_data("list_volume", item_volume[i]);
      }
    } else if (ch == '>' && item_name.size() > (offset + 1) * offset_size) {
      offset++;
      i_inv.clear_data("list_items");
      i_inv.clear_data("list_weight");
      i_inv.clear_data("list_volume");
      for (int i = offset * offset_size;
           i < (offset + 1) * offset_size && i < item_name.size();
           i++) {
        i_inv.add_data("list_items",  item_name[i]);
        i_inv.add_data("list_weight", item_weight[i]);
        i_inv.add_data("list_volume", item_volume[i]);
      }
    } else if (ch == KEY_ESC) {
      std::vector<Item> empty;
      return empty;
    } else if (ch == '\n') {
      done = true;
    } else { // Anything else warrants a check for the matching key!
      bool found = false;
      if (ch == weapon_letter) {
        found = true;
        include_weapon = !include_weapon;
        std::stringstream weapon_ss;
        weapon_ss << (include_weapon ? "<c=green>" : "<c=ltgray>") << 
                     weapon_letter << (include_weapon ? " + " : " - ") <<
                     weapon.get_name_full();
        i_inv.set_data("text_weapon", weapon_ss.str());
      }
      if (!found) {
        for (int i = 0; i < clothing_letters.size(); i++) {
          if (ch == clothing_letters[i]) {
            found = true;
            include_clothing[i] = !include_clothing[i];
            bool inc = include_clothing[i];
            std::stringstream clothing_ss;
            clothing_ss << (inc ? "<c=green>" : "<c=ltgray>") <<
                           clothing_letters[i] << (inc ? " + " : " - ") <<
                           items_worn[i].get_name_full();
            clothing_name[i] = clothing_ss.str();
          }
        }
      }
      if (!found) { // Not the weapon, not clothing - let's check inventory
        for (int n = 0; n < ITEM_CLASS_MAX; n++) {
          for (int i = 0; i < item_letters[n].size(); i++) {
            if (ch == item_letters[n][i]) {
              found = true;
              int index = item_indices[n][i];
              include_item[index] = !include_item[index];
              bool inc = include_item[index];
              std::stringstream item_ss;
              item_ss << (inc ? "<c=green>" : "<c=ltgray>") <<
                         item_letters[n][i] << (inc ? " + " : " - ") <<
                         inventory[index].get_name_full();
// It's easiest to just set up the text lists for items from scratch!
              populate_item_lists(this, offset_size, item_indices, item_letters,
                                  include_item, item_name, item_weight,
                                  item_volume);
            }
          }
        }
      }
      if (found) {
        if (single) {
          done = true;
        }
// Need to refresh our lists!
        i_inv.clear_data("list_items");
        i_inv.clear_data("list_weight");
        i_inv.clear_data("list_volume");
        for (int i = offset * offset_size;
             i < (offset + 1) * offset_size && i < item_name.size();
             i++) {
          i_inv.add_data("list_items",  item_name[i]);
          i_inv.add_data("list_weight", item_weight[i]);
          i_inv.add_data("list_volume", item_volume[i]);
        }
        i_inv.set_data("list_clothing", clothing_name);
      }
    } // Last check for ch
  } // while (!done)

/* If we reach this point, either we're in single-mode and we've selected an
 * item, or we're in multiple mode and we've hit Enter - either with some items
 * items selected or without.
 * Things set at this point:
 * include_weapon - a bool marked true if we selected our weapon
 * include_item - a set of bools, true if the item with that index is selected
 * include_clothing - like include_item but for items_worn
 */
  std::vector<Item> ret;
  if (include_weapon) {
    ret.push_back(weapon);
  }
  for (int i = 0; i < include_item.size(); i++) {
    if (include_item[i]) {
      ret.push_back( inventory[i] );
    }
  }
  for (int i = 0; i < include_clothing.size(); i++) {
    if (include_clothing[i]) {
      ret.push_back( items_worn[i] );
    }
  }

  if (remove) {
    if (include_weapon) {
      weapon = Item();
    }
/* We go through these vectors backwards - to guarantee that the indices remain
 * valid, even after removing items!
 */
    for (int i = include_item.size() - 1; i >= 0; i--) {
      if (include_item[i]) {
        inventory.erase(inventory.begin() + i);
      }
    }
    for (int i = include_clothing.size() - 1; i >= 0; i--) {
      if (include_clothing[i]) {
        items_worn.erase(items_worn.begin() + i);
      }
    }

  }

  return ret;
}


Item Player::pick_ammo_for(Item *it)
{
  if (!it || !it->can_reload()) {
    return Item();
  } 
  if (it->charges == it->get_max_charges()) {
    return Item();
  }
  if (it->get_item_class() != ITEM_CLASS_LAUNCHER) {
    GAME.add_msg("You cannot reload %s.", it->get_name_indefinite().c_str());
    return Item();
  }
  if (it->charges > 0 && it->ammo) {
    return get_item_of_type(it->ammo);
  }
// TODO: Limit this to valid ammo slots
  Item ret = inventory_single();
  if (!ret.is_real()) {
    GAME.add_msg("Never mind.");
    return Item();
  }
  if (ret.get_item_class() != ITEM_CLASS_AMMO) {
    GAME.add_msg("That %s is not ammo.", ret.get_name().c_str());
    return Item();
  }
  Item_type_ammo* ammo = static_cast<Item_type_ammo*>(ret.type);
  Item_type_launcher* launcher = static_cast<Item_type_launcher*>(it->type);
  if (ammo->ammo_type != launcher->ammo_type) {
    GAME.add_msg("You picked %s ammo, but your %s needs %s.",
                 ammo->ammo_type.c_str(), it->get_name().c_str(),
                 launcher->ammo_type.c_str());
    return Item();
  }
  return ret;
}

Tripoint Player::pick_target_for(Item* it)
{
  if (!it || it->get_item_class() != ITEM_CLASS_TOOL) {
    return Tripoint(-1, -1, -1);
  }

  Item_type_tool* tool = static_cast<Item_type_tool*>(it->type);
  Tool_action* action = &(tool->applied_action);
  std::string verb = action->signal;

  Tripoint ret = pos;

  switch (action->target) {

    case TOOL_TARGET_NULL:
// No need to do anything
      break;

    case TOOL_TARGET_ADJACENT: {
      GAME.add_msg("<c=ltgreen>%s where? (Press direction key)<c=/>",
                   verb.c_str());
      GAME.draw_all();
      Point dir = input_direction(input());
      if (dir.x == -2) {  // We canceled
        return Tripoint(-1, -1, -1);
      }
      ret.x += dir.x;
      ret.y += dir.y;
     } break;

    case TOOL_TARGET_RANGED: {
      int range = action->range;
      if (range <= 0) {
        range = -1; // -1 means "No range limit" for Game::target_selector()
      }
      ret = GAME.target_selector(pos.x, pos.y, range);
      if (ret.x == -1) { // We canceled
        return Tripoint(-1, -1, -1);
      }
    } break;

    default:
      debugmsg("Don't know how to handle Tool_target %s",
               tool_target_name(action->target).c_str());
      return Tripoint(-1, -1, -1);
  }

  return ret;
}

void Player::take_damage(Damage_type damtype, int damage, std::string reason,
                         Body_part part)
{
  if (damage <= 0) {
    return;
  }
  absorb_damage(damtype, damage, part);
  take_damage_no_armor(damtype, damage, reason, part);
}


void Player::take_damage_no_armor(Damage_type damtype, int damage,
                                  std::string reason, Body_part part)
{
  current_hp[ convert_to_HP(part) ] -= damage;
  int min_pain = damage / 2, max_pain = damage;
  if (has_trait(TRAIT_PAIN_RESISTANT)) {
    min_pain /= 2;
    max_pain = max_pain * .75;
  }
  pain += rng(damage / 2, damage);
}

void Player::take_damage_everywhere(Damage_set damage, std::string reason)
{
  for (int i = 0; i < DAMAGE_MAX; i++) {
    Damage_type type = Damage_type(i);
    int dam = damage.get_damage(type);
    take_damage_everywhere(type, dam, reason);
  }
}

void Player::take_damage_everywhere(Damage_type type, int damage,
                                    std::string reason)
{
  take_damage(type, (damage * 3) / 6, reason, BODY_PART_HEAD);
  take_damage(type, (damage * 1) / 6, reason, BODY_PART_EYES);
  take_damage(type, (damage * 2) / 6, reason, BODY_PART_MOUTH);

  take_damage(type, damage          , reason, BODY_PART_TORSO);

  take_damage(type, (damage * 1) / 6, reason, BODY_PART_LEFT_HAND);
  take_damage(type, (damage * 1) / 6, reason, BODY_PART_RIGHT_HAND);
  take_damage(type, (damage * 2) / 6, reason, BODY_PART_LEFT_ARM);
  take_damage(type, (damage * 2) / 6, reason, BODY_PART_RIGHT_ARM);

  take_damage(type, (damage * 1) / 8, reason, BODY_PART_LEFT_FOOT);
  take_damage(type, (damage * 1) / 8, reason, BODY_PART_RIGHT_FOOT);
  take_damage(type, (damage * 3) / 8, reason, BODY_PART_LEFT_LEG);
  take_damage(type, (damage * 3) / 8, reason, BODY_PART_RIGHT_LEG);
}

void Player::heal_damage(int damage, HP_part part)
{
  current_hp[part] += damage;
  if (current_hp[part] > max_hp[part]) {
    current_hp[part] = max_hp[part];
  }
}

int Player::get_armor(Damage_type damtype, Body_part part)
{
  int ret = 0;
  for (int i = 0; i < items_worn.size(); i++) {
    if (items_worn[i].covers(part)) {
      Item_type* type = items_worn[i].type;
      Item_type_clothing* clothing = static_cast<Item_type_clothing*>(type);
      //debugmsg("%s covers %s and provides %d:%d:%d", items_worn[i].get_name().c_str(), body_part_name(part).c_str(), clothing->armor_bash, clothing->armor_cut, clothing->armor_pierce);
      switch (damtype) {
        case DAMAGE_BASH:
          ret += clothing->armor_bash;
          break;
        case DAMAGE_CUT:
          ret += clothing->armor_cut;
          break;
        case DAMAGE_PIERCE:
          ret += clothing->armor_pierce;
          break;
      }
    }
  }
  return ret;
}

int Player::get_protection(Body_part part)
{
  int ret = 0;
  for (int i = 0; i < items_worn.size(); i++) {
    if (items_worn[i].covers(part)) {
      Item_type* type = items_worn[i].type;
      Item_type_clothing* clothing = static_cast<Item_type_clothing*>(type);
      ret += clothing->protection;
    }
  }
  return ret;
}

// TODO: This function.  Once we have morale.
int Player::personal_mission_cap()
{
  return 3;
}

std::string Player::hp_text(Body_part part)
{
  return hp_text( convert_to_HP(part) );
}

std::string Player::hp_text(HP_part part)
{
// Sanity check
  if (part == HP_PART_MAX) {
    return "BP_MAX???";
  }
  std::stringstream ret;
  if (current_hp[part] == max_hp[part]) {
    ret << "<c=green>";
  } else {
    int percent = (100 * current_hp[part]) / max_hp[part];
    if (percent >= 75) {
      ret << "<c=ltgreen>";
    } else if (percent >= 50) {
      ret << "<c=yellow>";
    } else if (percent >= 25) {
      ret << "<c=ltred>";
    } else {
      ret << "<c=red>";
    }
  }
  ret << current_hp[part] << "<c=/>";
  return ret.str();
}

void Player::status_interface()
{
  cuss::interface i_status;
  std::string iface_file = CUSS_DIR + "/i_char_status.cuss";
  if (!i_status.load_from_file(iface_file)) {
    return;
  }

  Window w_status(0, 0, 80, 24);

  i_status.set_data("num_str_cur", stats.strength        );
  i_status.set_data("num_str_max", stats.strength_max    );
  i_status.set_data("num_str_max", c_white);
  if (stats.strength < stats.strength_max) {
    i_status.set_data("num_str_cur", c_ltred);
  } else if (stats.strength > stats.strength_max) {
    i_status.set_data("num_str_cur", c_ltgreen);
  }

  i_status.set_data("num_dex_cur", stats.dexterity       );
  i_status.set_data("num_dex_max", stats.dexterity_max   );
  i_status.set_data("num_dex_max", c_white);
  if (stats.dexterity < stats.dexterity_max) {
    i_status.set_data("num_dex_cur", c_ltred);
  } else if (stats.dexterity > stats.dexterity_max) {
    i_status.set_data("num_dex_cur", c_ltgreen);
  }

  i_status.set_data("num_per_cur", stats.perception      );
  i_status.set_data("num_per_max", stats.perception_max  );
  i_status.set_data("num_per_max", c_white);
  if (stats.perception < stats.perception_max) {
    i_status.set_data("num_per_cur", c_ltred);
  } else if (stats.perception > stats.perception_max) {
    i_status.set_data("num_per_cur", c_ltgreen);
  }

  i_status.set_data("num_int_cur", stats.intelligence    );
  i_status.set_data("num_int_max", stats.intelligence_max);
  i_status.set_data("num_int_max", c_white);
  if (stats.intelligence < stats.intelligence_max) {
    i_status.set_data("num_int_cur", c_ltred);
  } else if (stats.intelligence > stats.intelligence_max) {
    i_status.set_data("num_int_cur", c_ltgreen);
  }

  i_status.set_data("num_morale", get_morale());

  i_status.set_data("num_speed",   get_speed()           );

  std::map<std::string,int> speed_mods = get_speed_modifiers();
  for (std::map<std::string,int>::iterator it = speed_mods.begin();
       it != speed_mods.end();
       it++) {
    std::stringstream mod_name, mod_amount;
    if (it->second < 0) {
      mod_name   << "<c=ltred>";
      mod_amount << "<c=ltred>";
    } else {
      mod_name   << "<c=ltgreen>";
      mod_amount << "<c=ltgreen>+";
    }
    mod_name   << it->first  << "<c=/>";
    mod_amount << it->second << "<c=/>";
    i_status.add_data("list_speed_mod_names",   mod_name.str()  );
    i_status.add_data("list_speed_mod_amounts", mod_amount.str());
  }

  for (int i = 0; i < effects.size(); i++) {
    i_status.add_data("list_status_effects", effects[i].get_display_name());
  }

  std::vector<Trait_id> trait_list;
  for (int i = 0; i < TRAIT_MAX; i++) {
    Trait_id id = Trait_id(i);
    if (has_trait(id)) {
      trait_list.push_back(id);
      std::stringstream ss_trait_name;
      if (id < TRAIT_MAX_GOOD) {
        ss_trait_name << "<c=ltgreen>";
      } else if (id < TRAIT_MAX_NEUTRAL) {
        ss_trait_name << "<c=yellow>";
      } else if (id < TRAIT_MAX_BAD) {
        ss_trait_name << "<c=ltred>";
      }
      ss_trait_name << trait_id_name(id) << "<c=/>";
      i_status.add_data("list_traits", ss_trait_name.str());
    }
  }

  i_status.select("list_status_effects");
  if (!effects.empty()) {
    i_status.set_data("text_description", effects[0].get_description());
  }

// Okay!  All done filling out the interface.
  while (true) {
    i_status.draw(&w_status);
    w_status.refresh();

    std::string selected_name = i_status.selected()->name;
    int last_selected = i_status.selected()->get_int();

    long ch = input();

    if (ch == KEY_ESC || ch == 'q' || ch == 'Q') {
      return;

    } else if (ch == '2') {
      skills_interface();
      return;

    } else if (ch == '3') {
      clothing_interface();
      return;

    } else if (ch == '4') {
// TODO: Put link to quests_screen() here
      return;

    } else {
      i_status.handle_action(ch);
      if (i_status.selected()->name != selected_name ||
          i_status.selected()->get_int() != last_selected) {
        if (i_status.selected()->name == "list_status_effects") {
          int sel = i_status.selected()->get_int();
          if (!effects.empty() && sel < effects.size()) {
            i_status.set_data("text_description",
                              effects[sel].get_description());
          }
        } else {
          int sel = i_status.selected()->get_int();
          if (!trait_list.empty() && sel < trait_list.size()) {
            i_status.set_data("text_description",
                             trait_description(trait_list[sel]));
          }
        }
      }
    }
  } // while (true)
}

void Player::skills_interface()
{
  cuss::interface i_skills;
  std::string iface_file = CUSS_DIR + "/i_char_skills.cuss";
  if (!i_skills.load_from_file(iface_file)) {
    return;
  }

  Window w_skills(0, 0, 80, 24);

  setup_skills_interface(i_skills);

  i_skills.draw(&w_skills);
  w_skills.refresh();

  bool help_mode = false;

  while (true) {
    long ch = input();

    if (ch == KEY_ESC) {
      return;

    } else if (ch == '1') {
      status_interface();
      return;

    } else if (ch == '3') {
      clothing_interface();
      return;

    } else if (ch == '4') {
// TODO: Put link to quests_screen() here
      return;

    } else if (ch == '?') {
      if (help_mode) {
        help_screen("skills.txt");
      }
      help_mode = !help_mode;

      if (help_mode) {
        i_skills.set_data("text_help", "\
<c=yellow>Press the letter attached to a skill for information on that skill.\
\n\n\
Press <c=pink>?<c=yellow> again for general help on skills.<c=/>");
      } else {
        i_skills.set_data("text_help", "\
<c=yellow>Press the letter attached to a skill to improve it.  You cannot \
improve a skill beyond its cap.\n\n\
Press <c=pink>?<c=yellow> and then a skill letter to get help.<c=/>");
      }
      i_skills.draw(&w_skills);
      w_skills.refresh();


    } else if ( (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ) {
// We picked a skill!
      int sk_num = ch;
      if (sk_num >= 'a' && sk_num <= 'z') {
        sk_num = sk_num - 'a' + 1;
      } else {
        sk_num = sk_num - 'Z' + 27;
      }
      Skill_type sk = Skill_type(sk_num);

      if (help_mode) {
        help_mode = false;
        help_skill_desc(sk);
      } else {
        int cost = skills.improve_cost(sk);
        if (skills.maxed_out(sk) && has_trait(TRAIT_AUTODIDACT)) {
          cost *= 2;
        }
        if (skills.maxed_out(sk) && !has_trait(TRAIT_AUTODIDACT)) {
          popup("That skill has hit its cap!");
        } else if (experience >= cost) {
          if (query_yn("Spend %d XP to increase %s to %d?", cost,
                       skill_type_user_name(sk).c_str(),
                       skills.get_level(sk) + 1)) {
            experience -= cost;
            skills.increase_level(sk);
          }
        } else {
          popup("You do not have enough XP! (You: %d; %d required)",
                experience, cost);
        }
      }

      setup_skills_interface(i_skills);
      i_skills.draw(&w_skills);
      w_skills.refresh();

    } // End of "ch is a letter"

  } // while (true)
}

void Player::setup_skills_interface(cuss::interface& i_skills)
{
  std::string cur_name  = "list_name_1",
              cur_level = "list_level_1",
              cur_max   = "list_max_1",
              cur_cost  = "list_cost_1";

  i_skills.clear_data("list_name_1");
  i_skills.clear_data("list_level_1");
  i_skills.clear_data("list_max_1");
  i_skills.clear_data("list_cost_1");
  i_skills.clear_data("list_name_2");
  i_skills.clear_data("list_level_2");
  i_skills.clear_data("list_max_2");
  i_skills.clear_data("list_cost_2");

  int list_size = i_skills.element_height(cur_name);
  if (list_size == -1) {
    debugmsg("i_char_skills.cuss is missing element 'list_name_1'!");
    return;
  }

// Set up the lists.
  for (int i = 1; i < SKILL_MAX; i++) {

    Skill_type sk = Skill_type(i);
    int cost = skills.improve_cost(sk);
    if (skills.maxed_out(sk) && has_trait(TRAIT_AUTODIDACT)) {
      cost *= 2;
    }
    bool maxed = (skills.maxed_out(sk) && !has_trait(TRAIT_AUTODIDACT));

    std::stringstream sk_name, sk_level, sk_max, sk_cost;
    sk_name << "<c=pink>";
    if (i <= 26) {
      sk_name << char( i - 1 + 'a' );
    } else {
      sk_name << char( i - 27 + 'A' );
    }
    sk_name << "<c=/>: ";
    if (!maxed && experience >= cost) {
      sk_name << "<c=white>";
    }
    sk_name << skill_type_user_name(sk) << "<c=/>";
    i_skills.add_data(cur_name, sk_name.str());

    if (skills.get_level(sk) == 0) {
      sk_level << "<c=dkgray>";
    } else if (maxed) {
      sk_level << "<c=red>";
      sk_max   << "<c=red>";
    }
    sk_level << skills.get_level(sk) << "<c=/> /";
    i_skills.add_data(cur_level, sk_level.str());

    if (skills.is_unlocked(sk)) {
      sk_max << "<c=dkgray>--<c=/>";
    } else {
      sk_max << skills.get_max_level(sk) << "<c=/>";
    }
    i_skills.add_data(cur_max, sk_max.str());

    if (experience >= cost) {
      sk_cost << "<c=white>";
    } else {
      sk_cost << "<c=dkgray>";
    }
    sk_cost << cost << "<c=/>";
    i_skills.add_data(cur_cost, sk_cost.str());

// Switch to the next list if needed
    if (i == list_size) {
      cur_name  = "list_name_2";
      cur_level = "list_level_2";
      cur_max   = "list_max_2";
      cur_cost  = "list_cost_2";
    }
  }

  i_skills.set_data("num_experience", experience);

  i_skills.set_data("text_help", "\
<c=yellow>Press the letter attached to a skill to improve it.  You cannot \
improve a skill beyond its cap.\n\n\
Press <c=pink>?<c=yellow> and then a skill letter to get help.<c=/>");

}

void Player::clothing_interface()
{
  cuss::interface i_clothing;
  std::string iface_file = CUSS_DIR + "/i_char_clothing.cuss";
  if (!i_clothing.load_from_file(iface_file)) {
    return;
  }

  Window w_clothing(0, 0, 80, 24);

// Init the lists.
  for (int i = 1; i < BODY_PART_MAX; i++) {
    Body_part bp = Body_part(i);
    i_clothing.add_data("list_part_name", body_part_name(bp));
    std::stringstream ss_bash, ss_cut, ss_pierce, ss_protection;
    int armor_bash   = get_armor(DAMAGE_BASH,   bp);
    int armor_cut    = get_armor(DAMAGE_CUT,    bp);
    int armor_pierce = get_armor(DAMAGE_PIERCE, bp);
    if (armor_bash == 0) {
      ss_bash << "<c=dkgray>";
    } else if (armor_bash >= 8) {
      ss_bash << "<c=white>";
    }
    ss_bash << armor_bash << "<c=/>";
    if (armor_cut == 0) {
      ss_cut << "<c=dkgray>";
    } else if (armor_cut >= 8) {
      ss_cut << "<c=white>";
    }
    ss_cut << armor_cut << "<c=/>";
    if (armor_pierce == 0) {
      ss_pierce << "<c=dkgray>";
    } else if (armor_pierce >= 8) {
      ss_pierce << "<c=white>";
    }
    ss_pierce << armor_pierce << "<c=/>";
    int prot = get_protection(bp);
    if (prot == 0) {
      ss_protection << "<c=dkgray>";
    } else if (prot >= 5) {
      ss_protection << "<c=white>";
    }
    ss_protection << prot << "<c=/>";
    i_clothing.add_data("list_armor_bash",   ss_bash.str()      );
    i_clothing.add_data("list_armor_cut",    ss_cut.str()       );
    i_clothing.add_data("list_armor_pierce", ss_pierce.str()    );
    i_clothing.add_data("list_protection",   ss_protection.str());
  }

  i_clothing.select("list_part_name");

  while (true) {
    Body_part bp = Body_part( i_clothing.get_int("list_part_name") + 1 );
    i_clothing.set_data("text_protection_description",
                        get_protection_meaning(bp));
    i_clothing.draw(&w_clothing);
    w_clothing.refresh();
    long ch = input();
    if (ch == KEY_ESC || ch == 'q' || ch == 'Q') {
      return;

    } else if (ch == '1') {
      status_interface();
      return;

    } else if (ch == '2') {
      skills_interface();
      return;

    } else if (ch == '4') {
// TODO: Put link to quests_screen() here
      return;

    } else {
      i_clothing.handle_keypress(ch);
    }
  }
}


void populate_item_lists(Player* p, int offset_size,
                         std::vector<int>  item_indices[ITEM_CLASS_MAX],
                         std::vector<char> item_letters[ITEM_CLASS_MAX],
                         std::vector<bool> include_item,
                         std::vector<std::string> &item_name,
                         std::vector<std::string> &item_weight,
                         std::vector<std::string> &item_volume)
{
  item_name.clear();
  item_weight.clear();
  item_volume.clear();
  for (int n = 0; n < ITEM_CLASS_MAX; n++) {
    if (!item_indices[n].empty()) {
      std::string class_name = "<c=ltblue>" +
                               item_class_name_plural(Item_class(n)) +
                               "<c=/>";
      item_name.push_back( class_name );
      item_weight.push_back("");
      item_volume.push_back("");
      for (int i = 0; i < item_indices[n].size(); i++) {
// Check to see if we're starting a new page.  If so, repeat the category header
        if (item_name.size() % offset_size == 0) {
          item_name.push_back( class_name + "(cont)" );
          item_weight.push_back("");
          item_volume.push_back("");
        }
        int index = item_indices[n][i];
        Item* item = &( p->inventory[ index ] );
        bool inc = include_item[index];
        std::stringstream item_ss, weight_ss, volume_ss;
        item_ss << (inc ? "<c=green>" : "<c=ltgray>") << item_letters[n][i] <<
                   (inc ? " + " : " - ") << item->get_name_full() << "<c=/>";
        item_name.push_back( item_ss.str() );
        weight_ss << (inc ? "<c=green>" : "<c=ltgray>") << item->get_weight() <<
                     "<c=/>";
        item_weight.push_back( weight_ss.str() );
        volume_ss << (inc ? "<c=green>" : "<c=ltgray>") << item->get_volume() <<
                     "<c=/>";
        item_volume.push_back( volume_ss.str() );
      }
    }
  }
}
