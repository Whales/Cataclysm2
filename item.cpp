#include "item.h"
#include "item_type.h"
#include "game.h"
#include "globals.h"
#include "cuss.h"
#include "entity.h"
#include "attack.h"
#include "files.h"    // For CUSS_DIR
#include "rng.h"
#include <sstream>

Item::Item(Item_type* T)
{
  type = T;
  count = 1;
  ammo = NULL;
  charges = 0;
  subcharges = 0;
  fire_mode = 0;
  hp = 100;   // TODO: Use Item_type durability instead
  corpse = NULL;
  active = ITEM_ACTIVE_OFF;
  if (type) {
    uid = GAME.get_item_uid();
    charges = type->default_charges();
    if (charges > 0 && get_item_class() == ITEM_CLASS_TOOL) {
// Tools have subcharges, so set that up
      Item_type_tool* tool = static_cast<Item_type_tool*>(type);
      subcharges = tool->subcharges;
    }
    if (type->volume < 100) {
      hp = type->volume;
    }
    if (hp < 5) {
      hp = 5;
    }
  } else {
    uid = -1;
  }
}

Item::Item(const Item &rhs)
{
  type        = rhs.type;
  count       = rhs.count;
  uid         = rhs.uid;
  ammo        = rhs.ammo;
  charges     = rhs.charges;
  subcharges  = rhs.subcharges;
  fire_mode   = rhs.fire_mode;
  active      = rhs.active;
  corpse      = rhs.corpse;
// Note lack of UID copy - is this correct?

  if (!rhs.contents.empty()) {
    contents.clear();
    for (int i = 0; i < rhs.contents.size(); i++) {
      contents.push_back( rhs.contents[i] );
    }
  }
}

Item::~Item()
{
}

Item& Item::operator=(const Item& rhs)
{
  type        = rhs.type;
  ammo        = rhs.ammo;
  count       = rhs.count;
  charges     = rhs.charges;
  subcharges  = rhs.subcharges;
  fire_mode   = rhs.fire_mode;
  uid         = rhs.uid;    // Will this cause bugs?
  corpse      = rhs.corpse;

  contents.clear();
  if (!rhs.contents.empty()) {
    for (int i = 0; i < rhs.contents.size(); i++) {
      contents.push_back( rhs.contents[i] );
    }
  }

  return *this;
}

Item_class Item::get_item_class()
{
  if (!type) {
    return ITEM_CLASS_MISC;
  }
  return type->get_class();
}

Item_action Item::default_action()
{
  if (!type) {
    return IACT_NULL;
  }
  return type->default_action();
}

bool Item::is_real()
{
  return type;
}

bool Item::can_reload()
{
// TODO: Reloadable tools
  return get_item_class() == ITEM_CLASS_LAUNCHER;
}

int Item::time_to_reload()
{
  if (!can_reload() || !is_real()) {
    return 0;
  }
  return type->time_to_reload();
}

int Item::time_to_fire()
{
  if (!is_real()) {
    return 0;
  }
  return type->time_to_fire();
}

int Item::get_uid()
{
  if (!is_real()) {
    return -1;
  }
  return uid;
}

glyph Item::top_glyph()
{
  if (type) {
    glyph ret = type->sym;
    if (get_item_class() == ITEM_CLASS_CORPSE && corpse) {
      ret.fg = corpse->sym.fg;
    }
    return ret;
  }
  return glyph();
}

std::string Item::get_data_name()
{
  if (type) {
    return type->get_data_name();
  }
  return "Typeless item";
}

std::string Item::get_name()
{
  if (type) {
    if (get_item_class() == ITEM_CLASS_CORPSE && corpse) {
      return corpse->get_name() + " " + type->get_name();
    }
    return type->get_name();
  }
  return "Typeless item";
}

std::string Item::get_name_indefinite()
{
// TODO: Unique items?
  std::string article = (has_flag(ITEM_FLAG_PLURAL) ? "some" : "a");
  if (type) {
    std::stringstream ret;
    switch (get_item_class()) {
      case ITEM_CLASS_AMMO:
        ret << "a box of " << get_name(); // TODO: Not always box?
        break;
      default:
        ret << article << " " << get_name();
    }
// Display FULL info on contained items
    if (!contents.empty()) {
      std::string preposition = "containing";
      bool use_article = true;
      if (get_item_class() == ITEM_CLASS_CONTAINER) {
        Item_type_container* cont = static_cast<Item_type_container*>(type);
        preposition = cont->preposition;
        use_article = cont->use_article;
      }
      ret << " " << preposition << " ";
      if (use_article) {
        ret << contents[0].get_name_indefinite();
      } else {
        ret << contents[0].get_name_full();
      }
    }
    return ret.str();
  }
  return "a typeless item";
}

std::string Item::get_name_definite()
{
// TODO: Check Item_type for "plural" flag
// TODO: Unique items?
  if (type) {
    std::stringstream ret;
    ret << "the " << get_name();
    return ret.str();
  }
  return "the typeless item";
}

std::string Item::get_name_full()
{
  if (!type) {
    return "typeless item (0)";
  }
  std::stringstream ret;
  ret << get_name();
// Display FULL info on contained items
  if (!contents.empty()) {
    std::string preposition = "containing";
    bool use_article = true;
    if (get_item_class() == ITEM_CLASS_CONTAINER) {
      Item_type_container* cont = static_cast<Item_type_container*>(type);
      preposition = cont->preposition;
      use_article = cont->use_article;
    }
    ret << " " << preposition << " ";
    if (use_article) {
      ret << contents[0].get_name_indefinite();
    } else {
      ret << contents[0].get_name_full();
    }
  }

// Display the number of charges for items that use them
  if (type->uses_charges() || active == ITEM_ACTIVE_TIMER) {
    ret << " (" << charges << ")";
  }
    
  if (is_active()) {
    ret << " <c=yellow>[on]<c=/>";
  }

  return ret.str();
}

std::string Item::get_description()
{
  if (type) {
    return type->description;
  }
  return "Undefined item - type is NULL (this is a bug)";
}

std::string Item::get_description_full()
{
  if (!type) {
    return "Undefined item - type is NULL (this is a bug)";
  }
  std::stringstream ret;
  ret << type->description << std::endl << std::endl <<
         type->get_property_description();

// If we're a corpse, then we need a special description based on the monster
  if (get_item_class() == ITEM_CLASS_CORPSE && corpse) {
    ret << "Type: " << corpse->get_name() << std::endl;
// TODO: Put in a list of relevant monster flags, like "poison to eat"
  }

// If we are a container, include info on our contents!
  if (get_item_class() == ITEM_CLASS_CONTAINER && !contents.empty()) {
    ret << std::endl << "<c=yellow>Contents:<c=/>" << std::endl <<
           contents[0].get_description_full();
  }

  return ret.str();
}

std::vector<Item_action> Item::get_applicable_actions()
{
  std::vector<Item_action> ret;
// We can always(?) drop and wield items.
  ret.push_back(IACT_WIELD);
  ret.push_back(IACT_DROP);
  switch (get_item_class()) {
    case ITEM_CLASS_CLOTHING:
      ret.push_back(IACT_WEAR);
      break;
    case ITEM_CLASS_AMMO:
      break;  // Can't do anything with ammo by itself!
    case ITEM_CLASS_LAUNCHER:
      ret.push_back(IACT_RELOAD);
      ret.push_back(IACT_UNLOAD);
      break;
    case ITEM_CLASS_FOOD:
      ret.push_back(IACT_EAT);
      break;
    case ITEM_CLASS_TOOL:
      ret.push_back(IACT_APPLY);
      break;
    case ITEM_CLASS_CONTAINER:
      ret.push_back(IACT_UNLOAD);
      break;
    case ITEM_CLASS_CORPSE:
      ret.push_back(IACT_BUTCHER);
      break;
  }
// Now, check the contents.
  for (int i = 0; i < contents.size(); i++) {
    std::vector<Item_action> cont_acts = contents[i].get_applicable_actions();
// Gotta make sure we don't have duplicates...
    for (int n = 0; n < cont_acts.size(); n++) {
      bool found = false;
      for (int m = 0; !found && m < ret.size(); m++) {
        if (cont_acts[n] == ret[m]) {
          found = true;
        }
      }
      if (!found) {
        ret.push_back( cont_acts[n] );
      }
    }
  }

  return ret;
}

int Item::get_weight()
{
  if (!type) {
    return 0;
  }

  if (get_item_class() == ITEM_CLASS_AMMO) {
    return (charges * type->weight) / 100;
  }

  if (has_flag(ITEM_FLAG_LIQUID) ||
      (get_item_class() == ITEM_CLASS_FOOD && !has_flag(ITEM_FLAG_CONSTANT))) {
    return (charges * type->weight);
  }

  if (get_item_class() == ITEM_CLASS_CORPSE) {
    if (corpse) {
      return corpse->get_weight();
    }
    return 0;
  }

  int ret = type->weight;
  for (int i = 0; i < contents.size(); i++) {
    ret += contents[i].get_weight();
  }

  return ret * count;
}

int Item::get_volume()
{
  if (!type) {
    return 0;
  }
  if (get_item_class() == ITEM_CLASS_AMMO) {
    return (charges * type->volume) / 100;
  }

  if (has_flag(ITEM_FLAG_LIQUID) ||
      (get_item_class() == ITEM_CLASS_FOOD && !has_flag(ITEM_FLAG_CONSTANT))) {
    return (charges * type->volume);
  }

/* Some containers, like a plastic bag, can hold more than their own volume
 * (since when they're empty you can wad them up).  If that's the case, return
 * the total volume of its contents instead!
 */
  if (get_item_class() == ITEM_CLASS_CONTAINER) {
    int own_volume = type->volume;
    int contents_volume = get_volume_capacity_used();
    if (own_volume > contents_volume) {
      return own_volume;
    }
    return contents_volume;
  }

  if (get_item_class() == ITEM_CLASS_CORPSE) {
    if (corpse) {
      return corpse->get_volume();
    }
    return 0;
  }

// Everything else just returns its type's volume.
  return type->volume;
}

int Item::get_volume_capacity()
{
  if (!is_real()) {
    return 0;
  }

  if (get_item_class() == ITEM_CLASS_CONTAINER) {
    Item_type_container* container = static_cast<Item_type_container*>(type);
    return container->capacity;
  }

/* Even though "volume capacity" means something totally different for
 * containers and clothing, put clothing in this function too.  It makes sense,
 * and there shouldn't be any overlap.
 */
  if (get_item_class() == ITEM_CLASS_CLOTHING) {
    Item_type_clothing* clothing = static_cast<Item_type_clothing*>(type);
    return clothing->carry_capacity;
  }

// We're neither a container nor clothing, return 0.
  return 0;
}

int Item::get_volume_capacity_used()
{
  int ret = 0;
  for (int i = 0; i < contents.size(); i++) {
    ret += contents[i].get_volume();
  }
  return ret;
}

bool Item::has_flag(Item_flag itf)
{
  if (!type) {
    return false;
  }
  return type->has_flag(itf);
}

bool Item::covers(Body_part part)
{
  if (!is_real() || get_item_class() != ITEM_CLASS_CLOTHING) {
    return false;
  }
  Item_type_clothing* clothing = static_cast<Item_type_clothing*>(type);
  return clothing->covers[part];
}

int Item::get_damage(Damage_type dtype)
{
  if (type) {
    return type->damage[dtype];
  }
  return 0;
}

int Item::get_to_hit()
{
  if (type) {
    return type->to_hit;
  }
  return 0;
}

int Item::get_base_attack_speed()
{
  Stats stats;
  return get_base_attack_speed(stats);
}

int Item::get_base_attack_speed(Stats stats)
{
  if (!type) {
    return 0;
  }
// TODO: Do we really need Item_type::attack_speed?
  int ret = 50 + type->attack_speed;

  int min_weight_penalty = stats.strength;
  int penalty_per_pound  = (2 * (20 - stats.strength) ) / 3;
  int wgt = get_weight();
  if (stats.strength < 20 && wgt >= min_weight_penalty) {
    wgt -= min_weight_penalty;
// Divide by 10 since the penalty is per pound - 1 unit of weight is 0.1 lbs
    ret += (wgt * penalty_per_pound) / 10;
  }

// TODO: Tweak this section - this is very guess-y.
  int min_volume_penalty = stats.dexterity * 3;
  int penalty_per_10_volume = (2 * (20 - stats.dexterity) ) / 3;
  int vol = get_volume();
  if (stats.dexterity < 20 && vol >= min_volume_penalty) {
    vol -= min_volume_penalty;
    ret += (vol * penalty_per_10_volume) / 10;
  }

  return ret;
}

int Item::get_shots_fired()
{
  if (get_item_class() != ITEM_CLASS_LAUNCHER) {
    return 0;
  }
  Item_type_launcher* launcher = static_cast<Item_type_launcher*>(type);
// Sanity check
  if (fire_mode < 0 || fire_mode >= launcher->modes.size()) {
    fire_mode = 0;
  }
  if (launcher->modes.empty()) {
    return 1;
  }
  return launcher->modes[fire_mode];
}
  

int Item::get_max_charges()
{
  if (get_item_class() == ITEM_CLASS_LAUNCHER) {
    Item_type_launcher* launcher = static_cast<Item_type_launcher*>(type);
    return launcher->capacity;
  }
  return 0;
}

bool Item::combines()
{
  if (!is_real()) {
    return false;
  }
  return type->always_combines();
}

bool Item::combine_by_charges()
{
  if (!is_real()) {
    return false;
  }
  return type->combine_by_charges();
}

// TODO: Use stats & skills.
Ranged_attack Item::get_thrown_attack()
{
  Ranged_attack ret;
  if (!type) {
    return ret;
  }
// If the ranged speed is 0, then set it based on our weight
  if (type->thrown_speed == 0) {
    ret.speed = 50 + 5 * type->weight;
    ret.speed += type->volume / 10;
  } else {
    ret.speed = type->thrown_speed;
  }
// Copy variance from type
  ret.variance = type->thrown_variance;
// Add variance for heavy items
  Dice extra_variance;
  extra_variance.number = 1 + type->weight / 20;
  extra_variance.sides  = type->volume / 5;
  ret.variance += extra_variance;
// Copy damage; note that thrown_dmg_percent defaults to 50
  for (int i = 0; i < DAMAGE_MAX; i++) {
    ret.damage[i] = (type->damage[i] * type->thrown_dmg_percent) / 100;
  }
  return ret;
}

// TODO: Use stats & skills.
Ranged_attack Item::get_fired_attack()
{
  if (!type || get_item_class() != ITEM_CLASS_LAUNCHER || charges <= 0 ||
      !ammo) {
    return Ranged_attack();
  }

  Item_type_launcher* launcher = static_cast<Item_type_launcher*>(type);
  Item_type_ammo*     itammo   = static_cast<Item_type_ammo*>    (ammo);

  Ranged_attack ret;
  ret.speed    = launcher->fire_ap;
  ret.range    = itammo->range;
  ret.variance = launcher->accuracy + itammo->accuracy;
  ret.pellets  = itammo->pellets;
// TODO: Can fired items ever be non-pierce?
  ret.damage       [DAMAGE_PIERCE] = itammo->damage + launcher->damage;
  ret.armor_divisor[DAMAGE_PIERCE] = itammo->armor_pierce;

  return ret;
}

bool Item::combine_with(const Item& rhs)
{
// TODO: Handle combining items of different damage levels etc.
  if (!combines()) {
    return false;
  }
  if (type != rhs.type) {
    return false;
  }
  if (combine_by_charges()) {
    charges += rhs.charges;
  } else {
    count += rhs.count;
  }
  return true;
}

void Item::prep_for_generation()
{
  place_in_its_container();
}

bool Item::place_in_its_container()
{
  if (type->container.empty()) {
    return false;
  }
  Item_type* container_type = ITEM_TYPES.lookup_name( type->container );
  if (container_type) {
    Item tmp(container_type);
    if (tmp.get_item_class() != ITEM_CLASS_CONTAINER) {
      debugmsg("%s has non-container container (%s; %s)",
               get_data_name().c_str(), type->container.c_str(),
               item_class_name(tmp.get_item_class()).c_str());
      return false;
    }
    if (!tmp.add_contents(*this)) {
      debugmsg("%s could not be placed in its container (%s)",
               get_data_name().c_str(), type->container.c_str());
      return false;
    }
    *this = tmp;
    return true;
  }
  debugmsg("%s has non-existant container '%s'", get_data_name().c_str(),
           type->container.c_str());
  return false;
}

bool Item::add_contents(Item it)
{
// TODO: Require water-tight containers for liquids?
  if (!is_real() || get_item_class() != ITEM_CLASS_CONTAINER ||
      !it.is_real()) {
    return false;
  }
  int capacity = get_volume_capacity();
  if (has_flag(ITEM_FLAG_OPEN_END)) {
    capacity *= 5;
  }
  if (get_volume_capacity_used() + it.get_volume() > capacity) {
    return false;
  }
  contents.push_back(it);
  return true;
}

bool Item::advance_fire_mode()
{
  if (get_item_class() != ITEM_CLASS_LAUNCHER) {
    return false;
  }
  Item_type_launcher* launcher = static_cast<Item_type_launcher*>(type);
  if (launcher->modes.size() <= 1) {
    return false; // It only has one mode, so we can't change it!
  }
  fire_mode++;
  if (fire_mode >= launcher->modes.size()) {
    fire_mode = 0;
  }
  return true;
}

bool Item::reload(Entity* owner, int ammo_uid)
{
  if (!owner) {
    return false;
  }
  Item* ammo_used = owner->ref_item_uid(ammo_uid);
  if (!ammo_used) {
    return false;
  }
  int charges_available = get_max_charges() - charges;
  if (charges_available <= 0) {
    return false;
  }
  if (ammo_used->charges > charges_available) {
    ammo_used->charges -= charges_available;
    charges = get_max_charges();
  } else {
    charges += ammo_used->charges;
    owner->remove_item_uid(ammo_uid);
  }
  ammo = ammo_used->type;
  if (get_item_class() == ITEM_CLASS_TOOL) {  // Need to set up subcharges too
    Item_type_tool* tool = static_cast<Item_type_tool*>(type);
    subcharges = tool->subcharges;
  }
  return true;
}

bool Item::damage(int dam)
{
  if (dam < 0) {
    return false;
  }
  hp -= dam;
  if (hp <= 0) {
    return true;
  }
  return false;
}

bool Item::absorb_damage(Damage_type damtype, int dam)
{
  if (!is_real() || get_item_class() == ITEM_CLASS_CLOTHING) {
    return false; // Don't do anything
  }
  Item_type_clothing* clothing = static_cast<Item_type_clothing*>(type);
/* TODO:  Item_type_clothing should, eventually, use a Damage_set for its armor
 *        ratings.  Once it does, the following can be simplified.
 */
  int armor = 0;
  switch (damtype) {
    case DAMAGE_BASH:
      armor = clothing->armor_bash;
      break;
    case DAMAGE_CUT:
      armor = clothing->armor_cut;
      break;
    case DAMAGE_PIERCE:
      armor = clothing->armor_pierce;
      break;
  }

  armor = rng(0, armor);  // Absorb some of the damage!
  int damage_to_item = armor;
  if (dam < armor) {
    damage_to_item = dam;
  }
  damage_to_item = dice(2, damage_to_item + 1) - 2;
// Reduce the damage by the amount of the armor used
  dam -= armor;
  return damage(damage_to_item);
}

bool Item::power_on()
{
// TODO: Can we power on other classes?
  if (!is_real() || get_item_class() != ITEM_CLASS_TOOL) {
    return false;
  }
  if (charges == 0 && subcharges == 0) {
    return false;
  }
  if (is_active()) {
    return false;
  }
  active = ITEM_ACTIVE_POWERED;
  GAME.add_active_item(this);
  return true;
}

bool Item::power_off()
{
  if (!is_active()) {
    return false;
  }
  active = ITEM_ACTIVE_OFF;
  GAME.remove_active_item(this);
  return true;
}

bool Item::start_countdown()
{
  if (!is_real() || get_item_class() != ITEM_CLASS_TOOL) {
    return false;
  }
  if (is_active()) {
    return false;
  }
  Item_type_tool* tool = static_cast<Item_type_tool*>(type);
  if (!tool->countdown_action.real) {
    return false;
  }
  charges = tool->countdown_timer;
  active = ITEM_ACTIVE_TIMER;
  GAME.add_active_item(this);
  return true;
}

bool Item::finish_countdown()
{
  if (!is_real()) {
    return false;
  }
  if (active != ITEM_ACTIVE_TIMER || get_item_class() != ITEM_CLASS_TOOL) {
    return false;
  }
  Item_type_tool* tool = static_cast<Item_type_tool*>(type);
  tool->countdown_action.activate(this);
  GAME.remove_active_item(this);
  active = ITEM_ACTIVE_OFF;
  if (tool->countdown_action.destroy_if_chargeless) {
    if (!GAME.destroy_item_uid(get_uid())) {
      debugmsg("Couldn't destroy item!");
    }
  }
  return true;
}
  

bool Item::is_active()
{
  if (!is_real()) {
    return false;
  }
  return (active == ITEM_ACTIVE_POWERED || active == ITEM_ACTIVE_TIMER);
}

bool Item::process_active()
{
// TODO: Can we power on other classes?
  if (get_item_class() != ITEM_CLASS_TOOL) {
    return false;
  }
  if (active == ITEM_ACTIVE_OFF) {
    debugmsg("Item::process_active() run on a non-active item (%s)!",
             get_name_full().c_str());
    return false;
  }

  Item_type_tool* tool = static_cast<Item_type_tool*>(type);

  if (active == ITEM_ACTIVE_POWERED) {
    subcharges--;
    if (subcharges <= 0) {
      charges--;
      if (charges >= 0) {
        subcharges += tool->subcharges;
      } else {
// We're truly out of power!
        charges = 0;
        subcharges = 0;
// If we're on a timer, activate the timer function!
        power_off();  // This removes us from Game::active_items
// We have to destroy the item AFTER removing it from Game:active_items!
        if (tool->powered_action.destroy_if_chargeless) {
          GAME.destroy_item(this);
          return true;
        }
        return false;
      }
    }
// Finally, do what we're meant to do while active.
    if (tool->powered_action.real) {
      tool->powered_action.activate(this);
    } else {
      debugmsg("%s is ITEM_ACTIVE_POWERED but its powered_action isn't real!",
               get_name_full().c_str());
    }
    return false;

  } else if (active == ITEM_ACTIVE_TIMER) {
    if (!tool->countdown_action.real) {
      debugmsg("%s is ITEM_ACTIVE_TIMER but its countdown_action isn't real!",
               get_name_full().c_str());
      return false;
    }
      
    charges--;
    if (charges <= 0) { // We hit the countdown!
      finish_countdown();
    }
    return true;
  }

  return false;
}

Item_action Item::show_info(Entity* user)
{
  if (!type) {
    return IACT_NULL;
  }
  Window w_info(0, 0, 80, 24);
  cuss::interface i_info;
  if (!i_info.load_from_file(CUSS_DIR + "/i_item_info.cuss")) {
    return IACT_NULL;
  }

  i_info.set_data("item_name",  get_name_full());
  i_info.set_data("num_weight", get_weight());
  i_info.set_data("num_volume", get_volume());
  i_info.set_data("num_bash",   get_damage(DAMAGE_BASH));
  i_info.set_data("num_cut",    get_damage(DAMAGE_CUT));
  i_info.set_data("num_pierce", get_damage(DAMAGE_PIERCE));
  i_info.set_data("num_to_hit", get_to_hit());
  if (user) {
    i_info.set_data("num_speed",  get_base_attack_speed(user->stats));
  } else {
    i_info.set_data("num_speed",  get_base_attack_speed());
  }

  std::stringstream actions;
  std::vector<Item_action> app_actions = get_applicable_actions();
  for (int i = 0; i < app_actions.size(); i++) {
    switch (app_actions[i]) {
      case IACT_WIELD:
        actions << "<c=magenta>w<c=/>ield" << std::endl;
        break;
      case IACT_WEAR:
        actions << "<c=magenta>W<c=/>ear" << std::endl;
        break;
      case IACT_DROP:
        actions << "<c=magenta>d<c=/>rop" << std::endl;
        break;
      case IACT_EAT:
        actions << "<c=magenta>e<c=/>at" << std::endl;
        break;
      case IACT_APPLY:
        actions << "<c=magenta>a<c=/>pply" << std::endl;
        break;
      case IACT_UNLOAD:
        actions << "<c=magenta>U<c=/>nload" << std::endl;
        break;
      case IACT_RELOAD:
        actions << "<c=magenta>R<c=/>eload" << std::endl;
        break;
      case IACT_BUTCHER:
        actions << "<c=magenta>B<c=/>utcher" << std::endl;
        break;
    }
  }
  actions << std::endl << "<c=magenta>Esc<c=/> or <c=magenta>q<c=/>: " <<
             "Cancel / Do nothing";
  i_info.set_data("text_actions", actions.str());
// get_desciption_full() includes type-specific info, e.g. nutrition for food
  i_info.set_data("description", get_description_full());
  
  i_info.draw(&w_info);
  while (true) {
    long ch = input();
    Item_action ret = IACT_NULL;
    switch (ch) {
      case 'd':
      case 'D': ret = IACT_DROP;    break;
      case 'w': ret = IACT_WIELD;   break;
      case 'W': ret = IACT_WEAR;    break;
      case 'e': ret = IACT_EAT;     break;
      case 'a': ret = IACT_APPLY;   break;
      case 'U': ret = IACT_UNLOAD;  break;
      case 'R': ret = IACT_RELOAD;  break;
      case 'B': ret = IACT_BUTCHER; break;

      case KEY_ESC:
      case 'q':
      case 'Q':
        return IACT_NULL;
    }

    if (ret != IACT_NULL) { // We chose an action.
// Check the list of applicable actions to see if the action the player chose is
// actually available.
      for (int i = 0; i < app_actions.size(); i++) {
        if (app_actions[i] == ret) {
          return ret;
        }
      }
    }
  }
  return IACT_NULL;
}

std::string list_items(std::vector<Item> *items)
{
  if (items->empty()) {
    return "nothing.";
  }
  std::stringstream item_text;
  for (int i = 0; i < items->size(); i++) {
    item_text << (*items)[i].get_name_indefinite();
    if (i == items->size() - 2) {
      item_text << " and ";
    } else if (items->size() >= 3 && i < items->size() - 2) {
      item_text << ", ";
    }
  }
  item_text << ".";
  return item_text.str();
}
