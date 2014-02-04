#ifndef _ENTITY_H_
#define _ENTITY_H_

#include "glyph.h"
#include "item.h"
#include "player_activity.h"
#include "geometry.h"
#include "enum.h"
#include "pathfind.h"
#include "attack.h"
#include "entity_ai.h"
#include <string>
#include <map>
#include <list>

class Map;

struct Stats
{
  Stats();
  ~Stats();

  int str, dex, intel, per;
};

struct Entity_plan
{
  Entity_plan();
  ~Entity_plan();

  void set_target(AI_goal goal, Tripoint target, int att = -1);
  void set_target(AI_goal goal, Entity*  target, int att = -1);

  void generate_path_to_target(Entity_AI AI, Tripoint origin);

  bool is_active();

  Tripoint next_step();
  void erase_step();
  void clear();

  Tripoint target_point;
  Entity*  target_entity;
  Pathfinder pf;
  Path path;
  int attention;
  AI_goal goal_type;
};

class Entity
{
public:
  Entity();
  virtual ~Entity();

  virtual std::string get_data_name();
  virtual std::string get_name();
  virtual std::string get_name_to_player();
  virtual std::string get_possessive();
  virtual std::string conjugate(const std::string &verb);
  virtual glyph get_glyph();

  virtual bool is_player()  { return false; }
  virtual bool is_monster() { return false; }
  virtual bool is_you()     { return false; } // As in THE player

  virtual void die();
  virtual void gain_action_points();
  virtual int  get_speed();
  virtual void take_turn();
  virtual bool try_goal(AI_goal goal);
  virtual bool pick_attack_victim();
  virtual bool pick_flee_target();

  virtual Entity_AI   get_AI();
  virtual bool has_sense(Sense_type sense);
  virtual int  get_genus_uid();

  virtual bool can_sense  (Entity* entity);
  virtual bool can_see    (Map* map, Tripoint target);
  virtual bool can_see    (Map* map, int x, int y, int z = 999);

  virtual bool can_move_to(Map* map, Tripoint move);
  virtual bool can_move_to(Map* map, int x, int y, int z = 999);
  virtual bool can_smash  (Map* map, Tripoint move);
  virtual bool can_smash  (Map* map, int x, int y, int z = 999);
  virtual void move_to    (Map* map, Tripoint move);
  virtual void move_to    (Map* map, int x, int y, int z = 999);
  virtual void smash      (Map* map, Tripoint sm);
  virtual void smash      (Map* map, int x, int y, int z = 999);
  virtual void pause();

// Misc action functions
  void set_activity(Player_activity_type type, int duration,
                    int primary_uid = -1, int secondary_uid = -1);
  void use_ap(int amount);
  void shift(int shiftx, int shifty); // For map shifting

  void prepare(); // Reset their stats, increase hunger/thirst, etc.
  
// Inventory functions
  virtual bool add_item(Item item);
  Item  get_item_of_type(Item_type *type);
  Item* ref_item_of_type(Item_type *type);
  Item* ref_item_uid   (int uid);
  Item  remove_item_uid(int uid, int count = 0);
  void  wield_item_uid (int uid);
  void  sheath_weapon();
  void  wear_item_uid  (int uid);
  void  reload_prep    (int uid);
  virtual Item pick_ammo_for(Item *it);

  bool is_wielding_item_uid(int uid);
  bool is_wearing_item_uid(int uid);
  bool is_carrying_item_uid(int uid);
  bool has_item_uid(int uid); // wielding, wearing or carrying

// Message functions
  virtual std::string drop_item_message(Item &it);
  virtual std::string wear_item_message(Item &it);
  virtual std::string wield_item_message(Item &it);
  virtual std::string sheath_weapon_message();

// Combat functions
  virtual Attack base_attack();
  virtual Attack std_attack(); // With weapon if it exists
  virtual void attack(Entity* target);
  virtual int  hit_roll(int bonus);
  virtual int  dodge_roll();
  virtual void take_damage(Damage_type type, int damage, std::string reason,
                           Body_part part = BODYPART_NULL);
  virtual void take_damage(Damage_set damage, std::string reason,
                           Body_part part = BODYPART_NULL);
  virtual Ranged_attack throw_item(Item it);
  virtual Ranged_attack fire_weapon();

  virtual bool can_sense(Map* map, int x, int y, int z = 999);
  virtual bool can_sense(Map* map, Tripoint target);

// Values
  int uid;

  Tripoint pos;
  //int posx, posy, posz;
  int action_points;

  bool dead;
  bool killed_by_player;

  Player_activity activity;
  Entity_plan plan;

  Stats stats;
  int hunger, thirst;

  Item weapon;
  std::vector<Item> inventory;
  std::vector<Item> items_worn;
};

/* For now, Entity_pool does NOT include a map which uses location as a key.
 * Originally I thought this would speed up Game::entity_at(int x, int y), but
 * in order for this map to be useful, we'd have to update it every turn, which
 * means it'd probably be more trouble than it's worth, except when the map is
 * being called several times per turn.  We'd also have to update it after
 * every monster moves, which is a lot.
 * This means that monster_at() has to iterate over all monsters, which is
 * potentially slow, but what can you do.
 */

class Entity_pool
{
public:
  Entity_pool();
  ~Entity_pool();

  void add_entity(Entity* ent);
  void push_back(Entity* ent); // Same, except don't re-assign UID
  void clear();
  std::list<Entity*>::iterator erase(std::list<Entity*>::iterator it);

  Entity* lookup_uid(int uid);
  Entity* entity_at(int posx, int posy);
  Entity* entity_at(Tripoint pos);
  Entity* entity_at(int posx, int posy, int posz);

  std::list<Entity*> instances;
private:
  std::map<int,Entity*> uid_map;
  int next_uid;
};
  
#endif
