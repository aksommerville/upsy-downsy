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
 
int flames_add(int x,int y,int w,int h,int period,int phase) {
  if (upsy.flames.c>=FLAMES_LIMIT) return -1;
  struct flame *flame=upsy.flames.v+upsy.flames.c++;
  flame->x=x;
  flame->y=y;
  flame->w=w;
  flame->h=h;
  if (period>0) {
    flame->period=period/1000.0+WAIT_TIME;
    flame->clock=phase/1000.0+WAIT_TIME;
    flame->offtime=WAIT_TIME;
  } else {
    flame->period=1.0;
    flame->clock=0.0;
    flame->offtime=0.0;
  }
  return 0;
}

/* Killbox.
 */
 
static void flames_calculate_killbox(double *x,double *y,double *w,double *h,const struct flame *flame) {
  *x=flame->x;
  *y=flame->y;
  *w=*h=1.0;
  if (flame->w<0) { (*x)+=flame->w; (*w)-=flame->w; }
  else if (flame->w>0) (*w)+=flame->w-1.0;
  if (flame->h<0) { (*y)+=flame->h; (*h)-=flame->h; }
  else if (flame->h>0) (*h)+=flame->h-1.0;
  (*x)+=0.125;
  (*y)+=0.125;
  (*w)-=0.250;
  (*h)-=0.250;
}

static inline int point_in_killbox(double x,double y,double w,double h,double qx,double qy) {
  if (qx<x) return 0;
  if (qy<y) return 0;
  if (qx>=x+w) return 0;
  if (qy>=y+h) return 0;
  return 1;
}

/* Update.
 */
 
void flames_update(double elapsed) {
  if (upsy.victoryclock>0.0) return;
  struct flame *flame=upsy.flames.v;
  int i=upsy.flames.c;
  for (;i-->0;flame++) {
    if ((flame->clock-=elapsed)<=0.0) {
      flame->clock+=flame->period;
    }
    if (flame->clock>=flame->offtime) {
      double x,y,w,h;
      flames_calculate_killbox(&x,&y,&w,&h,flame);
      if (upsy.rabbit.state!=RABBIT_STATE_DEAD) {
        if (point_in_killbox(x,y,w,h,upsy.rabbit.x,upsy.rabbit.y)) {
          rabbit_squash();
        }
      }
      if (upsy.crocodile.present) {
        if (point_in_killbox(x,y,w,h,upsy.crocodile.x,upsy.crocodile.y)) {
          upsy.crocodile.present=0;
          upsy_sfx(RID_sound_squash_croc);
          fireworks_start(upsy.crocodile.x,upsy.crocodile.y);
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
    if (flame->clock<flame->offtime) continue;
    int dstx=SCENEX+flame->x*TILESIZE;
    int dsty=SCENEY+flame->y*TILESIZE;
    int frame=6+(((int)(flame->clock*10.0))&1);
    int srcy=TILESIZE*frame;
    int srcx=TILESIZE*6;
    if (flame->w<0) {
      int i=-flame->w; for (;i-->0;) {
        dstx-=TILESIZE;
        if (!i) srcx=TILESIZE*8;
        gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,GFX_XFORM_XREV);
        srcx=TILESIZE*7;
      }
    } else if (flame->w>0) {
      int i=flame->w; for (;i-->0;) {
        dstx+=TILESIZE;
        if (!i) srcx=TILESIZE*8;
        gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,0);
        srcx=TILESIZE*7;
      }
    } else if (flame->h<0) {
      int i=-flame->h; for (;i-->0;) {
        dsty-=TILESIZE;
        if (!i) srcx=TILESIZE*8;
        gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,GFX_XFORM_XREV|GFX_XFORM_SWAP);
        srcx=TILESIZE*7;
      }
    } else if (flame->h>0) {
      int i=flame->h; for (;i-->0;) {
        dsty+=TILESIZE;
        if (!i) srcx=TILESIZE*8;
        gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,GFX_XFORM_SWAP);
        srcx=TILESIZE*7;
      }
    }
  }
}
