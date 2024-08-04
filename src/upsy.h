#ifndef UPSY_H
#define UPSY_H

#define TILESIZE 6
#define COLC 10
#define ROWC 10

#define SCREENW 64
#define SCREENH 64

/* The proper scene is 60x60. Centered horizontally, at the bottom of our 64x64 framebuffer.
 * Those left and right margins go to waste.
 * Top margin, we can draw status on it.
 */
#define SCENEX 2
#define SCENEY 4

#define UPSY_VICTORY_TIME 3.0

// We can have pbltool generate a header with macros for the custom types and resource names, but meh.
#define PBL_TID_scene 16

#include "pebble/pebble.h"
#include "image/image.h"
#include "gfx/gfx.h"
#include "lofi/lofi.h"
#include "util/rom.h"
#include "stdlib/egg-stdlib.h"
#include "map.h"
#include "focus.h"
#include "rabbit.h"
#include "hammer.h"
#include <stdint.h>

extern struct upsy {
  int pvinput;
  int sceneid;
  
  int texid_tiles;
  int texid_bgbits;
  int texid_title;
  
  int16_t audio[1024];
  int songid;
  
  double victoryclock;
  struct map map;
  struct focus focus;
  struct rabbit rabbit;
  struct hammer hammer;
} upsy;

int prepare_scene(int sceneid);

void update_scene(double elapsed);
void render_scene();

void upsy_play_song(int songid);

//TODO Sound effects.
static void upsy_sfx_reject_grow() {}
static void upsy_sfx_reject_shrink() {}
static void upsy_sfx_grow() {}
static void upsy_sfx_shrink() {}
static void upsy_sfx_squash() {}
static void upsy_sfx_victory() {}
static void upsy_sfx_hammer_smash() {}

#endif
