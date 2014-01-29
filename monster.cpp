#include "monster.h"
#include "rng.h"
#include "game.h"
#include "player.h"
#include <sstream>

Monster::Monster()
{
  dead = false;
  killed_by_player = false;
  current_hp = 0;
  type = NULL;
  action_points = 0;
}

void Monster::set_type(std::string name)
{
  dead = false;
  killed_by_player = false;
  type = MONSTER_TYPES.lookup_name(name);
  if (type) {
    current_hp = rng(type->minimum_hp, type->maximum_hp);
  }
  action_points = 0;
}

Monster::~Monster()
{
}

glyph Monster::get_glyph()
{
  if (type) {
    return type->sym;
  }
  return glyph();
}

std::string Monster::get_name()
{
  if (type) {
    return type->name;
  }
  return "Typeless Monster";
}

std::string Monster::get_name_to_player()
{
  return get_name_definite();
}

std::string Monster::get_possessive()
{
  std::stringstream ret;
  ret << get_name_definite() << "'s";
  return ret.str();
}

std::string Monster::get_name_indefinite()
{
// TODO: more complicated
  std::stringstream ret;
  ret << "a " << get_name();
  return ret.str();
}

std::string Monster::get_name_definite()
{
  std::stringstream ret;
  ret << "the " << get_name();
  return ret.str();
}

bool Monster::has_sense(Sense_type sense)
{
  if (!sense) {
    return false;
  }
  return type->has_sense(sense);
}

Intel_level Monster::get_intelligence()
{
  if (!type) {
    return INTEL_NULL;
  }
  return type->intel;
}

int Monster::get_speed()
{
  if (!type) {
    return 0;
  }
  return type->speed;
}

void Monster::make_plans()
{
  Player *player = GAME.player;
  Map *map = GAME.map;
// TODO: Support different senses
// TODO: Support non-aggressive monsters
// TODO: Don't hard-code for player; instead select a target from Game.entities
  bool senses_player = false;
  if (has_sense(SENSE_SIGHT) && can_sense(map, player->pos)) {
    senses_player = true;
  }
  if (senses_player) {
    plan.set_target(player);
  }
}

void Monster::take_turn()
{
  if (action_points <= 0 || dead) {
    return;
  }
// TODO: Move make_plans() outside of this function?
  make_plans();
  if (!plan.is_active()) {
    wander();
    return;
  }
  plan.attention--;
  if (plan.target_entity && can_attack(plan.target_entity)) {
    attack(plan.target_entity);
  } else {
    move_towards(plan.target_point);
  }
}

bool Monster::can_attack(Entity* entity)
{
  if (!entity) {
    return false;
  }
  if (rl_dist(pos.x, pos.y, pos.z, entity->pos.x, entity->pos.y, entity->pos.z) <= 1){
    return true;
  }
  return false;
}

Attack Monster::base_attack()
{
  if (!type || type->attacks.empty()) {
    return Attack();
  }
  int index = rng(1, type->total_attack_weight);
  for (int i = 0; i < type->attacks.size(); i++) {
    index -= type->attacks[i].weight;
    if (index <= 0) {
      return type->attacks[i];
    }
  }
  return type->attacks.back();
}

void Monster::take_damage(Damage_type type, int damage, std::string reason,
                          Body_part part)
{
  current_hp -= damage;
  if (current_hp <= 0) {
    dead = true;
    if (reason == "you") {
      killed_by_player = true;
    }
  }
}

void Monster::move_towards(Entity* entity)
{
  if (!entity) {
    debugmsg("Monster attempted move_towards() on a null target.");
    return;
  }
  move_towards(entity->pos);
}

void Monster::move_towards(int target_x, int target_y)
{
  Tripoint target(target_x, target_y, pos.z);
  move_towards(target);
}

void Monster::move_towards(Tripoint target)
{
  Generic_map move_map = GAME.map->get_movement_map(get_intelligence());
  Pathfinder pf(move_map);
// Simple, dumb movement - suitable for zombies at least
  Tripoint move = pos;
  switch (get_intelligence()) {
    case INTEL_NULL:
    case INTEL_PLANT:
// Mobile plants just drunken walk.
      move.x = rng(pos.x - 1, pos.x + 1);
      move.y = rng(pos.y - 1, pos.y + 1);
      break;

    case INTEL_ZOMBIE:
      move = pf.get_step(PATH_LINE, pos, target);
      break;

    case INTEL_ANIMAL:
    case INTEL_HUMAN:
      move = pf.get_step(PATH_A_STAR, pos, target);
      break;

    default:
      debugmsg("No AI movement coded for Intel_level %s",
               intel_level_name(get_intelligence()).c_str());
  }

// TODO:  Add a "Stumble" flag that occasionally randomly picks, rather than
//        picking the best available.

  if (can_move_to( GAME.map, move )) {
    move_to( GAME.map, move );
// TODO: Add a "smashes terrain" flag, and if we can't move then smash
  } else if (GAME.map->is_smashable(move)) {
    std::string sound = GAME.map->smash(move, base_attack().roll_damage());
    GAME.make_sound(sound, move);
    use_ap(100);
  } else {
    pause();
  }
}

void Monster::wander()
{
  if (wander_duration <= 0) {
    wander_target = Tripoint( pos.x + rng(-3, 3), pos.y + rng(-3, 3), pos.z );
    wander_duration = 3;
  }
  move_towards(wander_target);
}

void Monster::pause()
{
  action_points = 0;
}

void Monster::die()
{
    Item tmp ( type->corpse );
    GAME.map->add_item(tmp, pos);
    Entity::die();
}

