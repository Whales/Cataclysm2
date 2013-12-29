#include "player.h"
#include "cuss.h"
#include <sstream>

void populate_item_lists(Player* p, int offset_size,
                         std::vector<int> item_indices[ITEM_CLASS_MAX],
                         std::vector<char> item_letters[ITEM_CLASS_MAX],
                         std::vector<std::string> &item_name,
                         std::vector<std::string> &item_weight,
                         std::vector<std::string> &item_volume);

Player::Player()
{
  posx = 15;
  posy = 15;
  action_points = 100;
  name = "Whales";
  for (int i = 0; i < BODYPART_MAX; i++) {
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

int Player::get_speed()
{
// TODO: Make this use actual stuff
  return 100;
}

bool Player::can_move_to(Map *map, int x, int y)
{
// TODO: Remove me, obvs
  return true;
}

bool Player::add_item(Item item)
{
// TODO: Weight isn't a hard limit
  if (current_weight() + item.get_weight() > maximum_weight()) {
    return false;
  }
// TODO: Prompt player to wear/wield/etc the item
  if (current_volume() + item.get_volume() > maximum_volume()) {
    return false;
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
// TODO: Base this on Strength or something.
  return 1000;
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
// TODO: Base this on items_worn
  return 1000;
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
// Sanity checks
  if (!i_inv.load_from_file("cuss/i_inventory.cuss")) {
    debugmsg("Couldn't open cuss/i_inventory.cuss!");
    std::vector<Item> ret;
    return ret;
  }
  cuss::element *ele_list_items = i_inv.find_by_name("list_items");
  if (ele_list_items == NULL) {
    debugmsg("No element 'list_items' in cuss/i_inventory.cuss");
    std::vector<Item> ret;
    return ret;
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
    weapon_ss << weapon_letter << " - " << weapon.get_name();
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

    clothing_ss << letter << " - " << items_worn[i].get_name();
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
  populate_item_lists(this, offset_size, item_indices, item_letters, item_name,
                      item_weight, item_volume);

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
    if (ch == '<' && offset > 0) {
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
                     weapon.get_name();
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
                           items_worn[i].get_name();
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
              include_item[index] = true;
              bool inc = include_item[index];
              std::stringstream item_ss;
              item_ss << (inc ? "<c=green>" : "<c=ltgray>") <<
                         item_letters[n][i] << (inc ? " + " : " - ") <<
                         inventory[index].get_name();
// It's easiest to just set up the text lists for items from scratch!
              populate_item_lists(this, offset_size, item_indices, item_letters,
                                  item_name, item_weight, item_volume);
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

void Player::take_damage(Damage_type type, int damage, std::string reason,
                         Body_part part)
{
// TODO: Armor absorbs damage
  current_hp[part] -= damage;
}

std::string Player::hp_text(Body_part part)
{
// Sanity check
  if (part == BODYPART_MAX) {
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

void populate_item_lists(Player* p, int offset_size,
                         std::vector<int> item_indices[ITEM_CLASS_MAX],
                         std::vector<char> item_letters[ITEM_CLASS_MAX],
                         std::vector<std::string> &item_name,
                         std::vector<std::string> &item_weight,
                         std::vector<std::string> &item_volume)
{
  item_name.clear();
  item_weight.clear();
  item_volume.clear();
  for (int n = 0; n < ITEM_CLASS_MAX; n++) {
    if (!item_indices[n].empty()) {
      item_name.push_back( item_class_name( Item_class(n) ) );
      item_weight.push_back("");
      item_volume.push_back("");
      for (int i = 0; i < item_indices[n].size(); i++) {
// Check to see if we're starting a new page.  If so, repeat the category header
        if (item_name.size() % offset_size == 0) {
          item_name.push_back( item_class_name( Item_class(n) ) + "(cont)" );
          item_weight.push_back("");
          item_volume.push_back("");
        }
        int index = item_indices[n][i];
        Item* item = &( p->inventory[ index ] );
        std::stringstream item_ss;
        item_ss << item_letters[n][i] << " - " << item->get_name();
        item_name.push_back( item_ss.str() );
        item_weight.push_back( itos(item->get_weight()) );
        item_volume.push_back( itos(item->get_volume()) );
      }
    }
  }
}
