#include "item.h"
#include "item_type.h"
#include "game.h"
#include "globals.h"
#include "cuss.h"
#include "entity.h"
#include "attack.h"
#include <sstream>

Item::Item(Item_type* T)
{
  type = T;
  count = 1;
  ammo = NULL;
  charges = 0;
  hp = 100;   // TODO: Use Item_type durability instead
  if (type) {
    uid = GAME.get_item_uid();
    charges = type->default_charges();
  } else {
    uid = -1;
  }
}

Item::Item(const Item &rhs)
{
  type  = rhs.type;
  count = rhs.count;
  uid   = rhs.uid;
  ammo  = rhs.ammo;
  charges = rhs.charges;
}

Item::~Item()
{
}

Item& Item::operator=(const Item& rhs)
{
  type    = rhs.type;
  ammo    = rhs.ammo;
  count   = rhs.count;
  charges = rhs.charges;

  return *this;
}

Item_class Item::get_item_class()
{
  if (!type) {
    return ITEM_CLASS_MISC;
  }
  return type->get_class();
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
    return -1;
  }
  return type->time_to_reload();
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
    return type->sym;
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
    return type->get_name();
  }
  return "Typeless item";
}

std::string Item::get_name_indefinite()
{
// TODO: Check Item_type for "plural" flag
// TODO: Unique items?
  if (type) {
    std::stringstream ret;
    switch (type->get_class()) {
      case ITEM_CLASS_AMMO:
        ret << "a box of " << type->name << " ammo"; // TODO: Not always box?
        break;
      default:
        ret << "a " << type->name;
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
    ret << "the " << type->name;
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

  switch (get_item_class()) {
    case ITEM_CLASS_AMMO:
    case ITEM_CLASS_LAUNCHER:
      ret << " (" << charges << ")";
      break;
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

int Item::get_weight()
{
  if (!type) {
    return 0;
  }
  if (get_item_class() == ITEM_CLASS_AMMO) {
    return (charges * type->weight) / 100;
  }
  return type->weight * count;
}

int Item::get_volume()
{
  if (!type) {
    return 0;
  }
  if (get_item_class() == ITEM_CLASS_AMMO) {
    Item_type_ammo* ammo = static_cast<Item_type_ammo*>(type);
    return (charges * type->volume) / ammo->count;
  }
  return type->volume;
}

bool Item::has_flag(Item_flag itf)
{
  if (!type) {
    return false;
  }
  return type->has_flag(itf);
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
  int ret = type->attack_speed;
  int min_weight_penalty = stats.str * 3;
  int penalty_per_pound  = 20 - stats.str;
  int wgt = get_weight();
  if (stats.str < 20 && wgt >= min_weight_penalty) {
    wgt -= min_weight_penalty;
// Divide by 10 since the penalty is per pound - 1 unit of weight is 0.1 lbs
    ret += (wgt * penalty_per_pound) / 10;
  }

// TODO: Tweak this section - this is very guess-y.
  int min_volume_penalty = stats.dex * 10;
  int penalty_per_10_volume = 20 - stats.dex;
  int vol = get_volume();
  if (stats.dex < 20 && vol >= min_volume_penalty) {
    vol -= min_volume_penalty;
    ret += (vol * penalty_per_10_volume) / 10;
  }

  return ret;
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
  ret.speed = launcher->fire_ap;
  ret.range = itammo->range;
  ret.variance = launcher->accuracy + itammo->accuracy;
// TODO: Can fired items ever be non-pierce?
  ret.damage       [DAMAGE_PIERCE] = itammo->damage + launcher->damage;
  ret.armor_divisor[DAMAGE_PIERCE] = itammo->armor_pierce;

  return ret;
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
  return true;
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

bool Item::damage(int dam)
{
  hp -= dam;
  if (hp <= 0) {
    return true;
  }
  return false;
}

Item_action Item::show_info()
{
  if (!type) {
    return IACT_NULL;
  }
  Window w_info(0, 0, 80, 24);
  cuss::interface i_info;
  if (!i_info.load_from_file("cuss/i_item_info.cuss")) {
    debugmsg("Couldn't load cuss/i_item_info.cuss!");
    return IACT_NULL;
  }

  i_info.set_data("item_name",  get_name());
  i_info.set_data("num_weight", get_weight());
  i_info.set_data("num_volume", get_volume());
  i_info.set_data("num_bash",   get_damage(DAMAGE_BASH));
  i_info.set_data("num_cut",    get_damage(DAMAGE_CUT));
  i_info.set_data("num_pierce", get_damage(DAMAGE_PIERCE));
  i_info.set_data("num_to_hit", get_to_hit());
// TODO: Use GAME.player's stats
  i_info.set_data("num_speed",  get_base_attack_speed());
  i_info.set_data("description",get_description());
  i_info.draw(&w_info);
  while (true) {
    long ch = input();
    if (ch == 'd' || ch == 'D') {
      return IACT_DROP;
    } else if (ch == 'w') {
      return IACT_WIELD;
    } else if (ch == KEY_ESC || ch == 'q' || ch == 'Q') {
      return IACT_NULL;
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
