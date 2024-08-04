#ifndef MAP_H
#define MAP_H

#define PLATFORM_LIMIT 16

struct map {
  int dirty;
  uint8_t dirt[COLC]; // Elevation from bottom.
  int carrotx,carroty;
  struct platform {
    uint8_t x,y,w;
    uint8_t tileid,xform;
  } platformv[PLATFORM_LIMIT];
  int platformc;
};

int tile_is_dirt(uint8_t tileid);
int tile_is_sky(uint8_t tileid);
int cell_solid(int x,int y);
int cell_solid_below_topsoil(int x,int y); // don't count the topmost row of dirt

int map_add_platform(int x,int y,int w,int tileid);
int map_add_flamethrower(int x,int y,int w);

void map_render();

#endif
