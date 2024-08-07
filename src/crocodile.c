#include "upsy.h"

/* Test collisions.
 * With (solid_only), allow that he could be floating.
 */
 
static int crocodile_collision(int solid_only) {
  if (upsy.crocodile.dx<0.0) {
    if (upsy.crocodile.x<1.0) return 1;
  } else {
    if (upsy.crocodile.x>COLC-1.0) return 1;
  }
  
  /* Check for dirt on my row, and also gaps on the row below.
   */
  int y=(int)(upsy.crocodile.y);
  if (y<0) y=0; else if (y>=ROWC) y=ROWC-1;
  int x=(int)(upsy.crocodile.x);
  if (x<0) x=0; else if (x>=COLC) x=COLC-1;
  if (cell_solid(x,y)) return 1;
  if (!solid_only&&(y<ROWC-1)) {
    if (!cell_solid(x,y+1)) return 1;
  }

  return 0;
}

/* Look for the rabbit and eat him if we overlap.
 */
 
static void crocodile_check_rabbit() {
  double dy=upsy.rabbit.y-upsy.crocodile.y;
  if ((dy<-0.25)||(dy>0.25)) return;
  double dx=upsy.rabbit.x-upsy.crocodile.x;
  if ((dx<-1.0)||(dx>1.0)) return;
  rabbit_squash();
  upsy.crocodile.frame=2;
}

/* Dirt changed.
 */
 
void crocodile_dirt_changed() {
  if (!upsy.crocodile.present) return;
  
  /* If we are in a state of collision after this change, try bumping up by one row.
   */
  if (crocodile_collision(1)) {
    upsy.crocodile.y-=1.0;
    if (!crocodile_collision(1)) return;
    upsy.crocodile.y+=1.0;
    
  /* No collision where we are. Can we fall by one row?
   * Crocodiles don't experience gravity (check wikipedia), they fall by discrete intervals the moment it's possible.
   */
  } else if (upsy.crocodile.y<ROWC-1.0) {
    upsy.crocodile.y+=1.0;
    if (crocodile_collision(1)) {
      upsy.crocodile.y-=1.0;
    }
  }
}

/* Update.
 */
 
void crocodile_update(double elapsed) {
  if (!upsy.crocodile.present) return;
  if (upsy.victoryclock>0.0) return;
  if (upsy.rabbit.state==RABBIT_STATE_DEAD) return;
  
  if (upsy.crocodile.pauseclock>0.0) {
    if ((upsy.crocodile.pauseclock-=elapsed)>0.0) {
      if (upsy.crocodile.frame) {
        if ((upsy.crocodile.animclock-=elapsed)<=0.0) {
          upsy.crocodile.frame=0;
        }
      }
      return;
    }
    upsy.crocodile.dx*=-1.0;
  }
  
  if (upsy.crocodile.frame<2) {
    if ((upsy.crocodile.animclock-=elapsed)<=0.0) {
      upsy.crocodile.animclock+=0.250;
      if (++(upsy.crocodile.frame)>=2) upsy.crocodile.frame=0;
    }
  }
  
  if (upsy.crocodile.frame<2) {
    upsy.crocodile.x+=upsy.crocodile.dx*elapsed;
    if (crocodile_collision(0)) {
      upsy.crocodile.x-=upsy.crocodile.dx*elapsed;
      upsy.crocodile.pauseclock=0.5;
    } else {
      crocodile_check_rabbit();
    }
  }
}

/* Render.
 */
 
void crocodile_render() {
  if (!upsy.crocodile.present) return;
  int srcx=TILESIZE*6;
  int srcy=TILESIZE*(3+upsy.crocodile.frame);
  int dstx=SCENEX+(int)(upsy.crocodile.x*TILESIZE)-TILESIZE;
  int dsty=SCENEY+(int)(upsy.crocodile.y*TILESIZE)-(TILESIZE>>1);
  uint8_t xform=0;
  if (upsy.crocodile.dx>0.0) xform=GFX_XFORM_XREV;
  gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE<<1,TILESIZE,xform);
}
