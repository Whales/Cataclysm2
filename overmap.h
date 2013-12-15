#ifndef _OVERMAP_H_
#define _OVERMAP_H_

#include <string>
#include "world_terrain.h"

#define OVERMAP_SIZE 100

class Overmap_tile
{
  World_terrain *type;
};

class Overmap
{
  Overmap_tile tiles[100][100];
};

#endif
