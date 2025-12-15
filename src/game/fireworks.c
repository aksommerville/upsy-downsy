#include "upsy.h"

#define FIREWORKS_SPEED 4.0

/* Clear.
 */
 
void fireworks_clear() {
  upsy.fireworks.present=0;
}

/* Start.
 */
 
void fireworks_start(double x,double y) {
  upsy.fireworks.present=1;
  upsy.fireworks.ttl=5.0;
  struct fireworks_circle *circle=upsy.fireworks.circlev;
  int i=FIREWORKS_CIRCLE_COUNT;
  double t=0.0;
  double dt=(M_PI*2.0)/FIREWORKS_CIRCLE_COUNT;
  double clock=0.0;
  int frame=0x60;
  for (;i-->0;circle++,t+=dt) {
    circle->x=x;
    circle->y=y;
    circle->dx=sin(t)*FIREWORKS_SPEED;
    circle->dy=cos(t)*FIREWORKS_SPEED;
    circle->tileid=frame;
    circle->clock=clock;
    if (++frame>0x64) frame=0x60;
    clock+=0.050;
  }
}

/* Update.
 */
 
void fireworks_update(double elapsed) {
  if (!upsy.fireworks.present) return;
  if ((upsy.fireworks.ttl-=elapsed)<=0.0) {
    upsy.fireworks.present=0;
    return;
  }
  struct fireworks_circle *circle=upsy.fireworks.circlev;
  int i=FIREWORKS_CIRCLE_COUNT;
  for (;i-->0;circle++) {
    circle->x+=circle->dx*elapsed;
    circle->y+=circle->dy*elapsed;
    if ((circle->clock-=elapsed)<=0.0) {
      circle->clock+=0.125;
      if (++(circle->tileid)>0x64) circle->tileid=0x60;
    }
  }
}

/* Render.
 */
 
void fireworks_render() {
  if (!upsy.fireworks.present) return;
  struct fireworks_circle *circle=upsy.fireworks.circlev;
  int i=FIREWORKS_CIRCLE_COUNT;
  for (;i-->0;circle++) {
    int srcx=(circle->tileid&15)*TILESIZE;
    int srcy=(circle->tileid>>4)*TILESIZE;
    int dstx=SCENEX+(int)((circle->x-0.5)*TILESIZE);
    int dsty=SCENEY+(int)((circle->y-0.5)*TILESIZE);
    gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,0);
  }
}
