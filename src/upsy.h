#ifndef UPSY_H
#define UPSY_H

#include "pebble/pebble.h"
#include <stdint.h>

#define TILESIZE 6
#define COLC 10
#define ROWC 10

extern struct upsy {
  int texid_tiles;
  int texid_bgbits;
  int texid_title;
  int bgbits_dirty;
  uint8_t map[COLC*ROWC];
  int focuscol;
  double focusclock;
  int focusframe;
  int running;
  int pvinput;
} upsy;

#endif
