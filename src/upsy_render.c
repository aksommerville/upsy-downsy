#include "upsy.h"

/* Render scene.
 */

/* The proper scene is 60x60. Centered horizontally, at the bottom of our 64x64 framebuffer.
 * Those left and right margins go to waste.
 * Top margin, we can draw status on it.
 */
#define SCENEX 2
#define SCENEY 4
 
void render_scene() {

  // Redraw bgbits if needful.
  if (upsy.bgbits_dirty) {
    upsy.bgbits_dirty=0;
    const uint8_t *src=upsy.map;
    int dsty=0,yi=ROWC;
    for (;yi-->0;dsty+=TILESIZE) {
      int dstx=0,xi=COLC;
      for (;xi-->0;dstx+=TILESIZE,src++) {
        int srcx=((*src)&15)*TILESIZE;
        int srcy=((*src)>>4)*TILESIZE;
        gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,0);
      }
    }
  }
  
  // bgbits
  gfx_blit(0,upsy.texid_bgbits,SCENEX,SCENEY,0,0,-1,-1,0);
  
  { // Rabbit.
    int dstx=SCENEX+((int)(upsy.rabbit.x*TILESIZE))-(TILESIZE>>1);
    int dsty=SCENEY+((int)(upsy.rabbit.y*TILESIZE))-(TILESIZE>>1);
    uint8_t tileid=0x17;
    if (upsy.rabbit.state==RABBIT_STATE_WALK) tileid+=upsy.rabbit.animframe;
    int srcx=(tileid&15)*TILESIZE;
    int srcy=(tileid>>4)*TILESIZE;
    uint8_t xform=0;
    if (upsy.rabbit.dx<0.0) xform=GFX_XFORM_XREV;
    gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,xform);
  }
  //TODO other sprites
  
  { // Focus column.
    int dstx=SCENEX+upsy.focus.x*TILESIZE;
    int srcx=upsy.focus.animframe*TILESIZE;
    int srcy=TILESIZE*4; // middle
    gfx_blit(0,upsy.texid_tiles,dstx,SCENEY,srcx,srcy-TILESIZE,TILESIZE,TILESIZE,0);
    gfx_blit(0,upsy.texid_tiles,dstx,SCREENH-TILESIZE,srcx,srcy+TILESIZE,TILESIZE,TILESIZE,0);
    int dsty=SCENEY+TILESIZE;
    int yi=ROWC-2;
    for (;yi-->0;dsty+=TILESIZE) {
      gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,0);
    }
  }

  // Blot margins. Important to do this after drawing the proper scene, so sprites can't touch the margins.
  gfx_fill_rect(0,0,0,SCREENW,SCENEY,0);
  gfx_fill_rect(0,0,SCENEY,SCENEX,SCREENH,0);
  gfx_fill_rect(0,SCREENW-SCENEX,0,SCENEY,SCREENH,0);
  
  { // Status row.
    //TODO status
  }
}
