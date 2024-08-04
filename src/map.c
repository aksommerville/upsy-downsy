#include "upsy.h"

/* Tile properties.
 */
 
int tile_is_dirt(uint8_t tileid) {
  return ((tileid>=0x03)&&(tileid<=0x0a));
}

int tile_is_sky(uint8_t tileid) {
  return (tileid<=0x02);
}

/* Render.
 */
 
static void map_render_dirt_cell(
  int dstx,int dsty,
  int nn,int nnw,int nw,int nne,int ne
) {
  /* Draw from cell 0x01 of the tilesheet.
   * It contains 4 sub-images:
   *   FULL    EDGE
   *   CONCAVE CORNER
   * CONCAVE and CORNER are drawn in the orientation where they live, EDGE has the edge on top.
   * Since dirt only rises from the bottom, our bottom row can only be EDGE or FULL.
   * Top row can be any of the 4.
   * The five provided neighbors determine it completely.
   */
  const int ht=TILESIZE>>1;
  if (nn&&nnw&&nw) {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx,dsty,TILESIZE,0,ht,ht,0);
  } else if (nn&&nw) {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx,dsty,TILESIZE,ht,ht,ht,GFX_XFORM_SWAP|GFX_XFORM_YREV);
  } else if (nn) {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx,dsty,TILESIZE+ht,0,ht,ht,GFX_XFORM_SWAP|GFX_XFORM_XREV);
  } else if (nw) {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx,dsty,TILESIZE+ht,0,ht,ht,0);
  } else {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx,dsty,TILESIZE+ht,ht,ht,ht,GFX_XFORM_XREV|GFX_XFORM_YREV);
  }
  if (nn&&nne&&ne) {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx+ht,dsty,TILESIZE,0,ht,ht,0);
  } else if (nn&&ne) {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx+ht,dsty,TILESIZE,ht,ht,ht,GFX_XFORM_XREV|GFX_XFORM_YREV);
  } else if (nn) {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx+ht,dsty,TILESIZE+ht,0,ht,ht,GFX_XFORM_SWAP|GFX_XFORM_YREV);
  } else if (ne) {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx+ht,dsty,TILESIZE+ht,0,ht,ht,0);
  } else {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx+ht,dsty,TILESIZE+ht,ht,ht,ht,GFX_XFORM_SWAP|GFX_XFORM_XREV);
  }
  if (nw) {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx,dsty+ht,TILESIZE,0,ht,ht,0);
  } else {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx,dsty+ht,TILESIZE+ht,0,ht,ht,GFX_XFORM_SWAP|GFX_XFORM_XREV);
  }
  if (ne) {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx+ht,dsty+ht,TILESIZE,0,ht,ht,0);
  } else {
    gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx+ht,dsty+ht,TILESIZE+ht,0,ht,ht,GFX_XFORM_SWAP|GFX_XFORM_YREV);
  }
}
 
void map_render() {
  if (upsy.map.dirty) {
    upsy.map.dirty=0;
    gfx_clear(upsy.texid_bgbits,0xff954f3c);
    int dstx=0,col=0;
    for (;col<COLC;col++,dstx+=TILESIZE) {
      int dsty=(ROWC-1)*TILESIZE;
      int extent=0;
      for (;extent<upsy.map.dirt[col];extent++,dsty-=TILESIZE) {
        int nn=0,nnw=0,nw=0,nne=0,ne=0;
        if (extent<upsy.map.dirt[col]-1) nn=1;
        if (col) {
          if (extent<upsy.map.dirt[col-1]-1) nnw=nw=1;
          else if (extent==upsy.map.dirt[col-1]-1) nw=1;
        } else nnw=nw=1;
        if (col<COLC-1) {
          if (extent<upsy.map.dirt[col+1]-1) nne=ne=1;
          else if (extent==upsy.map.dirt[col+1]-1) ne=1;
        } else nne=ne=1;
        if (!dsty) nn=nnw=nne=1;
        map_render_dirt_cell(dstx,dsty,nn,nnw,nw,nne,ne);
      }
    }
  }
  gfx_blit(0,upsy.texid_bgbits,SCENEX,SCENEY,0,0,-1,-1,0);
  
  gfx_blit(
    0,upsy.texid_tiles,
    SCENEX+upsy.map.carrotx*TILESIZE,SCENEY+upsy.map.carroty*TILESIZE,
    7*TILESIZE,2*TILESIZE,TILESIZE,TILESIZE,0
  );
}
