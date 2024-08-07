#ifndef UPSY_H
#define UPSY_H

#define TILESIZE 6
#define COLC 10
#define ROWC 10

#define SCREENW 60
#define SCREENH 64

/* The proper scene is 60x60. Centered horizontally, at the bottom of our 64x64 framebuffer.
 * Those left and right margins go to waste.
 * Top margin, we can draw status on it.
 */
#define SCENEX 0
#define SCENEY 4

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
#include "crocodile.h"
#include "fireworks.h"
#include "hawk.h"
#include "flames.h"
#include <stdint.h>

/* Nonzero to start at the last scene and advance backward to 1.
 * Don't do that in prod! But it's very helpful while adding new scenes.
 * Also causes us to skip the hello splash.
 */
#define BACKWARD_SCENES 0

extern struct upsy {
  int pvinput;
  int sceneid;
  double totalclock;
  
  int texid_tiles;
  int texid_bgbits;
  int texid_title;
  
  int16_t audio[1024];
  int songid;
  
  // Stage stats.
  int mortc;
  double stagetime;
  int clear_bonus;
  int death_bonus;
  int time_bonus;
  
  // Session stats.
  int mortc_total;
  int score;
  double totaltime;
  
  int hiscore;
  int scenec;
  
  double victoryclock;
  struct map map;
  struct focus focus;
  struct rabbit rabbit;
  struct hammer hammer;
  struct crocodile crocodile;
  struct fireworks fireworks;
  struct hawk hawk;
  struct flames flames;
} upsy;

int prepare_scene(int sceneid);

void update_scene(double elapsed);
void render_scene();

void upsy_play_song(int songid);
void upsy_render_text(int dstx,int dsty,const char *src,int srcc);

void upsy_save_hiscore();
void upsy_load_hiscore();
void upsy_save_hiscore_if();

void upsy_sfx_reject_grow();
void upsy_sfx_reject_shrink();
void upsy_sfx_grow();
void upsy_sfx_shrink();
void upsy_sfx_move_focus();
void upsy_sfx_squash();
void upsy_sfx_victory();
void upsy_sfx_hammer_smash();
void upsy_sfx_squash_croc();
void upsy_sfx_score_tick();

#endif
