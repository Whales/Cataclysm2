#ifndef _ENTITY_AI_H_
#define _ENTITY_AI_H_

#include "geometry.h"
#include <string>
#include <istream>
#include <vector>

enum Pathing_feature
{
  PATHFEAT_NULL = 0,
  PATHFEAT_FIELDS,    // Awareness & avoidance of fields
  PATHFEAT_MAX
};

Pathing_feature lookup_pathing_feature(std::string name);
std::string pathing_feature_name(Pathing_feature feat);

enum AI_goal
{
  AIGOAL_NULL = 0,
  AIGOAL_ATTACK_ENEMIES,  // Attack those which are hostile to us
  AIGOAL_ATTACK_NEUTRALS, // Attack neutral entities (e.g. player)
  AIGOAL_FLEE,            // Flee from danger
  AIGOAL_EAT_CORPSES,     // Path to & eat corpses
  AIGOAL_COLLECT_ITEMS,   // Path to & collect items
/* TODO:
   Follow player
   Maintain distance
 */
  AIGOAL_MAX
};

AI_goal lookup_AI_goal(std::string name);
std::string AI_goal_name(AI_goal goal);

struct Entity_AI
{
  Entity_AI();
  ~Entity_AI();

  bool uses_feature(Pathing_feature feat) const;
  bool uses_goal(AI_goal goal) const;

  bool load_data(std::istream &data, std::string owner_name = "Unknown");

  int  area_awareness; // Extra squares considered for pathing
  int  attention_span; // Default attention level upon setting a new target
  std::vector<AI_goal> goals; // Ordered from most important to least

  Entity_AI& operator=(const Entity_AI& rhs);

private:
  bool pathing_features[PATHFEAT_MAX];
  bool goals_in_use[AIGOAL_MAX];
};

#endif
