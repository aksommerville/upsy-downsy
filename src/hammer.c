#include "upsy.h"

#define HAMMER_DOWN_SPEED 16.0
#define HAMMER_UP_SPEED -2.0

/* Pack any dirt we collided with, kill any living thing, and begin moving back up.
 */
 
static void hammer_stop() {
  upsy_sfx_hammer_smash();
  upsy.hammer.h=(double)(int)(upsy.hammer.h);
  
  // Any dirt in our range gets packed down to wherever our edge is.
  // This is also a condition for stopping; it should be either zero or one level.
  int dirtlimit=ROWC-(int)(upsy.hammer.h-0.5)-1;
  if (dirtlimit<0) dirtlimit=0;
  int changed=0;
  int i=upsy.hammer.x+upsy.hammer.w;
  while (i-->upsy.hammer.x) {
    if (upsy.map.dirt[i]>dirtlimit) {
      upsy.map.dirt[i]=dirtlimit;
      upsy.map.dirty=1;
      changed=1;
    }
  }
  if (changed) {
    rabbit_dirt_changed();
    crocodile_dirt_changed();
    if (upsy.rabbit.state==RABBIT_STATE_DEAD) return;
  }
  
  // Did we kill the rabbit? Force its vertical position if so.
  if ((upsy.rabbit.y<upsy.hammer.h)&&(upsy.rabbit.x>=upsy.hammer.x)&&(upsy.rabbit.x<=upsy.hammer.x+upsy.hammer.w)) {
    rabbit_squash();
    upsy.rabbit.y=upsy.hammer.h-0.5;
  }
  
  // How about the crocodile? Kill that too.
  if (upsy.crocodile.present&&(upsy.crocodile.y<upsy.hammer.h)&&(upsy.crocodile.x>=upsy.hammer.x)&&(upsy.crocodile.x<=upsy.hammer.x+upsy.hammer.w)) {
    // Can't do a gore effect for the crocodile, since the user is free to move terrain around after.
    upsy.crocodile.present=0;
    upsy_sfx_squash_croc();
    fireworks_start(upsy.crocodile.x,upsy.crocodile.y);
  }

  upsy.hammer.dh=HAMMER_UP_SPEED;
}

/* Check collisions, when extending.
 * We are responsible for crushing things and reversing direction.
 */
 
static void hammer_check_collisions() {
  if (upsy.hammer.h>=ROWC) {
    hammer_stop();
    return;
  }
  int top=ROWC;
  int y=1; for (;y<ROWC;y++) {
    int x=upsy.hammer.x+upsy.hammer.w;
    int solid=0;
    for (;x-->upsy.hammer.x;) {
      if (cell_solid_below_topsoil(x,y)) {
        solid=1;
        top=y;
        break;
      }
    }
    if (top<ROWC) break;
  }
  if (upsy.hammer.h>=top) {
    hammer_stop();
    return;
  }
}

/* Update.
 */
 
void hammer_update(double elapsed) {
  if (upsy.hammer.w<2) return;
  if (upsy.victoryclock>0.0) return;
  if (upsy.rabbit.state==RABBIT_STATE_DEAD) return;
  if (upsy.hammer.dh<0.0) {
    if ((upsy.hammer.h+=upsy.hammer.dh*elapsed)<=1.0) {
      upsy.hammer.h=1.0;
      upsy.hammer.dh=0.0;
    }
  } else if (upsy.hammer.dh>0.0) {
    upsy.hammer.h+=upsy.hammer.dh*elapsed;
    hammer_check_collisions();
  } else {
    if ((upsy.hammer.clock+=elapsed)>=upsy.hammer.period) {
      upsy.hammer.clock=0.0;
      upsy.hammer.dh=HAMMER_DOWN_SPEED;
    }
  }
}

/* Render.
 */
 
void hammer_render() {
  if (upsy.hammer.w<2) return;
  int dsty=SCENEY+(int)(upsy.hammer.h*TILESIZE); // bottom edge
  uint8_t tileid=0x43; // left +1=center +2=right
  while (dsty>SCENEY) {
    dsty-=TILESIZE;
    int dstx=SCENEX+upsy.hammer.x*TILESIZE;
    gfx_blit(0,upsy.texid_tiles,dstx,dsty,(tileid&15)*TILESIZE,(tileid>>4)*TILESIZE,TILESIZE,TILESIZE,0);
    gfx_blit(0,upsy.texid_tiles,dstx+TILESIZE*(upsy.hammer.w-1),dsty,((tileid+2)&15)*TILESIZE,((tileid+2)>>4)*TILESIZE,TILESIZE,TILESIZE,0);
    int xi=upsy.hammer.w-2;
    dstx+=TILESIZE;
    int srcx=((tileid+1)&15)*TILESIZE;
    int srcy=((tileid+1)>>4)*TILESIZE;
    for (;xi-->0;dstx+=TILESIZE) {
      gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,0);
    }
    tileid=0x33;
  }
}
