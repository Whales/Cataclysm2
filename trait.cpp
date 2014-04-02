#include "trait.h"
#include "stringfunc.h"
#include <string>

Trait_id lookup_trait_id(std::string name)
{
  name = no_caps( trim( name ) );
  for (int i = 0; i < TRAIT_MAX; i++) {
    Trait_id ret = Trait_id(i);
    if (name == no_caps( trait_id_name(ret) ) ) {
      return ret;
    }
  }
  return TRAIT_NULL;
}

std::string trait_name(Trait_id trait)
{
  return trait_id_name(trait);
}

std::string trait_id_name(Trait_id trait)
{
  switch (trait) {

    case TRAIT_NULL:
      return "NULL";

    case TRAIT_FLEET:
      return "Fleet-Footed";

    case TRAIT_QUICK:
      return "Quick";

    case TRAIT_LIGHT_EATER:
      return "Light Eater";

    case TRAIT_CAMEL:
      return "Camel";

    case TRAIT_NIGHT_VISION:
      return "Night Vision";

    case TRAIT_DURABLE:
      return "Durable";

    case TRAIT_STURDY_CONSTITUTION:
      return "Sturdy Constitution";

    case TRAIT_PACKMULE:
      return "Packmule";

    case TRAIT_ROBUST_GENETICS:
      return "Robust Genetics";

    case TRAIT_PAIN_RESISTANT:
      return "Pain Resistant";

    case TRAIT_MAX_GOOD:
      return "BUG - TRAIT_MAX_GOOD";

    case TRAIT_LIGHTWEIGHT:
      return "Lightweight";

    case TRAIT_CHEMICAL_IMBALANCE:
      return "Chemical Imbalance";

    case TRAIT_MAX_NEUTRAL:
      return "BUG - TRAIT_MAX_NEUTRAL";

    case TRAIT_MYOPIC:
      return "Myopic";

    case TRAIT_BAD_BACK:
      return "Bad Back";

    case TRAIT_BAD_HEARING:
      return "Bad Hearing";

    case TRAIT_SMELLY:
      return "Smelly";

    case TRAIT_MAX_BAD:
      return "BUG - TRAIT_MAX_BAD";

    case TRAIT_MAX:
      return "BUG - TRAIT_MAX";

    default:
      return "BUG - Unnamed Trait_id";
  }

  return "BUG - Escaped Trait_id_name() switch!";
}

std::string trait_description(Trait_id trait)
{
  switch (trait) {

    case TRAIT_NULL:
      return "NULL Trait.  If you're reading this, it's a bug!";

    case TRAIT_FLEET:
      return "\
You are especially fast on your feet.  When moving over flat terrain, like \
dirt, grass, or pavement, you only use 95 action points, instead of 100.";

    case TRAIT_QUICK:
      return "\
You are quick at everything you do!  You receive an extra 5 percent action \
points each turn.  Under normal conditions, this means 5 AP; if your speed has \
been reduced to 60, you'll only receive 3 AP, and so on.";

    case TRAIT_LIGHT_EATER:
      return "\
You're able to get by with less food than most people.  Your hunger increases \
once every 8 turns, instead of once every 6.";

    case TRAIT_CAMEL:
      return "\
You're able to get by with less water than most people.  Your thirst increases \
once every 8 turns, instead of once every 6.";

    case TRAIT_NIGHT_VISION:
      return "\
You have a natural ability to see in the dark, more than the average person.  \
Under reduced light, your sight radius is double what it would normally be!";

    case TRAIT_DURABLE:
      return "\
You're particularly durable.  Your maximum HP is 115, instead of the normal \
100.";

    case TRAIT_STURDY_CONSTITUTION:
      return "\
You're very resistance to disease.  You're less likely to get sick, and even \
if you do, diseases, poisons, and other ailments of that nature have a reduced \
effect on you.";

    case TRAIT_PACKMULE:
      return "\
You're good at making do with limited storage space.  Your volume limit is \
increased by 20%.";

    case TRAIT_ROBUST_GENETICS:
      return "\
Your genes are fit in a very particular way.  If you happen to mutate, you're \
as likely to get a good mutation as a bad one (others are much more likely to \
wind up with a bad mutation).";

    case TRAIT_PAIN_RESISTANT:
      return "\
You have a high pain threshold.  The amount of pain needed to reach each level \
(e.g. \"Minor Pain,\" \"Moderate Pain\" etc) is 20 percent higher.";

    case TRAIT_MAX_GOOD:
      return "TRAIT_MAX_GOOD - If you're seeing this, it's a bug!";

    case TRAIT_LIGHTWEIGHT:
      return "\
Drugs of all kind have an increased effect on you.  This can be desirable - it \
means you have to take less to get the same effect - but on the other hand, \
sometimes you don't want things to go to your head so much!";

    case TRAIT_CHEMICAL_IMBALANCE:
      return "\
A slight chemical imbalance results in occasional minor status effects.  These \
are as likely to be good as they are to be bad, but are never too severe.";

    case TRAIT_MAX_NEUTRAL:
      return "TRAIT_MAX_NEUTRAL - If you're seeing this, it's a bug!";

    case TRAIT_MYOPIC:
      return "\
You have poor long-distance vision.  Your sight range is capped at 6 tiles, \
unless you're wearing your glasses.  Taking this trait means you get to start \
with a pair of glasses, but they break easily and a replacement pair may be \
hard to acquire!";

    case TRAIT_BAD_BACK:
      return "\
You have a weak back.  While your strength is just as good for all other \
purposes, your carrying capacity is reduced by 20%.";

    case TRAIT_BAD_HEARING:
      return "\
Your hearing is bad, and you won't hear sounds unless they're twice as loud as \
they'd need to be for a normal person.  This effect is amplified - and \
particularly dangerous - while you're asleep!";

    case TRAIT_SMELLY:
      return "\
You have a strong body odor.  Animals and monsters that track using scent will \
find you easier to detect, and many NPCs will have a slight negative reaction.";

    case TRAIT_MAX_BAD:
      return "TRAIT_MAX_BAD - If you're seeing this, it's a bug!";

    case TRAIT_MAX:
      return "TRAIT_MAX - If you're seeing this, it's a bug!";

    default:
      return "Undefined trait description!  Someone forgot to write this.";
  }

  return "Escaped trait_description() switch?!";
}

int trait_cost(Trait_id trait)
{
  switch (trait) {

    case TRAIT_NULL:
      return 0;

    case TRAIT_FLEET:
      return 3;

    case TRAIT_QUICK:
      return 3;

    case TRAIT_LIGHT_EATER:
      return 2;

    case TRAIT_CAMEL:
      return 2;

    case TRAIT_NIGHT_VISION:
      return 2;

    case TRAIT_DURABLE:
      return 3;

    case TRAIT_STURDY_CONSTITUTION:
      return 2;

    case TRAIT_PACKMULE:
      return 2;

    case TRAIT_ROBUST_GENETICS:
      return 2;

    case TRAIT_PAIN_RESISTANT:
      return 3;

    case TRAIT_MAX_GOOD:
      return 0;

    case TRAIT_LIGHTWEIGHT:
      return 0;

    case TRAIT_CHEMICAL_IMBALANCE:
      return 0;

    case TRAIT_MAX_NEUTRAL:
      return 0;

    case TRAIT_MYOPIC:
      return -2;

    case TRAIT_BAD_BACK:
      return -2;

    case TRAIT_BAD_HEARING:
      return -2;

    case TRAIT_SMELLY:
      return -2;

    case TRAIT_MAX_BAD:
      return 0;

    case TRAIT_MAX:
      return 0;

    default:
      return 0;
  }

  return 0;
}
