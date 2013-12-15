#ifndef _MAPDATA_H_
#define _MAPDATA_H_

#include "map.h"
#include "enum.h"

static Tile_type TILES[] =
{

{ TILE_NULL,  "Bug tile",
  { 'x', c_white, c_red },
  100, TF_NOFLAGS
},

{ TILE_DIRT,  "dirt",
  { '.', c_brown, c_black },
  100, TF_NOFLAGS
},

{ TILE_GRASS, "grass",
  { '.', c_green, c_black },
  100, TF_NOFLAGS
},

{ TILE_PAVEMENT, "pavement",
  { '.', c_dkgray, c_black },
  100, TF_NOFLAGS
},

{ TILE_TREE,  "tree",
  { '7', c_green, c_black },
  0,  TF_NOFLAGS
},

{ TILE_WALL,  "wall",
  { '#', c_ltgray, c_black },
  0,  TF_NOFLAGS
}

};

#endif
