#include "entity.h"
#include "rng.h"
#include "game.h"
#include "geometry.h"
#include "map.h"
#include <sstream>

Stats::Stats()
{
  str   = 10;
  dex   = 10;
  intel = 10;
  per   = 10;
}

Stats::~Stats()
{
}

Entity_plan::Entity_plan()
{
  target_point = Tripoint(-1, -1, -1);
  target_entity = NULL;
  attention = 0;
  goal_type = AIGOAL_NULL;
}

Entity_plan::~Entity_plan()
{
}

void Entity_plan::set_target(AI_goal goal, Tripoint target, int att)
{
  if (att == -1) { // att defaults to -1
    att = 15;
  }
  target_point = target;
  target_entity = NULL; // TODO: Don't do this?
  attention = att;
  goal_type = goal;
}

void Entity_plan::set_target(AI_goal goal, Entity* target, int att)
{
  if (!target) {
    target_point = Tripoint(-1, -1, -1);
    attention = 0;
    target_entity = NULL;
    return;
  }
  if (att == -1) { // att defaults to -1
    att = 15;
  }
  target_entity = target;
  target_point = target->pos;
  attention = att;
  goal_type = goal;
}

void Entity_plan::generate_path_to_target(Entity_AI AI, Tripoint origin)
{
  if (!is_active() || target_point.x < 0 || target_point.y < 0) {
    return;
  }

  Generic_map map = GAME.map->get_movement_map(AI, origin, target_point);
  Pathfinder pf(map);

  path = pf.get_path(PATH_A_STAR, origin, target_point);

  if (path.empty()) {
    attention = 0;
    target_point = Tripoint(-1, -1, -1);
    target_entity = NULL;
  }
}

void Entity_plan::update()
{
  if (attention > 0) {
    attention--;
  }
  if (attention <= 0) {
    target_entity = NULL;
  }
}

bool Entity_plan::is_active()
{
  if (attention <= 0) {
    return false;
  }
  if (target_point.x < 0 || target_point.y < 0) {
    return false;
  }
  return true;
}

Tripoint Entity_plan::next_step()
{
  if (path.empty()) {
    return Tripoint(-1, -1, -1);
  }
  return path[0];
}

void Entity_plan::erase_step()
{
  path.erase_step(0);
}

void Entity_plan::clear()
{
  target_point = Tripoint(-1, -1, -1);
  target_entity = NULL;
  attention = 0;
  goal_type = AIGOAL_NULL;
}

Entity::Entity()
{
  uid = -1;
  pos.x = 15;
  pos.y = 15;
  pos.z = 0;
  action_points = 0;
  dead = false;
  killed_by_player = false;
  hunger = 0;
  thirst = 0;
  special_timer = 0;
}

Entity::~Entity()
{
}

std::string Entity::get_data_name()
{
  return "Nothing";
}

std::string Entity::get_name()
{
  return "Nothing";
}

std::string Entity::get_name_to_player()
{
  return "Nothing";
}

std::string Entity::get_possessive()
{
  return "Nothing's";
}

std::string Entity::conjugate(const std::string &verb)
{
// Dumbest conjugation ever, but it should work for most cases!
// TODO: Special-case stuff?
  if (verb[ verb.length() - 1 ] == 's') {
    return verb + "es"; // "miss" => "misses"
  }
  return verb + "s";    // "hit"  => "hits"
}

glyph Entity::get_glyph()
{
  return glyph();
}

void Entity::die()
{
// TODO: Drop a corpse.
  for (int i = 0; i < inventory.size(); i++) {
    GAME.map->add_item( inventory[i], pos.x, pos.y, pos.z );
  }
  for (int i = 0; i < items_worn.size(); i++) {
    GAME.map->add_item( items_worn[i], pos.x, pos.y, pos.z );
  }
  if (weapon.is_real()) {
    GAME.map->add_item( weapon, pos.x, pos.y, pos.z );
  }
// Tell any monsters that're targeting us to QUIT IT
  for (std::list<Entity*>::iterator it = GAME.entities.instances.begin();
       it != GAME.entities.instances.end();
       it++) {
    if ( (*it)->plan.target_entity == this ) {
      (*it)->plan.clear();
    }
  }
}

void Entity::gain_action_points()
{
  action_points += get_speed();
}

int Entity::get_speed()
{
  return 100;
}

void Entity::take_turn()
{
}

bool Entity::try_goal(AI_goal goal)
{
  return false;
}

bool Entity::pick_attack_victim()
{
  return false;
}

bool Entity::pick_flee_target()
{
  return false;
}

std::vector<Ranged_attack> Entity::get_ranged_attacks()
{
  return std::vector<Ranged_attack>();
}

Ranged_attack Entity::pick_ranged_attack(Entity* target)
{
  std::vector<Ranged_attack> ra = get_ranged_attacks();
  if (ra.empty()) {
    return Ranged_attack();
  }
  int total_chance = 0;
  std::vector<Ranged_attack> used;
  for (int i = 0; i < ra.size(); i++) {
    if (target && rl_dist(pos, target->pos) > ra[i].range) {
      ra.erase(ra.begin() + i);
      i--;
    } else {
      total_chance += ra[i].weight;
    }
  }
  if (ra.empty()) { // No attacks in range!
    return Ranged_attack();
  }
  int index = rng(1, total_chance);
  for (int i = 0; i < ra.size(); i++) {
    index -= ra[i].weight;
    if (index <= 0) {
      return ra[i];
    }
  }
  return ra.back();
}

Entity_AI Entity::get_AI()
{
  return Entity_AI();
}

bool Entity::has_sense(Sense_type sense)
{
  return false;
}

int Entity::get_genus_uid()
{
  return -2;
}

bool Entity::can_sense(Entity* entity)
{
  return false;
}

bool Entity::can_see(Map* map, Tripoint target)
{
  return can_see(map, target.x, target.y, target.z);
}

bool Entity::can_see(Map* map, int x, int y, int z)
{
  if (z == 999) { // z defaults to 999
    z = pos.z;
  }
  if (!map || !has_sense(SENSE_SIGHT)) {
    return false;
  }
  return map->senses(pos.x, pos.y, pos.z, x, y, z, SIGHT_DIST, SENSE_SIGHT);
}

bool Entity::can_move_to(Map* map, Tripoint move)
{
  return can_move_to(map, move.x, move.y, move.z);
}

bool Entity::can_move_to(Map* map, int x, int y, int z)
{
  if (z == 999) { // z defaults to 999
    z = pos.z;
  }
  if (!map) {
    return false;
  }
  if (map->move_cost(x, y, z) == 0) {
    return false;
  }
  if (GAME.entities.entity_at(x, y, z) !=NULL) {
    return false;
  }
  return true;
}

bool Entity::can_smash(Map* map, int x, int y, int z)
{
  if (z == 999) { // z defaults to 999
    z = pos.z;
  }
  return can_smash(map, Tripoint(x, y, z));
}

bool Entity::can_smash(Map *map, Tripoint sm)
{
  return map->is_smashable(sm);
}

void Entity::move_to(Map* map, Tripoint move)
{
  move_to(map, move.x, move.y, move.z);
}

void Entity::move_to(Map* map, int x, int y, int z)
{
  pos.x = x;
  pos.y = y;
  if (z != 999) { // z defaults to 999
    pos.z = z;
  }
  if (map) {
    action_points -= map->move_cost(x, y, z);
  }
}

void Entity::smash(Map* map, Tripoint sm)
{
  if (!map) {
    return;
  }
  Attack att = base_attack();
  action_points -= att.speed;
  map->smash(sm, att.roll_damage());
}

void Entity::smash(Map* map, int x, int y, int z)
{
  if (z == 999) { // z defaults to 999
    z = pos.z;
  }
  smash(map, Tripoint(x, y, z));
}
  

void Entity::pause()
{
  action_points -= 100;
}

void Entity::set_activity(Player_activity_type type, int duration,
                          int primary_uid, int secondary_uid)
{
// TODO: Error or something if we have an activity?
  activity = Player_activity(type, duration, primary_uid, secondary_uid);
}

void Entity::use_ap(int amount)
{
  if (amount < 0) {
    return;
  }
  action_points -= amount;
}

void Entity::shift(int shiftx, int shifty)
{
  pos.x -= shiftx * SUBMAP_SIZE;
  pos.y -= shifty * SUBMAP_SIZE;
}

void Entity::prepare()
{
  hunger++;
  thirst += 2;
}

bool Entity::add_item(Item item)
{
  if (!item.is_real()) {
    return false;
  }
  if (item.combines()) {
    Item* added = ref_item_of_type(item.type);
    if (added) {
      return (*added).combine_with(item);
    }
  }
  inventory.push_back(item);
  return true;
}

Item* Entity::ref_item_uid(int uid)
{
  if (weapon.is_real() && weapon.get_uid() == uid) {
    return &weapon;
  }
  for (int i = 0; i < items_worn.size(); i++) {
    if (items_worn[i].get_uid() == uid) {
      return &(items_worn[i]);
    }
  }
  for (int i = 0; i < inventory.size(); i++) {
    if (inventory[i].get_uid() == uid) {
      return &(inventory[i]);
    }
  }
  return NULL;
}

Item Entity::get_item_of_type(Item_type *type)
{
  if (!type) {
    return Item();
  }
  for (int i = 0; i < inventory.size(); i++) {
    if (inventory[i].type == type) {
      return inventory[i];
    }
  }
// TODO: Weapon & armor?
  return Item();
}

Item* Entity::ref_item_of_type(Item_type *type)
{
  if (!type) {
    return NULL;
  }
  for (int i = 0; i < inventory.size(); i++) {
    if (inventory[i].type == type) {
      return &(inventory[i]);
    }
  }
// TODO: Weapon & armor?
  return NULL;
}

Item Entity::remove_item_uid(int uid, int count)
{
  Item ret = Item();
  int ret_count = count;
  if (weapon.type && weapon.get_uid() == uid) {
    if (count == 0) {
      ret_count = weapon.count;
    } else if (weapon.count < count) {
      ret_count = weapon.count;
    }
    weapon.count -= count;
    ret = weapon;
    ret.count = ret_count;
    if (weapon.count <= 0 || count == 0) {
      weapon = Item();
    }
    return ret;
  }
// Items_worn should never have a count.
  for (int i = 0; i < items_worn.size(); i++) {
    if (items_worn[i].get_uid() == uid) {
      ret = items_worn[i];
      ret.count = 1;
      items_worn.erase(items_worn.begin() + i);
      return ret;
    }
  }
  for (int i = 0; i < inventory.size(); i++) {
    if (inventory[i].get_uid() == uid) {
      if (count == 0) {
        ret_count = inventory[i].count;
      } else if (inventory[i].count < count) {
        ret_count = inventory[i].count;
      }
      ret = inventory[i];
      ret.count = ret_count;
      if (count == 0 || inventory[i].count <= 0) {
        inventory.erase(inventory.begin() + i);
      }
      return ret;
    }
  }
  return ret;
}

void Entity::wield_item_uid(int uid)
{
  for (int i = 0; i < inventory.size(); i++) {
    if (inventory[i].get_uid() == uid) {
      weapon = inventory[i];
      weapon.count = 1;
      if (inventory[i].count <= 1) {
        inventory.erase(inventory.begin() + i);
      } else {
        inventory[i].count--;
      }
      return;
    }
  }
}

void Entity::sheath_weapon()
{
  if (weapon.is_real()) {
    add_item(weapon);
  }
}

void Entity::wear_item_uid(int uid)
{
// TODO: Return a failure reason when attempting to wear something we're wearing
//       or a non-clothing item
  if (weapon.is_real() && weapon.get_uid() == uid) {
    if (weapon.get_item_class() == ITEM_CLASS_CLOTHING) {
      items_worn.push_back(weapon);
      weapon = Item();
    }
    return;
  }
  for (int i = 0; i < inventory.size(); i++) {
    if (inventory[i].get_uid() == uid) {
      if (inventory[i].get_item_class() == ITEM_CLASS_CLOTHING) {
        items_worn.push_back(inventory[i]);
        items_worn.back().count = 1;
        if (inventory[i].count <= 1) {
          inventory.erase( inventory.begin() + i );
        } else {
          inventory[i].count--;
        }
      }
      return;
    }
  }
}

void Entity::reload_prep(int uid)
{
  Item* it = ref_item_uid(uid);
  if (!it || !it->can_reload()) {
    return;
  }
  Item ammo = pick_ammo_for(it);
  if (ammo.is_real()) {
    set_activity(PLAYER_ACTIVITY_RELOAD, it->time_to_reload(),
                 it->get_uid(), ammo.get_uid());
  }
}

Item Entity::pick_ammo_for(Item *it)
{
  if (!it || !it->can_reload()) {
    return Item();
  } 
  if (it->charges == it->get_max_charges()) {
    return Item();
  }
  if (it->charges > 0 && it->ammo) {
    return get_item_of_type(it->ammo);
  }
// TODO: Automate ammo selection for NPCs
  return Item();
}

bool Entity::is_wielding_item_uid(int uid)
{
  return weapon.is_real() && weapon.get_uid() == uid;
}

bool Entity::is_wearing_item_uid(int uid)
{
  for (int i = 0; i < items_worn.size(); i++) {
    if (items_worn[i].is_real() && items_worn[i].get_uid() == uid) {
      return true;
    }
  }
  return false;
}

bool Entity::is_carrying_item_uid(int uid)
{
  for (int i = 0; i < inventory.size(); i++) {
    if (inventory[i].is_real() && inventory[i].get_uid() == uid) {
      return true;
    }
  }
  return false;
}

bool Entity::has_item_uid(int uid)
{
  return (is_wielding_item_uid(uid) || is_wearing_item_uid(uid) ||
          is_carrying_item_uid(uid));
}

std::string Entity::drop_item_message(Item &it)
{
  int uid = it.get_uid();
  if (!it.is_real() || !ref_item_uid(uid)) {
    return "You don't have that item.";
  }
  std::stringstream ret;
  ret << get_name_to_player() << " " << conjugate("drop") << " " <<
         get_possessive() << " " << it.get_name() << ".";
  return ret.str();
}

std::string Entity::wear_item_message(Item &it)
{
  int uid = it.get_uid();
  if (!it.is_real() || !ref_item_uid(uid)) {
    return "You don't have that item.";
  }
  if (it.get_item_class() != ITEM_CLASS_CLOTHING) {
    return it.get_name_indefinite() + " is not clothing!";
  }
  if (is_wearing_item_uid(uid)) { 
    return "You're already wearing that.";
  }
  std::stringstream ret;
  ret << get_name_to_player() << " " << conjugate("put") << " on " <<
         get_possessive() << " " << it.get_name() << ".";
  return ret.str();
}

std::string Entity::wield_item_message(Item &it)
{
  int uid = it.get_uid();
  std::stringstream ret;
  ret << get_name_to_player();
  if (!it.is_real() || !ref_item_uid(uid)) { // Should this indicate a bug?
    ret << " don't have that item.";
  } else if (is_wielding_item_uid(uid)) {
    ret << "'re already wielding that.";
  } else if (is_wearing_item_uid(uid)) {
    ret << "'re wearing that item - take it off first.";
  } else {
    ret << " " << conjugate("wield") << " " << it.get_name_definite() << ".";
  }
  return ret.str();
}

std::string Entity::sheath_weapon_message()
{
  if (!weapon.is_real()) {
    return "";
  }
  std::stringstream ret;
  ret << get_name_to_player() << " " << conjugate("put") << " away " <<
         get_possessive() << " " << weapon.get_name() << ".";
  return ret.str();
}

Attack Entity::base_attack()
{
  Attack ret;
  ret.damage[DAMAGE_BASH] = 1 + stats.str / 4;
  ret.speed  = 100 - stats.dex;
  ret.to_hit = stats.dex / 4 - 3;
  return ret;
}

Attack Entity::std_attack()
{
  Attack att = base_attack();
  if (weapon.is_real()) {
    att.use_weapon(weapon, stats);
  }
  return att;
}

bool Entity::can_attack(Entity* target)
{
  if (!target) {
    return false;
  }
  if (pos.z != target->pos.z) {
    return false;
  }
  if (rl_dist(pos, target->pos) <= 1){
    return true;
  }
  return false;
}

void Entity::attack(Entity* target)
{
  if (!target) {
    debugmsg("'%s' attempted attack() on a null target.");
    return;
  }

  Attack att = std_attack();

  action_points -= att.speed;

  bool you_see = GAME.player->can_sense(GAME.map, pos.x, pos.y);
  bool attacker_is_you = is_you();

  std::string miss_verb = (attacker_is_you ? "miss" : "misses");

  if (hit_roll(att.to_hit) < target->dodge_roll()) {
    if (you_see) {
      std::stringstream msg;
      msg << get_name_to_player() << " " << miss_verb << " " <<
             target->get_name_to_player() << "!";
      GAME.add_msg( msg.str() );
    }
// TODO: action_point penalty for missing?
    return;
  }

  Body_part bp_hit = (target->is_player() ? random_body_part_to_hit() :
                                            BODY_PART_NULL);

// TODO: Should total_damage be reduced by damage absorbed by armor?
  Damage_set damage = att.roll_damage();
  for (int i = 0; i < DAMAGE_MAX; i++) {
    int dam = damage.get_damage(i);
    target->take_damage(Damage_type(i), dam, get_name_to_player(), bp_hit);
  }

  if (you_see) {
    std::stringstream damage_ss;
    damage_ss << get_name_to_player() << " ";
    if (attacker_is_you) {
      damage_ss << att.verb_second;
    } else {
      damage_ss << att.verb_third;
    }
    damage_ss << " ";
    if (bp_hit == BODY_PART_NULL) {
      damage_ss << target->get_name_to_player();
    } else {
      damage_ss << target->get_possessive() << " " << body_part_name(bp_hit);
    }
    if (target->is_you()) {
      if (damage.total_damage() == 0) {
        damage_ss << " but does no damage";
      } else {
        damage_ss << " for " << damage.total_damage() << " damage";
      }
    }
    damage_ss << "!";
    GAME.add_msg( damage_ss.str() );
  }
}
  
int Entity::hit_roll(int bonus)
{
  return rng(1, 10) + bonus;
}

int Entity::dodge_roll()
{
  return rng(1, 10);
}

// This one gets overloaded fully.
void Entity::take_damage(Damage_type type, int damage, std::string reason,
                         Body_part part)
{
}

void Entity::take_damage(Damage_set damage, std::string reason, Body_part part)
{
  for (int i = 0; i < DAMAGE_MAX; i++) {
    Damage_type type = Damage_type(i);
    int dam = damage.get_damage(type);
    take_damage(type, dam, reason, part);
  }
}

Ranged_attack Entity::throw_item(Item it)
{
  Ranged_attack ret = it.get_thrown_attack();
// TODO: Alter variance based on skill
  return ret;
}

Ranged_attack Entity::fire_weapon()
{
// TODO: Base on skill
// TODO: Use >1 round if we've selected a burst/auto shot
  if (!weapon.is_real() || weapon.get_item_class() != ITEM_CLASS_LAUNCHER ||
      !weapon.ammo || weapon.charges == 0) {
    return Ranged_attack();
  }
  weapon.charges--;
  action_points -= weapon.time_to_fire();
  return weapon.get_fired_attack();
}

bool Entity::can_fire_weapon()
{
  if (!weapon.is_real()) {
    if (is_player()) {
      GAME.add_msg("You are not wielding anything.");
    }
    return false;
  } else if (weapon.get_item_class() != ITEM_CLASS_LAUNCHER) {
    if (is_player()) {
      GAME.add_msg("You cannot fire %s.", weapon.get_name_indefinite().c_str());
    }
    return false;
  } else if (weapon.charges == 0 || !weapon.ammo) {
    if (is_player()) {
      GAME.add_msg("You need to reload %s.",
                   weapon.get_name_definite().c_str());
    }
    return false;
  }
  return true;
}

bool Entity::can_attack_ranged(Entity* target)
{
  if (!target) {
    return false;
  }
  std::vector<Ranged_attack> ra = get_ranged_attacks();
  if (ra.empty()) {
    return false;
  }
  int range = rl_dist(pos, target->pos);
  for (int i = 0; i < ra.size(); i++) {
    if (ra[i].range >= range) {
      return true;
    }
  }
  return false;
}

void Entity::attack_ranged(Entity* target, Ranged_attack ra)
{
  if (!target) {
    return;
  }
  if (ra.range == 0) {
    return; // This means that it's not a real ranged attack
  }
// Set the special_timer.  This really only affects monsters - monsters can't
// use ranged attacks if their special_timer is more than 0.
  special_timer = ra.charge_time;
  GAME.launch_projectile(this, ra, pos, target->pos);
  action_points -= ra.speed;
  if (GAME.player->can_see(GAME.map, pos)) {
    std::string target_name;
    if (GAME.player->can_see(GAME.map, target->pos)) {
      target_name = target->get_name_to_player();
    } else {
      target_name = "something";
    }
    GAME.add_msg("%s %s at %s!", get_name_to_player().c_str(),
                 ra.verb_third.c_str(), target_name.c_str());
  }
}

bool Entity::can_sense(Map* map, int x, int y, int z)
{
  if (z == 999) { // z defaults to 999
    z = pos.z;
  }
// Default Entity function just uses sight
  return map->senses(pos.x, pos.y, pos.z, x, y, z, SIGHT_DIST, SENSE_SIGHT);
}

bool Entity::can_sense(Map* map, Tripoint target)
{
  return can_sense(map, target.x, target.y, target.z);
}

Entity_pool::Entity_pool()
{
  next_uid = 0;
}

Entity_pool::~Entity_pool()
{
  for (std::list<Entity*>::iterator it = instances.begin();
       it != instances.end();
       it++) {
    delete (*it);
  }
}

void Entity_pool::add_entity(Entity* ent)
{
  if (!ent) {
    debugmsg("Tried to add_entity NULL to Entity_pool");
    return;
  }
  ent->uid = next_uid;
  next_uid++;
  instances.push_back(ent);
  uid_map[ent->uid] = ent;
}

void Entity_pool::push_back(Entity* ent)
{
  if (!ent) {
    debugmsg("Tried to push_back NULL to Entity_pool");
    return;
  }
  instances.push_back(ent);
  uid_map[ent->uid] = ent;
}

void Entity_pool::clear()
{
  instances.clear();
  uid_map.clear();
}

std::list<Entity*>::iterator Entity_pool::erase(std::list<Entity*>::iterator it)
{
  uid_map.erase( (*it)->uid );
  return instances.erase(it);
}

Entity* Entity_pool::lookup_uid(int uid)
{
  if (uid_map.count(uid) == 0) {
    return NULL;
  }
  return uid_map[uid];
}

Entity* Entity_pool::entity_at(int posx, int posy)
{
  for (std::list<Entity*>::iterator it = instances.begin();
       it != instances.end();
       it++) {
    if ((*it)->pos.x == posx && (*it)->pos.y == posy) {
      return (*it);
    }
  }
  return NULL;
}

Entity* Entity_pool::entity_at(Tripoint pos)
{
  return entity_at(pos.x, pos.y, pos.z);
}

Entity* Entity_pool::entity_at(int posx, int posy, int posz)
{
  for (std::list<Entity*>::iterator it = instances.begin();
       it != instances.end();
       it++) {
    if ((*it)->pos.x == posx && (*it)->pos.y == posy && (*it)->pos.z == posz) {
      return (*it);
    }
  }
  return NULL;
}
