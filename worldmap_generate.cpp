#include "worldmap.h"
#include "biome.h"
#include "rng.h"

void Worldmap::generate()
{
  std::vector<Point> points_live;
  Biome_type biomes[WORLDMAP_SIZE][WORLDMAP_SIZE];
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      biomes[x][y] = BIOME_NULL;
    }
  }
// Randomly seed biomes
  for (int i = 1; i < BIOME_MAX; i++) {
    for (int n = 0; n < 8; n++) {
      Point p( rng(0, WORLDMAP_SIZE - 1), rng(0, WORLDMAP_SIZE - 1) );
      points_live.push_back(p);
      biomes[p.x][p.y] = Biome_type(i);
    }
  }

  while (!points_live.empty()) {
    std::vector<Point> new_points;
    //std::vector<Point> points_copy = points_live;
    //for (int i = 0; i < points_live.size(); i++) {
    int i = rng(0, points_live.size() - 1);
      std::vector<Point> valid_growth;
      Point p = points_live[i];
      if (p.x > 0 && biomes[p.x - 1][p.y] == BIOME_NULL) {
        valid_growth.push_back( Point(p.x - 1, p.y) );
      }
      if (p.y > 0 && biomes[p.x][p.y - 1] == BIOME_NULL) {
        valid_growth.push_back( Point(p.x, p.y - 1) );
      }
      if (p.x < WORLDMAP_SIZE - 1 && biomes[p.x + 1][p.y] == BIOME_NULL) {
        valid_growth.push_back( Point(p.x + 1, p.y) );
      }
      if (p.y < WORLDMAP_SIZE - 1 && biomes[p.x][p.y + 1] == BIOME_NULL) {
        valid_growth.push_back( Point(p.x, p.y + 1) );
      }
      if (valid_growth.empty()) { // No valid points - this point is dead!
        points_live.erase(points_live.begin() + i);
        i--;
      } else {
        Point growth = valid_growth[rng(0, valid_growth.size() - 1)];
        biomes[growth.x][growth.y] = biomes[p.x][p.y];
        //new_points.push_back(growth);
        points_live.push_back( growth );
      }
    //}
/*
    for (int i = 0; i < new_points.size(); i++) {
      points_live.push_back( new_points[i] );
    }
*/
  }

// Now look at the biomes and randomly select a terrain for each
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      tiles[x][y].terrain = terrain_from_biome( biomes[x][y] );
    }
  }

/* At this point, we have a lot of blobs of terrain, but no ocean!
 * So: use radials, go around, and decide how far from the edge of the map the
 * ocean will extend.
 */

/*
  int radial_points_needed = WORLDMAP_SIZE;
  int radial_points[WORLDMAP_SIZE];
  int min = WORLDMAP_SIZE / 30;
  int max = WORLDMAP_SIZE / 10;
  radial_points[0] = rng(min, max);
  for (int i = 1; i < WORLDMAP_SIZE; i++) {
    int min_diff =
    radial_points[i] = radial_points[i - 1] + rng(-2, 2);
    if (radial
*/
}
