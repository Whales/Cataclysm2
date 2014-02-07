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
  special_timer = 0;
}

void Monster::set_type(std::string name)
{
  set_type(MONSTER_TYPES.lookup_name(name));
}

void Monster::set_type(Monster_type* newtype)
{
  dead = false;
  killed_by_player = false;
  type = newtype;
  if (type) {
    current_hp = type->hp_dice.roll();
  } else {
    debugmsg("Monster::set_type(NULL)!");
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

std::string Monster::get_data_name()
{
  if (type) {
    return type->get_data_name();
  }
  return "Typeless Monster";
}

std::string Monster::get_name()
{
  if (type) {
    return type->get_name();
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
// TODO: Support a/an
// TODO: For unique monsters - redirect to definite=
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

Entity_AI Monster::get_AI()
{
  if (!type) {
    return Entity_AI();
  }
  return type->AI;
}

int Monster::get_genus_uid()
{
  if (!type || !type->genus) {
    return -1;
  }
  return type->genus->uid;
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
// TODO: Don't hard-code this, add a "don't cancel plan" function or something
  if (plan.is_active() && plan.goal_type == AIGOAL_FLEE) {
    return;
  }
// TODO: Support different senses
// TODO: Don't hard-code for player; instead select a target from Game.entities
// Iterate through our AI's goals until we find one we can try for
  Entity_AI AI = get_AI();
  bool found_goal = false;
  for (int i = 0; !found_goal && i < AI.goals.size(); i++) {
    found_goal = try_goal(AI.goals[i]);
  }
  if (!found_goal) {
// TODO: Straight line / pinball wandering
  } else {
    plan.generate_path_to_target(AI, pos);
  }
}

bool Monster::try_goal(AI_goal goal)
{
  switch (goal) {

    case AIGOAL_NULL: // Shouldn't happen
      return false;

    case AIGOAL_ATTACK_ENEMIES:
    case AIGOAL_ATTACK_NEUTRALS:
      return pick_attack_victim();

    case AIGOAL_FLEE:
      return pick_flee_target();

    case AIGOAL_EAT_CORPSES:  // TODO: This.
      return false;

    case AIGOAL_COLLECT_ITEMS: // TODO: This.
      return false;

    default:
      debugmsg("Monster::try_goal doesn't know how to handle %s!",
               AI_goal_name(goal).c_str());
      return false;

  }
  return false;
}

bool Monster::pick_attack_victim()
{
// TODO: Include an estimate of the target's strength relative to our own
// TODO: Differentiate between "attacking aggresors" and "preying upon randos"
  int closest = 0;
  std::vector<Entity*> best;
  Entity_pool *pool = &(GAME.entities);
  for (std::list<Entity*>::iterator it = pool->instances.begin();
       it != pool->instances.end();
       it++) {
    Entity* tmp = (*it);
// Not us, not in our genus, and we can sense them
    if (tmp->uid != uid && tmp->get_genus_uid() != get_genus_uid() &&
        can_sense(tmp)) {
      int dist = rl_dist(pos, tmp->pos);
      if (closest == 0 || dist < closest) {
        closest = dist;
        best.clear();
        best.push_back(tmp);
      } else if (dist == closest) {
        best.push_back(tmp);
      }
    }
  }

  if (best.empty()) {
    return false;
  }
  int index = rng(0, best.size() - 1);
  Entity_AI AI = get_AI();
  plan.set_target( AIGOAL_ATTACK_ENEMIES, best[index], AI.attention_span );
  return true;
}

bool Monster::pick_flee_target()
{
// TODO: This could be better.
  Tripoint flee_target;
  int map_max = MAP_SIZE * SUBMAP_SIZE - 1;
  int z_level = (pos.z >= 0 ? 0 : pos.z);
  switch (rng(1, 4)) {
    case 1:
      flee_target = Tripoint(0      , 0      , z_level);
      break;
    case 2:
      flee_target = Tripoint(0      , map_max, z_level);
      break;
    case 3:
      flee_target = Tripoint(map_max, 0      , z_level);
      break;
    case 4:
      flee_target = Tripoint(map_max, map_max, z_level);
      break;
  }
  Entity_AI AI = get_AI();
  plan.set_target( AIGOAL_FLEE, flee_target, AI.attention_span * 3 );
  return true;
}

std::vector<Ranged_attack> Monster::get_ranged_attacks()
{
  if (!type) {
    return std::vector<Ranged_attack>();
  }
  return type->ranged_attacks;
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
  plan.update();
  if (special_timer > 0) {
    special_timer--;
  }
  if (plan.target_entity && can_attack(plan.target_entity)) {
    attack(plan.target_entity);
  } else if (special_timer <= 0 && plan.target_entity &&
             can_attack_ranged(plan.target_entity)) {
    attack_ranged(plan.target_entity, pick_ranged_attack(plan.target_entity));
  } else if (can_move_to(GAME.map, plan.next_step())) {
    if (rl_dist(pos, plan.next_step()) > 1) {
      debugmsg("Monster %s jumping %d steps", get_name().c_str(),
               rl_dist(pos, plan.next_step()));
    }
    move_to(GAME.map, plan.next_step());
    plan.erase_step();
  } else if (can_smash(GAME.map, plan.next_step())) {
    smash(GAME.map, plan.next_step());
  } else {
/*
    Most likely, an entity is in our way
    TODO: Check if we want to attack that enemy
    Tripoint next = plan.next_step();
    plan.generate_path_to_target(get_AI(), pos);
*/
    pause();
  }
}

bool Monster::can_sense(Entity* entity)
{
  if (!entity) {
    return false;
  }
// Do it in order of lowest resource cost to highest!
  if (has_sense(SENSE_OMNISCIENT)) {
    return true;
  }
// TODO: require that the target is warm-blooded
  if (has_sense(SENSE_INFRARED)) {
    return true;
  }
// TODO: Use a range other than 15
  if (has_sense(SENSE_SIGHT)) {
    return GAME.map->senses(pos, entity->pos, 15, SENSE_SIGHT);
  }
// TODO: Other senses (e.g. echolocation)
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
/* TODO:  This is inefficient.  Maybe make a wrapper struct for damage reasons,
 *        which has a flag "is_the_player"?
 */
    if (reason.find("you") != std::string::npos) {
      killed_by_player = true;
    }
  }
}

// TODO: Rewrite this function.
void Monster::wander()
{
  std::vector<Tripoint> open_points;
  for (int x = pos.x - 1; x <= pos.x + 1; x++) {
    for (int y = pos.y - 1; y <= pos.y + 1; y++) {
      if (can_move_to(GAME.map, x, y, pos.z)) {
        open_points.push_back( Tripoint(x, y, pos.z) );
      }
    }
  }
  if (open_points.empty()) {
    pause();
  } else {
    move_to(GAME.map, open_points[ rng(0, open_points.size() - 1) ]);
  }
}

void Monster::pause()
{
  action_points = 0;
}

