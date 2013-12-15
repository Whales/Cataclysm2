#ifndef _OVERMAP_DATA_H_
#define _OVERMAP_DATA_H_

#include "overmap.h"

static Overmap_tile_type omtiles[] =
{

{
  OMT_KILL, "bug tile",
  { 'x', c_white, c_black }
},

{
  OMT_FIELD,  "field",
  { '.', c_green, c_black }
},

{
  OMT_FOREST, "forest",
  { 'F', c_green, c_black }
},

{
  OMT_ROAD, "road",
  { 'x', c_dkgray, c_black }
},

{
  OMT_HOUSE, "house",
  { 'o', c_green, c_black }
},

};
#endif
