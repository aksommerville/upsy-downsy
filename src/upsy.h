#ifndef UPSY_H
#define UPSY_H

#include "pebble/pebble.h"
#include "image/image.h"
#include "gfx/gfx.h"
#include "lofi/lofi.h"
#include "util/rom.h"
#include "stdlib/egg-stdlib.h"
#include <stdint.h>

#define TILESIZE 6
#define COLC 10
#define ROWC 10

#define SCREENW 64
#define SCREENH 64

#define RABBIT_STATE_CHILL 0
#define RABBIT_STATE_WALK 1
#define RABBIT_STATE_FALL 2
#define RABBIT_STATE_DEAD 3

extern struct upsy {
  int pvinput;
  int sceneid;
  
  int texid_tiles;
  int texid_bgbits;
  int texid_title;
  
  int bgbits_dirty;
  uint8_t map[COLC*ROWC];
  
  /* Being a quick-n-dirty jam game, I'm not going to generalize sprites.
   * The scene may have zero or one of: Carrot, Hammer, Crocodile.
   * Always one Rabbit.
   * Multiple Flamethrowers.
   */
  //XXX I'm modelling it badly. Rewrite it, keep it clean.
  struct focus {
    int x; // tiles
    double animclock;
    int animframe;
  } focus;
  struct rabbit {
    double x,y; // tiles
    double dx; // tile/s
    double animclock;
    int animframe;
    int state;
  } rabbit;
  struct hammer {
    int x,w; // tiles; (w) zero for no hammer
    double h; // Vertical extent in tiles, >=1
    double dh; // tile/s, zero only at rest, at the top
    double clock; // Counts down while at rest.
  } hammer;
  struct crocodile {
    int inuse;
    double x,y; // tiles
    double dx; // tile/s
    double animclock;
    int animframe;
  } crocodile;
  struct carrot {
    int x,y; // tiles
  } carrot;
  
  int16_t audio[1024];
  int songid;
} upsy;

int prepare_scene(int sceneid);

void update_scene(double elapsed);
void move_focus(int d);
void change_world(int d);
void render_scene();

int tile_is_dirt(uint8_t tileid);
int tile_is_sky(uint8_t tileid);

void upsy_play_song(int songid);

#endif
