#include "keybind.h"

bool Keybinding_pool::bind_key(long key, Interface_action action)
{
  bindings[key] = action;
  return true;
}

Interface_action Keybinding_pool::bound_to_key(long key)
{
  if (bindings.count(key) == 0) {
    return IACTION_NULL;
  }
  return bindings[key];
}
