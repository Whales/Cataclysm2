#include "entity.h"
#include "rng.h"
#include "game.h"
#include "geometry.h"
#include "map.h"
#include <sstream>

Stats::Stats()
{
  strength     = 10;
  dexterity    = 10;
  intelligence = 10;
  perception   = 10;
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
  path.clear();
}

void Entity_plan::shift(int shiftx, int shifty)
{
  if (target_point.x > -1) {
    target_point.x -= shiftx * SUBMAP_SIZE;
    target_point.y -= shifty * SUBMAP_SIZE;
  }
  if (target_point.x < 0 || target_point.y < 0) {
    clear();
  }
  path.shift(shiftx, shifty);
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
  fatigue = 0;
  stomach_food = 0;
  stomach_water = 0;
  pain = 0;
  painkill = 0;
  special_timer = 0;

// Initialize trait vector
  for (int i = 0; i < TRAIT_MAX; i++) {
    traits.push_back(false);
  }
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

void Entity::process_status_effects()
{
  for (int i = 0; i < effects.size(); i++) {
    int level = effects[i].level;

    switch (effects[i].type) {

      case STATUS_SLEEP_AID:
        if (GAME.minute_timer(1)) {
          fatigue++;
        }
        break;

      case STATUS_PAINKILL_MILD: {
        int pkill = (level > 2 ? 20 : 10 * level);
        if (GAME.turn_timer(24 - level * 4) && painkill < pkill) {
          painkill++;
        }
      } break;

      case STATUS_PAINKILL_MED: {
        int pkill = (level > 4 ? 90 : 30 + 15 * level);
        if (GAME.turn_timer(17 - level * 2) && painkill < pkill) {
          painkill++;
        }
      } break;

      case STATUS_PAINKILL_LONG: {
        int pkill = (level > 3 ? 60 : 30 + 10 * level);
        if (GAME.minute_timer(2) && painkill < pkill) {
          painkill++;
        }
      } break;

      case STATUS_PAINKILL_HEAVY: {
        int pkill = (level > 6 ? 200 : 80 + 20 * level);
        if (GAME.turn_timer(11 - level) && painkill < pkill) {
          painkill++;
        }
      } break;

    } // switch (effects[i].type)

    if (effects[i].decrement()) { // Returns true if duration <= 0
      effects.erase(effects.begin() + i);
      i--;
    }
  }
}

void Entity::gain_action_points()
{
  action_points += get_speed();
}

nc_color Entity::get_speed_color()
{
  int speed = get_speed();
  if (speed <= 70) {
    return c_red;
  }
  if (speed <= 85) {
    return c_ltred;
  }
  if (speed < 100) {
    return c_yellow;
  }
  if (speed == 100) {
    return c_white;
  }
  if (speed < 115) {
    return c_ltgreen;
  }
  return c_green;
}

int Entity::get_speed()
{
  int ret = 100;
  ret -= get_hunger_speed_penalty();
  ret -= get_thirst_speed_penalty();
  ret -= get_fatigue_speed_penalty();
  ret -= get_pain_speed_penalty();
  for (int i = 0; i < effects.size(); i++) {
    ret += effects[i].speed_mod();
  }

  if (has_trait(TRAIT_QUICK)) {
    ret = ret + (ret + 19) / 20; // "+ 19" means we always round up.
  }
  return ret;
}

int Entity::get_movement_cost()
{
  return 100;
}

int Entity::get_hunger_speed_penalty()
{
  if (hunger >= 960) {
    return 30;
  }
  if (hunger >= 480) {
    return 20;
  }
  if (hunger >= 240) {
    return 12;
  }
  if (hunger >= 120) {
    return 7;
  }
  if (hunger >= 60) {
    return 5;
  }
  return 0;
}

Stats Entity::get_hunger_stats_penalty()
{
  Stats ret;
  if (hunger >= 960) {
    ret.strength     = -3;
    ret.dexterity    = -3;
    ret.intelligence = -3;
    ret.perception   = -2;
  } else if (hunger >= 480) {
    ret.strength     = -3;
    ret.dexterity    = -2;
    ret.intelligence = -3;
    ret.perception   = -1;
  } else if (hunger >= 240) {
    ret.strength     = -2;
    ret.dexterity    = -1;
    ret.intelligence = -2;
    ret.perception   = -1;
  } else if (hunger >= 120) {
    ret.strength     = -1;
    ret.dexterity    = -1;
    ret.intelligence = -1;
  } else if (hunger >= 60) {
    ret.intelligence = -1;
  }
  return ret;
}

int Entity::get_thirst_speed_penalty()
{
  if (thirst >= 360) {
    return 35;
  }
  if (thirst >= 240) {
    return 25;
  }
  if (thirst >= 120) {
    return 18;
  }
  if (thirst >= 90) {
    return 10;
  }
  if (thirst >= 60) {
    return 5;
  }
  return 0;
}


Stats Entity::get_thirst_stats_penalty()
{
  Stats ret;
  if (thirst >= 360) {
    ret.strength     = -3;
    ret.dexterity    = -3;
    ret.intelligence = -3;
    ret.perception   = -3;
  } else if (thirst >= 240) {
    ret.strength     = -3;
    ret.dexterity    = -2;
    ret.intelligence = -3;
    ret.perception   = -2;
  } else if (thirst >= 120) {
    ret.strength     = -2;
    ret.dexterity    = -1;
    ret.intelligence = -2;
    ret.perception   = -1;
  } else if (thirst >= 90) {
    ret.strength     = -1;
    ret.intelligence = -1;
  } else if (thirst >= 60) {
    ret.intelligence = -1;
  }
  return ret;
}

int Entity::get_fatigue_speed_penalty()
{
// Note that we're "tired" starting at 160; the first four hours have no penalty
  if (fatigue >= 360) {
    return 35;
  }
  if (fatigue >= 320) {
    return 25;
  }
  if (fatigue >= 280) {
    return 15;
  }
  if (fatigue >= 240) {
    return 10;
  }
  if (fatigue >= 200) {
    return 5;
  }
  return 0;
}

Stats Entity::get_fatigue_stats_penalty()
{
  Stats ret;
  if (fatigue >= 360) {
    ret.strength     = -3;
    ret.dexterity    = -2;
    ret.intelligence = -3;
    ret.perception   = -3;
  } else if (fatigue >= 320) {
    ret.strength     = -3;
    ret.dexterity    = -1;
    ret.intelligence = -3;
    ret.perception   = -2;
  } else if (fatigue >= 280) {
    ret.strength     = -2;
    ret.dexterity    = -1;
    ret.intelligence = -3;
    ret.perception   = -2;
  } else if (fatigue >= 240) {
    ret.strength     = -1;
    ret.intelligence = -2;
    ret.perception   = -1;
  } else if (fatigue >= 200) {
    ret.intelligence = -1;
    ret.perception   = -1;
  } else if (fatigue >= 160) {
    ret.intelligence = -1;
  }
  return ret;
}

int Entity::get_net_pain()
{
  int ret = pain;
  ret -= painkill;
  return (ret < 0 ? 0 : ret);
}

int Entity::get_pain_speed_penalty()
{
  int p = get_net_pain();
  p -= 16;  // First 19 pain points don't incur a speed penalty
  if (p <= 0) {
    return 0;
  }
  int ret = p / 4;
  return (ret > 50 ? 50 : ret);
}

// TODO: Overload for monsters?
int Entity::get_smell()
{
  if (has_trait(TRAIT_SMELLY)) {
    return 18;
  }
  return 10;
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

bool Entity::has_trait(Trait_id trait)
{
  if (trait >= traits.size()) {
    debugmsg("Entity '%s' has traits.size() %d (should be %d)!",
             get_name().c_str(), traits.size(), TRAIT_MAX);
    return false;
  }
  return traits[trait];
}

int Entity::get_genus_uid()
{
  return -2;
}

int Entity::get_hunger_minimum()
{
  return -80;
}

int Entity::get_thirst_minimum()
{
  return -40;
}

int Entity::get_stomach_maximum()
{
  return 60;
}

int Entity::sight_range(int light_level)
{
  int ret = light_level;
  if (has_trait(TRAIT_NIGHT_VISION)) {
    if (ret < 3) {
      ret = ret * 2;
    } else if (ret == 3) {
      ret = 4;
    }
  }
  if (has_trait(TRAIT_MYOPIC) && ret > 6 &&
      !is_wearing_item_flag(ITEM_FLAG_GLASSES)) {
    ret = 6;
  }
  return ret;
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
  int range = sight_range( GAME.get_light_level() );
  return map->senses(pos.x, pos.y, pos.z, x, y, z, range, SENSE_SIGHT);
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
  if (GAME.entities.entity_at(x, y, z) != NULL) {
    return false;
  }
// Ensure that the furniture can be dragged thataway!
// can_drag_furniture() checks if (dragged) is empty, so we don't have to
  if (!can_drag_furniture_to(map, x, y, z)) {
    return false;
  }
  return true;
}

bool Entity::can_drag_furniture_to(Map* map, Tripoint move)
{
  return can_drag_furniture_to(map, move.x, move.y, move.z);
}

// This is NOT responsible for checking if we can move there (e.g. ensuring the
// destination isn't a solid wall)
bool Entity::can_drag_furniture_to(Map* map, int x, int y, int z)
{
// Make sure there's a map...
  if (!map) {
    return false;
  }
// If we're dragged nothing, it's always okay!
  if (dragged.empty()) {
    return true;
  }
// Check every tile of the furniture
  for (int i = 0; i < dragged.size(); i++) {
    Tripoint test = Tripoint(x + dragged[i].pos.x, y + dragged[i].pos.y, z);
    if (map->move_cost(test) == 0) {
      return false;
    }
// No displacing furniture; if the furniture there is of the same UID as the
// furniture we're dragging, then it IS the furniture we're dragging.
    //debugmsg("player %s; drag %s; test %s", pos.str().c_str(), dragged[i].pos.str().c_str(), test.str().c_str());
    Furniture* blocker = map->furniture_at(test);
    if (blocker && blocker->get_uid() != dragged[i].furniture.get_uid()) {
      return false;
    }
// No displacing entities (except us!)
    Entity* blocker_ent = GAME.entities.entity_at(test);
    if (blocker_ent != NULL && blocker_ent != this) {
      return false;
    }
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
  if (!map) {
    debugmsg("Entity::move_to() called with NULL map!");
    return;
  }
// First, removing the furniture we're dragged from its old position
  for (int i = 0; i < dragged.size(); i++) {
    map->clear_furniture(pos + dragged[i].pos);
  }
  pos.x = x;
  pos.y = y;
  if (z != 999) { // z defaults to 999
    pos.z = z;
  }

  if (map) {
/* If get_movement_cost() is 100, we just use the "normal" movement cost;
 * if it's 200, movement takes twice as long, etc.
 */
    int base_move_cost = map->move_cost(x, y, z);
    if (has_trait(TRAIT_FLEET) && base_move_cost == 100) {
      base_move_cost = 90;
    }
    int ap_cost = (base_move_cost * get_movement_cost()) / 100;
    use_ap(ap_cost);
  }
// Now add the furniture we're dragged to its new location
  for (int i = 0; i < dragged.size(); i++) {
    map->add_furniture(dragged[i].furniture, pos + dragged[i].pos);
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
  use_ap(100);
}

void Entity::fall(int levels)
{
// TODO: Overload this for the monster (maybe they can fly!)
// TODO: Overload this for the player (mutations & bionics might help!)
  if (levels <= 0) {
    return;
  }

  int num   =  6;
  int sides = (levels + 1) * (levels + 1);

  int fall_ability = stats.dexterity - 2;
  fall_ability += rng(0, skills.get_level(SKILL_DODGE));


  if (fall_ability >= 20) {
    num = 3;
  } else if (fall_ability >= 15) {
    num = 4;
  } else if (fall_ability >= 10) {
    num = 5;
  }

  if (levels == 1) {
    num -= 3;
  } else if (levels == 2) {
    num--;
  }

  int dam = dice(num, sides);
  std::stringstream reason;
  reason << "falling " << levels << " floors";
  take_damage_everywhere(DAMAGE_BASH, dam, reason.str());
  if (is_you()) {
// TODO: Messages about wings, etc. (Though not if we're overloaded for Player)
    GAME.add_msg("You take %d damage!", dam);
  }
}

void Entity::set_activity(Player_activity_type type, int duration,
                          int primary_uid, int secondary_uid)
{
// TODO: Error or something if we have an activity?
  activity = Player_activity(type, duration, primary_uid, secondary_uid);
}

void Entity::add_status_effect(Status_effect_type type, int duration, int level)
{
  Status_effect effect(type, duration, level);
  add_status_effect( effect );
}

void Entity::add_status_effect(Status_effect effect)
{
  if (effect.type == STATUS_NULL) {
    return;
  }
  if (effect.duration <= 0) {
    return;
  }
  for (int i = 0; i < effects.size(); i++) {
    if (effects[i].type == effect.type) {
      effects[i].boost(effect);
      return;
    }
  }
  effects.push_back(effect);
}

bool Entity::has_status_effect(Status_effect_type type)
{
  for (int i = 0; i < effects.size(); i++) {
    if (effects[i].type == type) {
      return true;
    }
  }
  return false;
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
  plan.shift(shiftx, shifty);
}

void Entity::start_turn()
{
// Move food/water from stomach to body
  if (GAME.minute_timer(1)) {
    if (stomach_food > 0) {
      int move = (stomach_food + 1) / 2;
      hunger -= move;
      stomach_food -= move;
      int min = get_hunger_minimum();
      if (hunger < min) {
        hunger = min;
      }
    }
    if (stomach_water > 0) {
      int move = (stomach_water + 1) / 2;
      thirst -= move;
      stomach_water -= move;
      int min = get_thirst_minimum();
      if (thirst < min) {
        thirst = min;
      }
    }
  }
// Increment hunger, thirst, and fatigue when appropriate...
  int hunger_interval = 6, thirst_interval = 6, fatigue_interval = 6;
  if (has_trait(TRAIT_LIGHT_EATER)) {
    hunger_interval += 2;
  }
  if (has_trait(TRAIT_CAMEL)) {
    thirst_interval += 2;
  }
  if (GAME.minute_timer(hunger_interval)) {
    hunger++;
  }
  if (GAME.minute_timer(thirst_interval)) {
    thirst++;
  }
  if (GAME.minute_timer(fatigue_interval)) {
    fatigue++;
  }

  process_status_effects();
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
  for (int i = 0; i < weapon.contents.size(); i++) {
    if (weapon.contents[i].get_uid() == uid) {
      return &(weapon.contents[i]);
    }
  }
// Can't wear containers, right?  Don't need to check their contents.
  for (int i = 0; i < items_worn.size(); i++) {
    if (items_worn[i].get_uid() == uid) {
      return &(items_worn[i]);
    }
  }
  for (int i = 0; i < inventory.size(); i++) {
    if (inventory[i].get_uid() == uid) {
      return &(inventory[i]);
    }
    for (int n = 0; n < inventory[i].contents.size(); n++) {
      if (inventory[i].contents[n].get_uid() == uid) {
        return &(inventory[i].contents[n]);
      }
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
    for (int n = 0; n < inventory[i].contents.size(); n++) {
      if (inventory[i].contents[n].type == type) {
        return &(inventory[i].contents[n]);
      }
    }
  }
// TODO: Weapon & armor?
  return NULL;
}

Item Entity::remove_item(Item* it, int uid, int count)
{
  Item ret = Item();
// Sanity check
  if (it == NULL && uid < 0) {
    return ret;
  }
  int ret_count = count;
  if (weapon.is_real() && (&weapon == it || weapon.get_uid()) == uid) {
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
  for (int i = 0; i < weapon.contents.size(); i++) {
    Item* content = &(weapon.contents[i]);
    if (content->is_real() && (content == it || content->get_uid() == uid)) {
      if (count == 0) {
        ret_count = content->count;
      } else if (content->count < count) {
        ret_count = content->count;
      }
      content->count -= count;
      ret = (*content);
      ret.count = ret_count;
      if (content->count <= 0 || count == 0) {
        weapon.contents.erase( weapon.contents.begin() + i );
      }
      return ret;
    }
  }
// Items_worn should never have a count (or contents?).
  for (int i = 0; i < items_worn.size(); i++) {
    if (&(items_worn[i]) == it || items_worn[i].get_uid() == uid) {
      ret = items_worn[i];
      ret.count = 1;
      items_worn.erase(items_worn.begin() + i);
      return ret;
    }
  }
// Check inventory
  for (int i = 0; i < inventory.size(); i++) {
    if ( &(inventory[i]) == it || inventory[i].get_uid() == uid) {
      if (count == 0) {
        ret_count = inventory[i].count;
      } else if (inventory[i].count < count) {
        ret_count = inventory[i].count;
      }
      ret = inventory[i];
      inventory[i].count--;
      ret.count = ret_count;
      if (count == 0 || inventory[i].count <= 0) {
        inventory.erase(inventory.begin() + i);
      }
      return ret;
    }
// Check contents, too!
    for (int n = 0; n < inventory[i].contents.size(); n++) {
      Item* content = &(inventory[i].contents[n]);
      if (content->is_real() && (content == it || content->get_uid() == uid)) {
        if (count == 0) {
          ret_count = content->count;
        } else if (content->count < count) {
          ret_count = content->count;
        }
        ret = (*content);
        ret.count = ret_count;
        content->count--;
        if (count == 0 || content->count <= 0) {
          inventory[i].contents.erase( inventory[i].contents.begin() + n );
        }
      }
      return ret;
    }
  }
  return ret;
}

Item Entity::remove_item_ref(Item* it, int count)
{
  return remove_item(it, -1, count);
}

Item Entity::remove_item_uid(int uid, int count)
{
  return remove_item(NULL, uid, count);
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
// TODO: Don't hardcode this.
  use_ap(300);
}

void Entity::take_off_item_uid(int uid)
{
  for (int i = 0; i < items_worn.size(); i++) {
    if (items_worn[i].get_uid() == uid) {
      add_item( items_worn[i] );
      items_worn.erase( items_worn.begin() + i );
      use_ap(300);  // TODO: Don't hardcode this.
    }
  }
}

void Entity::apply_item_uid(int uid)
{
// Find the item.
  Item* it;
  if (weapon.is_real() && weapon.get_uid() == uid) {
    it = &weapon;
  }
  for (int i = 0; i < inventory.size(); i++) {
    if (inventory[i].get_uid() == uid) {
      it = &(inventory[i]);
    }
  }
// Return for item not found or item isn't a tool
  if (!it || it->get_item_class() != ITEM_CLASS_TOOL) {
    return;
  }

// Get the tool information
  Item_type_tool* tool   = static_cast<Item_type_tool*>(it->type);
  Tool_action* action    = &(tool->applied_action);
  Tool_action* powered   = &(tool->powered_action);
  Tool_action* countdown = &(tool->countdown_action);
  //Tool_action* countdown = &(tool->countdown_action);
// Apply the base action, if any
  if (action->real) {
    apply_item_action(it, action);
    if (action->destroy_if_chargeless && it->charges == 0) {
      remove_item_uid(it->get_uid());
    }
  }

// We have an effect that happens while we're powered on; so power us on!
  if (powered->real) {
    if (it->charges == 0 && it->subcharges == 0) {
      if (is_you()) {
        GAME.add_msg("Your %s has no charges.", it->get_name().c_str());
      }
      return;
    }
    if (it->is_active() && it->power_off()) {
      use_ap(25);  // TODO: Don't hardcode this value.
    } else if (it->power_on()) {
      use_ap(25);  // TODO: Don't hardcode this value.
    }
  }

// We have an effect that triggers after a countdown; so start the countdown!
  if (countdown->real && !it->is_active()) {
    if (it->start_countdown()) {
      use_ap(25);  // TODO: Don't hardcode this value.
    }
  }
}

void Entity::apply_item_action(Item* it, Tool_action* action)
{
// Sanity check
  if (!it || !action || it->get_item_class() != ITEM_CLASS_TOOL) {
    return;
  }
  Item_type_tool* tool = static_cast<Item_type_tool*>(it->type);
// Verify that we have enough charges.
  if (tool->uses_charges() && it->charges < action->charge_cost) {
    return;
  }

// Pick a target, if applicable.
  Tripoint tool_pos = pick_target_for(it);
  if (action->target != TOOL_TARGET_NULL && tool_pos.x == -1) {  // We canceled
    return;
  }

  if (!action->activate(it, this, tool_pos)) {
    return;
  }

// Use charges and AP.
  if (tool->uses_charges()) {
    it->charges -= action->charge_cost;
  }
  use_ap(action->ap_cost);

}

bool Entity::eat_item_uid(int uid)
{
  Item* it = ref_item_uid(uid);
  if (!it) {
    return false;
  }
  if (it->get_item_class() != ITEM_CLASS_FOOD) {
// Try the contents, too!
    bool ate_something = false;
    for (int i = 0; !ate_something && i < it->contents.size(); i++) {
      if (eat_item_uid( it->contents[i].get_uid() )) {
        ate_something = true;
      }
    }
    return ate_something;
  }
  bool can_add_message = is_you();
  Item_type_food* food = static_cast<Item_type_food*>(it->type);
  stomach_food += food->food;
  if (stomach_food > get_stomach_maximum()) {
    stomach_food = get_stomach_maximum();
    if (can_add_message) {
      GAME.add_msg("You can't finish it all!");
      can_add_message = false;
    }
  }

  stomach_water += food->water;
  if (stomach_water > get_stomach_maximum()) {
    stomach_water = get_stomach_maximum();
    if (can_add_message) {
      GAME.add_msg("You can't finish it all!");
    }
  }

  if (food->effect.type != STATUS_NULL &&
      (food->effect_chance >= 100 || rng(1, 100) <= food->effect_chance)) {
    Status_effect effect = food->effect;
    if (has_trait(TRAIT_LIGHTWEIGHT)) {
      effect.duration *= 1.33;
      if (effect.level > 1 && rng(1, 5) < effect.level) {
        effect.level++;
      }
    }
    add_status_effect( effect );
  }

  it->charges--;
  if (it->charges <= 0) {
    remove_item_uid(uid, 1);
  }
// TODO: Variable eating/drinking times?
  use_ap(100);
  return true;
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

// Overloaded for Player
Tripoint Entity::pick_target_for(Item *it)
{
  return Tripoint(-1, -1, -1);
}

bool Entity::is_wielding_item_uid(int uid)
{
  return is_wielding_item(NULL, uid);
}

bool Entity::is_wielding_item(Item* it, int uid)
{
  if (it == NULL && uid < 0) {
    return false;
  }
  return (weapon.is_real() && (&weapon == it || weapon.get_uid() == uid));
}

bool Entity::is_wearing_item_uid(int uid)
{
  return is_wearing_item(NULL, uid);
}

bool Entity::is_wearing_item(Item* it, int uid)
{
  if (it == NULL && uid < 0) {
    return false;
  }
  for (int i = 0; i < items_worn.size(); i++) {
    if (items_worn[i].is_real() && 
        (&(items_worn[i]) == it || items_worn[i].get_uid() == uid)) {
      return true;
    }
  }
  return false;
}

bool Entity::is_wearing_item_flag(Item_flag flag)
{
  for (int i = 0; i < items_worn.size(); i++) {
    if (items_worn[i].has_flag(flag)) {
      return true;
    }
  }
  return false;
}

bool Entity::is_carrying_item_uid(int uid)
{
  return is_carrying_item(NULL, uid);
}

bool Entity::is_carrying_item(Item* it, int uid)
{
  if (it == NULL && uid < 0) {
    return false;
  }
  for (int i = 0; i < inventory.size(); i++) {
    if (inventory[i].is_real() &&
        (&(inventory[i]) == it || inventory[i].get_uid() == uid)) {
      return true;
    }
  }
  return false;
}

bool Entity::has_item_uid(int uid)
{
  return has_item(NULL, uid);
}

bool Entity::has_item(Item* it, int uid)
{
  return (is_wielding_item(it, uid) || is_wearing_item(it, uid) ||
          is_carrying_item(it, uid));
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

std::string Entity::take_off_item_message(Item &it)
{
  int uid = it.get_uid();
  if (!is_wearing_item_uid(uid)) {
    return "You're not wearing that.";
  }
  std::stringstream ret;
  ret << get_name_to_player() << " " << conjugate("take") << " off " <<
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

std::string Entity::apply_item_message(Item &it)
{
  int uid = it.get_uid();
  std::stringstream ret;
  if (!it.is_real() || !ref_item_uid(uid)) {
    ret << get_name_to_player() << " don't have that item.";
  } else if (it.get_item_class() != ITEM_CLASS_TOOL) {
    ret << get_name_to_player() << " cannot apply that.";
  } else {
    Item_type_tool* tool = dynamic_cast<Item_type_tool*>(it.type);
    Tool_action* action = &(tool->applied_action);
    if (tool->uses_charges() && it.charges < action->charge_cost) {
      ret << get_possessive() << " " << it.get_name() << " doesn't have " <<
             (it.charges == 0 ? "any" : "enough");
      if (tool->fuel.empty()) {
        ret << " charges";
      } else {
        ret << tool->fuel;
      }
      ret << ".";
    } else {
      ret << get_name_to_player() << " " << conjugate("use") << " " <<
             get_possessive() << " " << it.get_name() << ".";
    }
  }
  return ret.str();
}

std::string Entity::eat_item_message(Item &it)
{
  int uid = it.get_uid();
  std::stringstream ret;
  if (!it.is_real() || !ref_item_uid(uid)) {
    ret << get_name_to_player() << " don't have that item.";
  } else if (it.get_item_class() != ITEM_CLASS_FOOD) {
// Check the contents!
    for (int i = 0; i < it.contents.size(); i++) {
      if (it.contents[i].get_item_class() == ITEM_CLASS_FOOD) {
        return eat_item_message(it.contents[i]);
      }
    }
// Couldn't find anything to eat :(
    ret << get_name_to_player() << " cannot eat that.";
  } else {
    Item_type_food* food = static_cast<Item_type_food*>(it.type);
    std::string verb = food->verb;
    ret << "<c=ltblue>" << get_name_to_player() << " " << conjugate(verb) <<
           " " << get_possessive() << " " << it.get_name() << ".<c=/>";
  }
  return ret.str();
}

std::string Entity::advance_fire_mode_message()
{
  if (!weapon.is_real()) {
    return "You're not wielding anything.";
  }
  if (weapon.get_item_class() != ITEM_CLASS_LAUNCHER) {
    std::stringstream ret;
    ret << "That " << weapon.get_name() << " is not a firearm.";
    return ret.str();
  }
/* This is an unfortunate requirement, but when this function is called, it must
 * be *AFTER* we actually advance the weapon's firing mode, so that the correct
 * number of shots will be cited here.
 */
  std::stringstream ret;
  Item_type_launcher* launcher = static_cast<Item_type_launcher*>(weapon.type);
  int num_shots = weapon.get_shots_fired();
  if (launcher->modes.size() <= 1) {
    ret << "Your " << weapon.get_name() << " can only fire ";
  } else {
    ret << "Your " << weapon.get_name() << " now fires ";
  }
  if (num_shots <= 1) {
    ret << "single rounds.";
  } else {
    ret << num_shots << "-round bursts.";
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

std::string Entity::get_dragged_name()
{
  if (dragged.empty()) {
    return "";  // TODO: return "nothing"; ???
  }
  return dragged[0].furniture.get_name();
}

std::string Entity::get_all_status_text()
{
  std::stringstream ret;
  ret << get_hunger_text() << " " << get_thirst_text() << " " <<
         get_pain_text();
  return ret.str();
/*

  return get_hunger_text() + " " + get_thirst_text() + " " + get_pain_text();
  std::string ret = get_hunger_text();

  std::string thirst_text = get_thirst_text();
  if (!thirst_text.empty()) {
    if (!ret.empty()) {
      ret += " ";
    }
    ret += thirst_text;
  }

  std::string pain_text = get_pain_text();
  if (!pain_text.empty()) {
    if (!ret.empty()) {
      ret += " ";
    }
    ret += pain_text;
  }

  return ret;
*/
}

std::string Entity::get_hunger_text()
{
// If hunger isn't high enough to be hungry, base it on stomach contents
  if (hunger < 60) {
    if (stomach_food >= get_stomach_maximum() / 2) {
      return "<c=green>Overfull<c=/>";
    } else if (hunger < 0) {
      return "<c=ltgreen>Full<c=/>";
    }
    return "";
  }
  if (hunger < 120) {
    return "<c=yellow>Hungry<c=/>";
  }
  if (hunger < 240) {
    return "<c=yellow>Very Hungry<c=/>";
  }
  if (hunger < 480) {
    return "<c=ltred>Famished<c=/>";
  }
  if (hunger < 960) {
    return "<c=red>Near Starving<c=/>";
  }
  return "<c=red>Starving<c=/>";
}

std::string Entity::get_thirst_text()
{
  if (thirst < get_thirst_minimum() / 2) {
    return "<c=green>Quenched<c=/>";
  }
  if (thirst < 0) {
    return "<c=ltgreen>Hydrated<c=/>";
  }
  if (thirst < 60) {
    return "";
  }
  if (thirst < 90) {
    return "<c=yellow>Thirsty<c=/>";
  }
  if (thirst < 120) {
    return "<c=yellow>Very Thirsty<c=/>";
  }
  if (thirst < 240) {
    return "<c=ltred>Parched<c=/>";
  }
  if (thirst < 360) { 
    return "<c=red>Dehydrated<c=/>";
  }
  return "<c=red>Dying of thirst<c=/>";
}

std::string Entity::get_fatigue_text()
{
  if (fatigue < 160) {
    return "";
  }
  if (fatigue < 200) {
    return "<c=yellow>Tired<c=/>";
  }
  if (fatigue < 240) {
    return "<c=yellow>Very Tired<c=/>";
  }
  if (fatigue < 280) {
    return "<c=ltred>Severely Tired<c=/>";
  }
  if (fatigue < 320) {
    return "<c=ltred>Exahusted<c=/>";
  }
  if (fatigue < 360) {
    return "<c=ltred>Nearly Fainting<c=/>";
  }
  return "<c=red>Fainting<c=/>";
}

std::string Entity::get_pain_text()
{
  if (pain == 0) {
    return "";
  } else if (pain < 20) {
    return "<c=ltgray>Minor Pain<c=/>";
  } else if (pain < 40) {
    return "<c=yellow>Pain<c=/>";
  } else if (pain < 60) {
    return "<c=yellow>Heavy Pain<c=/>";
  } else if (pain < 100) {
    return "<c=ltred>Severe Pain<c=/>";
  } else if (pain < 160) {
    return "<c=ltred>Agonizing Pain<c=/>";
  }
  return "<c=red>Excrutiating Pain!<c=/>";
}

Attack Entity::base_attack()
{
  Attack ret;
  ret.damage[DAMAGE_BASH] = 1 + stats.strength / 4;
  ret.speed  = 100 - stats.dexterity;
  ret.to_hit = stats.dexterity / 4 - 3;
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

  att.adjust_with_stats (stats);
  att.adjust_with_skills(skills);

  use_ap(att.speed);

  bool you_see = GAME.player->can_sense(GAME.map, pos.x, pos.y);
  bool attacker_is_you = is_you();

  std::string miss_verb = conjugate("miss");

  int hit_sum = hit_roll(att.to_hit) - target->dodge_roll();

  if (hit_sum < 0) {
// If player can see it, construct a message regarding missing
    if (you_see) {
      std::stringstream msg;
      msg << "<c=dkgray>" << get_name_to_player() << " " << miss_verb << " " <<
             target->get_name_to_player() << "<c=/>.";
      GAME.add_msg( msg.str() );
    }
    int miss_penalty = rng(0, att.speed);
    use_ap(miss_penalty);
    return;
  }

  Melee_hit_type hit_type = MELEE_HIT_NORMAL;
// TODO: This could mess up if, later on, we restrict Entities to one dodge roll
//       per turn.  So, make it free_dodge_roll() or something.
  if (hit_sum < target->dodge_roll()) {
    hit_type = MELEE_HIT_GRAZE;
  } else if (hit_sum - target->dodge_roll() - target->dodge_roll() > 1) {
    hit_type = MELEE_HIT_CRITICAL;
  }

  Body_part bp_hit = (target->is_player() ? random_body_part_to_hit() :
                                            BODY_PART_NULL);

// Pick a damage type to use, based on its max damage - target's armor

  std::vector<Damage_type> damtype_ties;
  int dam_to_beat = -999;

  for (int i = 0; i < DAMAGE_MAX; i++) {
    Damage_type dtype = Damage_type(i);
    int damage_check = att.damage[dtype];
    damage_check -= target->get_armor(dtype, bp_hit);
// best_damtypes is a list of ties; so if we beat it outright, clear the list
    if (damage_check > dam_to_beat) {
      damtype_ties.clear();
      dam_to_beat = damage_check;
    }
    if (damage_check >= dam_to_beat) {
      damtype_ties.push_back(dtype);
    }
  }

  Damage_type best_damtype = damtype_ties[ rng(0, damtype_ties.size() - 1) ];

  int dam = att.roll_damage_type(best_damtype, hit_type);
/* We absorb damage (via armor) here, then use take_damage_no_armor(), because
 * we want the damage that's displayed (below) to be the actual, post-armor
 * damage.  That's much better feedback for telling the player that their uber-
 * sword isn't working so great against metal-armored creature.
 */
  target->absorb_damage(best_damtype, dam, bp_hit);
  target->take_damage_no_armor(best_damtype, dam, get_name_to_player(), bp_hit);

// Construct a message about the attack, if the player can see it.
  if (you_see) {
    std::stringstream damage_ss;
    bool capitalize_name = false;
// Special text if it's critical/grazing.
    switch (hit_type) {
      case MELEE_HIT_CRITICAL:
        damage_ss << "<c=yellow>Critical! ";
        capitalize_name = true;
        break;
      case MELEE_HIT_GRAZE:
        damage_ss << "<c=blue>Grazing hit. ";
        capitalize_name = true;
        break;
    }
// Color the rest of the message.
    if (dam == 0) {
      damage_ss << "<c=dkgray>";
    } else {
      damage_ss << "<c=ltred>";
    }
// Message looks like "(You) (Hit) (The Zombie)['s head] for (damage) damage!"
    if (capitalize_name) {
      damage_ss << capitalize( get_name_to_player() );
    } else {
      damage_ss << get_name_to_player();
    }
    damage_ss << " ";
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
    if (dam == 0) {
      damage_ss << " but " << (is_you() ? "do" : "does") << " no damage.";
    } else if (target->is_you() || TESTING_MODE) {
      damage_ss << " for " << dam << " damage!";
    } else {
      damage_ss << ".";
    }
// Close coloration.
    damage_ss << "<c=/>";
    GAME.add_msg( damage_ss.str() );
  }
}
  
int Entity::hit_roll(int bonus)
{
  return rng(1, 10 + bonus);
}

int Entity::dodge_roll()
{
  return rng(1, 10);
}

// This one gets overloaded fully.
void Entity::take_damage(Damage_type damtype, int damage, std::string reason,
                         Body_part part)
{
}

// This one gets overloaded fully.
void Entity::take_damage_no_armor(Damage_type damtype, int damage,
                                  std::string reason, Body_part part)
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

// Since most entities don't use body parts, this should be functionally
// equivalent to take_damage()
void Entity::take_damage_everywhere(Damage_set damage, std::string reason)
{
  take_damage(damage, reason, BODY_PART_NULL);
}

void Entity::take_damage_everywhere(Damage_type type, int damage,
                                    std::string reason)
{
  take_damage(type, damage, reason, BODY_PART_NULL);
}

void Entity::absorb_damage(Damage_type damtype, int& damage, Body_part part)
{
  for (int i = 0; damage > 0 && i < items_worn.size(); i++) {
    if (items_worn[i].covers(part)) {
      items_worn[i].absorb_damage(damtype, damage);
    }
  }
}

// Overridden in monster.cpp and player.cpp
void Entity::heal_damage(int damage, HP_part part)
{
}

// Overridden in monster.cpp and player.cpp
int Entity::get_armor(Damage_type damtype, Body_part part)
{
  return 0;
}

Ranged_attack Entity::throw_item(Item it)
{
  Ranged_attack ret = it.get_thrown_attack(this);
  return ret;
}

Ranged_attack Entity::fire_weapon()
{
  if (!weapon.is_real() || weapon.get_item_class() != ITEM_CLASS_LAUNCHER ||
      !weapon.ammo || weapon.charges == 0) {
    return Ranged_attack();
  }
  weapon.charges -= weapon.get_shots_fired();
  use_ap( weapon.time_to_fire() );
  Ranged_attack ret = weapon.get_fired_attack();
// Perception applies a flat bonus/penalty to accuracy
  ret.variance -= 3 * (stats.perception - 10);
// If we've got skill in the weapon's skill type, then reduce its variance
  Item_type_launcher* launcher = static_cast<Item_type_launcher*>(weapon.type);
  Skill_type sk_used = launcher->skill_used;
  int my_skill = (skills.get_level(SKILL_LAUNCHERS) +
                  skills.get_level(sk_used) * 3      ) / 4;
  ret.variance.number -= my_skill / 3;
  if (rng(1, 3) <= my_skill % 3) {
    ret.variance.number--;
  }
  if (ret.variance.number < 0) {
    ret.variance.number = 1;
    ret.variance.sides /= 2;
  }
// Also, add some variance based on LOW skill
  Dice extra_variance;
  extra_variance.number = 3;
  extra_variance.sides  = (20 - my_skill);
  extra_variance.bonus  = -3;
  ret.variance += extra_variance;
  
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
    GAME.add_msg("<c=ltred>%s %s at %s!<c=/>", get_name_to_player().c_str(),
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

bool Entity_pool::empty()
{
  return instances.empty();
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

// range defaults to -1, which means "no range cap"
Entity* Entity_pool::closest_seen_by(Entity* observer, int range)
{
  if (!observer) {
    return NULL;
  }
  Tripoint pos = observer->pos;
  int best_range = range;
  Entity* ret = NULL;
  for (std::list<Entity*>::iterator it = instances.begin();
       it != instances.end();
       it++) {
    Entity* target = *it;
    int dist = rl_dist(pos, target->pos);
    if ( target != observer && (best_range == -1 || dist <= best_range) &&
         GAME.map->senses(pos, target->pos, range, SENSE_SIGHT) ) {
      best_range = dist;
      ret = target;
    }
  }
  return ret; // Might be NULL!
}
