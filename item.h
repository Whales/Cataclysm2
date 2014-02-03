#ifndef _ITEM_H_
#define _ITEM_H_

#include "item_type.h"
#include "enum.h"
#include <string>
#include <vector>

enum Item_action
{
  IACT_NULL = 0,
  IACT_WIELD,
  IACT_WEAR,
  IACT_DROP,
  IACT_MAX
};

class Stats;
class Entity;
struct Ranged_attack;

class Item
{
public:
  Item(Item_type* T = NULL);
  Item(const Item &rhs);
  ~Item();

  Item& operator=(const Item &rhs);

  Item_type* type;
  Item_class get_item_class();
  bool is_real();
  bool can_reload();
  int time_to_reload();

// Info fetching
  int get_uid();

  glyph top_glyph();
  std::string get_data_name();
  std::string get_name();
  std::string get_name_indefinite();
  std::string get_name_definite();
  std::string get_name_full();  // Includes charges, mode, etc.
  std::string get_description();

  int get_weight();
  int get_volume();
  bool has_flag(Item_flag itf);

  int get_damage(Damage_type dtype);
  int get_to_hit();
  int get_base_attack_speed();
  int get_base_attack_speed(Stats stats);

  int get_max_charges();
  bool combines();
  bool combine_by_charges();

  Ranged_attack get_thrown_attack();
  Ranged_attack get_fired_attack();

// Changing
  bool reload(Entity* owner, int ammo_uid);
  bool combine_with(const Item& rhs);
  bool damage(int dam); // Returns true if the item is destroyed

// Interfaces
  Item_action show_info();

  Item_type* ammo;
  int count;
  int charges;
  int hp;
private:
  int uid;
};

std::string list_items(std::vector<Item> *items);

#endif
