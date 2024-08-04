#ifndef MAP_H
#define MAP_H

struct map {
  int dirty;
  uint8_t dirt[COLC]; // Elevation from bottom.
};

int tile_is_dirt(uint8_t tileid);
int tile_is_sky(uint8_t tileid);

void map_render();

#endif
