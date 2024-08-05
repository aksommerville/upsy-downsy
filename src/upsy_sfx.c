#include "upsy.h"

#define SFX(contrast,atkt,rlst,wave,trim,notea,notez,dur) \
  lofi_note((contrast<<7)|(atkt<<5)|(rlst<<3)|wave,trim,notea,notez,dur);

void upsy_sfx_reject_grow() {
  SFX(1,0,3,7,0xff,0x30,0x24,200)
}

void upsy_sfx_reject_shrink() {
  SFX(1,0,3,7,0xff,0x30,0x24,200)
}

void upsy_sfx_grow() {
  SFX(1,1,2,3,0xc0,0x30,0x3c,200)
}

void upsy_sfx_shrink() {
  SFX(1,1,2,3,0xc0,0x3c,0x30,200)
}

void upsy_sfx_move_focus() {
  SFX(1,0,1,5,0x60,0x3c,0x3a,100)
}

void upsy_sfx_squash() {
  SFX(0,0,3,6,0xff,0x40,0x20,600)
}

void upsy_sfx_victory() {
  SFX(1,1,2,2,0x40,0x40,0x4c,500)
}

void upsy_sfx_hammer_smash() {
  SFX(1,0,2,1,0x40,0x38,0x28,250)
}

void upsy_sfx_squash_croc() {
  SFX(0,0,1,6,0xff,0x40,0x20,600)
}

static double sfx_tick_recent=0.0;

void upsy_sfx_score_tick() {
  // This will be spammed by the main loop, once per video frame.
  // We must apply our own rate limiter.
  double elapsed=upsy.totalclock-sfx_tick_recent;
  if (elapsed<0.080) return;
  sfx_tick_recent=upsy.totalclock;
  SFX(1,0,0,2,0x20,0x44,0x42,100)
}
