#ifndef _WORLDMAP_H_
#define _WORLDMAP_H_

#include <string>
#include "world_terrain.h"

#define WORLDMAP_SIZE 100

class Worldmap_tile
{
  World_terrain *type;
};

class Worldmap
{
  Worldmap_tile tiles[100][100];
};

#endif
