#ifndef _HELP_H_
#define _HELP_H_

#include <string>
#include "skill.h"

void help_screen(std::string file = "", std::string term = "");

void help_skill_desc(Skill_type skill);

#endif
