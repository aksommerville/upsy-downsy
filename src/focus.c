#include "upsy.h"

/* Shuffle focussed column.
 */
 
void focus_shift(int d) {

  // No rearranging the world after the rabbit is dead.
  if (upsy.rabbit.state==RABBIT_STATE_DEAD) return;
  if (upsy.victoryclock>0.0) return;

  if (d<0) { // up (positive for dirt)
    if (upsy.map.dirt[upsy.focus.x]>=ROWC) {
      upsy_sfx_reject_grow();
      return;
    }
    // Under the hammer, we can only grow to the second row.
    if ((upsy.focus.x>=upsy.hammer.x)&&(upsy.focus.x<upsy.hammer.x+upsy.hammer.w)) {
      if (upsy.map.dirt[upsy.focus.x]>=ROWC-1) {
        upsy_sfx_reject_grow();
        return;
      }
    }
    upsy_sfx_grow();
    upsy.map.dirt[upsy.focus.x]++;
    
  } else { // down
    if (!upsy.map.dirt[upsy.focus.x]) {
      upsy_sfx_reject_shrink();
      return;
    }
    upsy_sfx_shrink();
    upsy.map.dirt[upsy.focus.x]--;
  }
  // Dirt levels have changed.
  upsy.map.dirty=1;
  rabbit_dirt_changed();
  crocodile_dirt_changed();
}

/* Move to another column.
 */
 
void focus_move(int d) {
  upsy.focus.x+=d;
  if (upsy.focus.x<0) upsy.focus.x=COLC-1;
  else if (upsy.focus.x>=COLC) upsy.focus.x=0;
}

/* Update.
 */
 
void focus_update(double elapsed) {
  if ((upsy.focus.clock-=elapsed)<=0.0) {
    upsy.focus.clock+=0.250;
    if (++(upsy.focus.frame)>=3) upsy.focus.frame=0;
  }
}

/* Render.
 */
 
void focus_render() {
  if (upsy.rabbit.state==RABBIT_STATE_DEAD) return;
  if (upsy.victoryclock>0.0) return;
  int dstx=SCENEX+upsy.focus.x*TILESIZE;
  int srcx=upsy.focus.frame*TILESIZE;
  int srcy=TILESIZE*4; // middle
  gfx_blit(0,upsy.texid_tiles,dstx,SCENEY,srcx,srcy-TILESIZE,TILESIZE,TILESIZE,0);
  gfx_blit(0,upsy.texid_tiles,dstx,SCREENH-TILESIZE,srcx,srcy+TILESIZE,TILESIZE,TILESIZE,0);
  int dsty=SCENEY+TILESIZE;
  int yi=ROWC-2;
  for (;yi-->0;dsty+=TILESIZE) {
    gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,0);
  }
}
