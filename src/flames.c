#include "upsy.h"

#define WAIT_TIME 2.0
#define FRAME_TIME 0.0725
#define STATE_COUNT 20

/* Reset.
 */
 
void flames_clear() {
  upsy.flames.c=0;
}

/* Add.
 */
 
int flames_add(int x,int y,int w) {
  if (upsy.flames.c>=FLAMES_LIMIT) return -1;
  if ((x<0)||(x>=COLC)||(y<0)||(y>=COLC)||!w||(w<-10)||(w>10)) return -1;
  struct flame *flame=upsy.flames.v+upsy.flames.c++;
  flame->x=x;
  flame->y=y;
  flame->w=w;
  flame->state=0;
  flame->clock=WAIT_TIME;
  //TODO Configurable period and phase?
  return 0;
}

/* Update.
 */
 
void flames_update(double elapsed) {
  if (upsy.victoryclock>0.0) return;
  struct flame *flame=upsy.flames.v;
  int i=upsy.flames.c;
  for (;i-->0;flame++) {
    if ((flame->clock-=elapsed)<=0.0) {
      if (++(flame->state)>=STATE_COUNT) flame->state=0;
      if (flame->state) flame->clock+=FRAME_TIME;
      else flame->clock+=WAIT_TIME;
    }
    if (flame->state) {
      if (upsy.rabbit.state!=RABBIT_STATE_DEAD) {
        if ((upsy.rabbit.y>=flame->y)&&(upsy.rabbit.y<flame->y+1)) {
          int dx=(int)upsy.rabbit.x-flame->x;
          if (flame->w<0) {
            if ((dx>=flame->w)&&(dx<0)) {
              rabbit_squash();
            }
          } else {
            if ((dx<=flame->w)&&(dx>0)) {
              rabbit_squash();
            }
          }
        }
      }
      if (upsy.crocodile.present) {
        if ((upsy.crocodile.y>=flame->y)&&(upsy.crocodile.y<=flame->y+1)) {
          int dx=(int)upsy.crocodile.x-flame->x;
          if (flame->w<0) {
            if ((dx>=flame->w)&&(dx<0)) {
              upsy.crocodile.present=0;
              upsy_sfx_squash_croc();
              fireworks_start(upsy.crocodile.x,upsy.crocodile.y);
            }
          } else {
            if ((dx<=flame->w)&&(dx>0)) {
              upsy.crocodile.present=0;
              upsy_sfx_squash_croc();
              fireworks_start(upsy.crocodile.x,upsy.crocodile.y);
            }
          }
        }
      }
    }
  }
}

/* Render.
 */
 
void flames_render() {
  if (upsy.victoryclock>0.0) return;
  struct flame *flame=upsy.flames.v;
  int i=upsy.flames.c;
  for (;i-->0;flame++) {
    if (!flame->state) continue;
    int dstx=SCENEX+flame->x*TILESIZE;
    int dsty=SCENEY+flame->y*TILESIZE;
    int srcy=TILESIZE*((flame->state&1)?6:7);
    int srcx=TILESIZE*6;
    if (flame->w<0) {
      int i=-flame->w; for (;i-->0;) {
        dstx-=TILESIZE;
        if (!i) srcx=TILESIZE*8;
        gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,GFX_XFORM_XREV);
        srcx=TILESIZE*7;
      }
    } else {
      int i=flame->w; for (;i-->0;) {
        dstx+=TILESIZE;
        if (!i) srcx=TILESIZE*8;
        gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,0);
        srcx=TILESIZE*7;
      }
    }
  }
}
