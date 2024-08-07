#include "upsy.h"

#define HAWK_ATTACK_SPEED 5.0

/* Update.
 */
 
void hawk_update(double elapsed) {
  if (!upsy.hawk.present) return;
  
  if (upsy.rabbit.state==RABBIT_STATE_CHILL) {
    if (!upsy.hawk.attack) {
      upsy.hawk.attack=1;
    }
  } else if (upsy.hawk.attack) {
    upsy.hawk.attack=0;
  }
  
  if ((upsy.rabbit.state==RABBIT_STATE_DEAD)||(upsy.victoryclock>0.0)) {
    // Stay put but do continue animating.
  
  } else if (upsy.hawk.attack) {
    double dx=upsy.rabbit.x-upsy.hawk.x;
    double dy=upsy.rabbit.y-upsy.hawk.y;
    double distance2=dx*dx+dy*dy;
    if (distance2<0.75) {
      rabbit_squash();
      return;
    }
    double distance=sqrt(distance2);
    dx/=distance;
    dy/=distance;
    dx*=HAWK_ATTACK_SPEED*elapsed;
    dy*=HAWK_ATTACK_SPEED*elapsed;
    upsy.hawk.x+=dx;
    upsy.hawk.y+=dy;
    if ((upsy.hawk.x<0.5)||(upsy.hawk.x>COLC-0.5)||(upsy.hawk.y>ROWC-0.5)) {
      upsy.hawk.attack=0;
    }
    
  } else if (upsy.hawk.y>0.5) {
    upsy.hawk.y-=2.0*elapsed;
    if (upsy.hawk.y<0.5) upsy.hawk.y=0.5;
  
  } else {
    upsy.hawk.x+=upsy.hawk.dx*elapsed;
    if ((upsy.hawk.x<1.0)&&(upsy.hawk.dx<0.0)) upsy.hawk.dx*=-1.0;
    else if ((upsy.hawk.x>COLC-1.0)&&(upsy.hawk.dx>0.0)) upsy.hawk.dx*=-1.0;
  }
  
  if ((upsy.hawk.animclock-=elapsed)<=0.0) {
    upsy.hawk.animclock+=0.150;
    if (++(upsy.hawk.frame)>=4) upsy.hawk.frame=0;
  }
}

/* Render.
 */
 
void hawk_render() {
  if (!upsy.hawk.present) return;
  int dstx=SCENEX+(int)(upsy.hawk.x*TILESIZE)-TILESIZE;
  int dsty=SCENEY+(int)(upsy.hawk.y*TILESIZE)-(TILESIZE>>1);
  int srcx=0;
  switch (upsy.hawk.frame) {
    case 1: case 3: srcx=TILESIZE<<1; break;
    case 2: srcx=TILESIZE<<2; break;
  }
  int srcy=TILESIZE*7;
  uint8_t xform=0;
  if (upsy.hawk.attack) {
    if (upsy.rabbit.x<upsy.hawk.x) xform=GFX_XFORM_XREV;
  } else {
    if (upsy.hawk.dx<0.0) xform=GFX_XFORM_XREV;
  }
  gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE<<1,TILESIZE,xform);
}
