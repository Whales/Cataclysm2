#include "entity_ai.h"
#include "stringfunc.h"
#include "window.h"

Entity_AI::Entity_AI()
{
  for (int i = 0; i < PATHFEAT_MAX; i++) {
    pathing_features[i] = false;
  }
  for (int i = 0; i < AIGOAL_MAX; i++) {
    goal_priorities[i] = 0;
  }
}

Entity_AI::~Entity_AI()
{
}

bool Entity_AI::uses_feature(Pathing_feature feat)
{
  return pathing_features[feat];
}

bool Entity_AI::uses_goal(AI_goal goal)
{
  return (goal_priorities[goal] > 0);
}

int Entity_AI::goal_priority(AI_goal goal)
{
  return goal_priorities[goal];
}

bool Entity_AI::load_data(std::istream &data, std::string parent_name)
{
  std::string ident, junk;
  while (ident != "done") {

    if ( ! (data >> ident) ) {
      return false;
    }

    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment
      std::getline(data, junk);

    } else if (ident == "awareness:" || ident == "area_awareness:") {
      data >> area_awareness;
      std::getline(data, junk);

    } else if (ident == "attention:" || ident == "attention_span:") {
      data >> attention_span;
      std::getline(data, junk);

    } else if (ident == "feature:") {
      std::string feat_name;
      std::getline(data, feat_name);
      Pathing_feature feat = lookup_pathing_feature(feat_name);
      if (feat == PATHFEAT_NULL) {
        debugmsg("Unknown pathing feature '%s' (%s)", feat_name.c_str(),
                 parent_name.c_str());
        return false;
      }
      pathing_features[feat] = true;

    } else if (ident == "priority:") {
      int prior;
      data >> prior;
      std::string goal_name;
      std::getline(data, goal_name);
      AI_goal goal = lookup_AI_goal(goal_name);
      if (goal == AIGOAL_NULL) {
        debugmsg("Unknown AI goal '%s' (%s)", goal_name.c_str(),
                 parent_name.c_str());
        return false;
      }
      goal_priorities[goal] = prior;

    } else if (ident != "done") {
      debugmsg("Unknown AI property '%s' (%s)", ident.c_str(),
               parent_name.c_str());
      return false;
    }
  }
  return true;
}

Pathing_feature lookup_pathing_feature(std::string name)
{
  name = no_caps(name);
  name = trim(name);
  for (int i = 0; i < PATHFEAT_MAX; i++) {
    Pathing_feature ret = Pathing_feature(i);
    if ( no_caps( pathing_feature_name(ret) ) == name) {
      return ret;
    }
  }
  return PATHFEAT_NULL;
}

std::string pathing_feature_name(Pathing_feature feat)
{
  switch (feat) {
    case PATHFEAT_NULL:   return "NULL";
    case PATHFEAT_FIELDS: return "fields";
    case PATHFEAT_MAX:    return "BUG - PATHFEAT_MAX";
    default:              return "BUG - Unnamed Pathing_feature";
  }
  return "BUG - Escaped pathing_feature_name switch!";
}

AI_goal lookup_AI_goal(std::string name)
{
  name = no_caps(name);
  name = trim(name);
  for (int i = 0; i < AIGOAL_MAX; i++) {
    AI_goal ret = AI_goal(i);
    if ( no_caps( AI_goal_name(ret) ) == name) {
      return ret;
    }
  }
  return AIGOAL_NULL;
}

std::string AI_goal_name(AI_goal goal)
{
  switch (goal) {
    case AIGOAL_NULL:             return "NULL";
    case AIGOAL_ATTACK_ENEMIES:   return "attack_enemies";
    case AIGOAL_ATTACK_NEUTRALS:  return "attack_neutrals";
    case AIGOAL_FLEE:             return "flee";
    case AIGOAL_EAT_CORPSES:      return "eat_corpses";
    case AIGOAL_COLLECT_ITEMS:    return "collect_items";
    case AIGOAL_MAX:              return "BUG - AIGOAL_MAX";
    default:                      return "BUG - Unnamed AI_goal";
  }
  return "BUG - Escaped AI_goal_name switch";
}
