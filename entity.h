#ifndef _ENTITY_H_
#define _ENTITY_H_

#include "glyph.h"
#include "map.h"
#include "item.h"
#include "player_activity.h"
#include "geometry.h"
#include <string>

class Entity
{
public:
  Entity();
  virtual ~Entity();

  virtual std::string get_name();
  virtual std::string get_name_to_player();
  virtual std::string get_possessive();
  virtual glyph get_glyph();
  virtual Point get_position();

  virtual bool is_player()  { return false; }
  virtual bool is_monster() { return false; }
  virtual bool is_you()     { return false; } // As in THE player

  virtual void die();
  virtual void gain_action_points();
  virtual int  get_speed();

  virtual bool has_sense(Sense_type sense);
  virtual bool can_see    (Map* map, int x, int y);
  virtual bool can_move_to(Map* map, int x, int y);
  virtual void move_to    (Map* map, int x, int y);
  virtual void pause();

// Misc action functions
  void set_activity(Player_activity_type type, int duration,
                    int primary_uid = -1, int secondary_uid = -1);
  void use_ap(int amount);
  void shift(int shiftx, int shifty); // For map shifting
  
// Inventory functions
  virtual bool add_item(Item item);
  Item  get_item_of_type(Item_type *type);
  Item* ref_item_of_type(Item_type *type);
  Item* ref_item_uid   (int uid);
  Item  remove_item_uid(int uid, int count = 0);
  void  wield_item_uid (int uid);
  void  wear_item_uid  (int uid);
  void  reload_prep    (int uid);
  virtual Item pick_ammo_for(Item *it);

// Combat functions
  virtual Attack base_attack();
  virtual Attack std_attack(); // With weapon if it exists
  virtual void attack(Entity* target);
  virtual int  hit_roll(int bonus);
  virtual int  dodge_roll();
  virtual void take_damage(Damage_type type, int damage, std::string reason,
                           Body_part part = BODYPART_NULL);

  virtual bool can_sense(Map* map, int x, int y);

  int posx, posy;
  int action_points;

  Player_activity activity;

  Item weapon;
  std::vector<Item> inventory;
  std::vector<Item> items_worn;
};
#endif
