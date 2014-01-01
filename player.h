#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <string>
#include "entity.h"
#include "map.h"
#include "enum.h"

class Player : public Entity
{
public:
  Player();
  ~Player();

  virtual std::string get_name();
  virtual std::string get_name_to_player();
  virtual std::string get_possessive();
  virtual glyph get_glyph();

  virtual bool is_player()  { return true; };
  virtual bool is_you()     { return true; }; // TODO: No?
  
// Movement functions
  virtual bool has_sense(Sense_type sense);
  virtual int  get_speed();
  virtual bool can_move_to(Map* map, int x, int y);

// Inventory functions
  virtual bool add_item(Item item);
  int current_weight();
  int maximum_weight();
  int current_volume();
  int maximum_volume();

  Item              use_inventory_single();
  Item              inventory_single();
  std::vector<Item> drop_items(); // Provides an interface via inventory()
  std::vector<Item> inventory_ui(bool single = false, bool remove = false);

  Item remove_item_uid(int uid);
  void wield_item_uid(int uid);

// Combat functions
  //virtual Attack base_attack();
  virtual void take_damage(Damage_type type, int damage, std::string reason,
                           Body_part part);

  std::string hp_text(Body_part part);

  int current_hp[BODYPART_MAX];
  int max_hp[BODYPART_MAX];

private:
  std::string name;
};

#endif
