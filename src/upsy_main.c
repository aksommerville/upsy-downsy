#include "upsy.h"
#include "image/image.h"
#include "gfx/gfx.h"
#include "util/rom.h"

struct upsy upsy={0};

void pbl_client_quit(int status) {
}

int pbl_client_init(int fbw,int fbh,int rate,int chanc) {
  if ((fbw!=64)||(fbh!=64)) return -1;
  pbl_set_synth_limit(0);//TODO We do want audio eventually.
  if (rom_init()<0) return -1;
  if (gfx_init(fbw,fbh)<0) return -1;
  
  { // Load image:1 as upsy.texid_tiles.
    const void *src=0;
    int srcc=rom_get(&src,PBL_TID_image,1);
    struct image *image=image_decode(src,srcc);
    if (!image) {
      pbl_log("Failed to decode image:1");
      return -1;
    }
    if (image_force_rgba(image)<0) return -1;
    if ((upsy.texid_tiles=gfx_texture_new_rgba(image->w,image->h,image->stride,image->v,image_get_pixels_length(image)))<0) return -1;
    image_del(image);
  }
  
  { // Generate bgbits.
    if ((upsy.texid_bgbits=gfx_texture_new(TILESIZE*COLC,TILESIZE*ROWC))<0) return -1;
    upsy.bgbits_dirty=1;
  }
  
  //XXX TEMP Generate map.
  {
    uint8_t *p=upsy.map+6*COLC;
    int c=COLC;
    for (;c-->0;p++) *p=0x02;
    c=(ROWC-3)*COLC;
    for (;c-->0;p++) *p=0x08;
  }
  upsy.map[3*COLC+5]=0x0a;
  upsy.map[3*COLC+6]=0x0a;
  upsy.map[3*COLC+7]=0x0a;
  upsy.focuscol=COLC>>1;
  
  return 0;
}

static int animframe=1;
static double animclock=0.0;
static double spritex=10.0;
static double spritedx=20.0; // px/s

static void upsy_move_focus(int d) {
  upsy.focuscol+=d;
  if (upsy.focuscol<0) upsy.focuscol=COLC-1;
  else if (upsy.focuscol>=COLC) upsy.focuscol=0;
}

static void upsy_change_world(int d) {
  int skyc=0;
  uint8_t *p=upsy.map+upsy.focuscol;
  while (skyc<ROWC) {
    if (*p) break; // sky is only tile zero
    skyc++;
    p+=COLC;
  }
  skyc+=d;
  if (skyc<0) return;
  if (skyc>ROWC) return;
  int i=skyc;
  for (p=upsy.map+upsy.focuscol;i-->0;p+=COLC) *p=0x00;
  if (skyc<ROWC) {
    *p=0x02;
    p+=COLC;
    for (i=ROWC-skyc-1;i-->0;p+=COLC) *p=0x08;
  }
  //TODO join and break neighbors
  upsy.bgbits_dirty=1;
}

void pbl_client_update(double elapsed,int in1,int in2,int in3,int in4) {
  if ((animclock-=elapsed)<=0.0) {
    animclock+=0.200;
    if (++animframe>=4) animframe=0;
  }
  spritex+=elapsed*spritedx;
  if ((spritex>50.0)&&(spritedx>0.0)) {
    spritedx=-spritedx;
  } else if ((spritex<10.0)&&(spritedx<0.0)) {
    spritedx=-spritedx;
  }
  if ((upsy.focusclock-=elapsed)<=0.0) {
    upsy.focusclock+=0.125;
    if (++(upsy.focusframe)>=3) upsy.focusframe=0;
  }
  in1|=in2|in3|in4;
  if (in1!=upsy.pvinput) {
    if ((in1&PBL_BTN_LEFT)&&!(upsy.pvinput&PBL_BTN_LEFT)) upsy_move_focus(-1);
    if ((in1&PBL_BTN_RIGHT)&&!(upsy.pvinput&PBL_BTN_RIGHT)) upsy_move_focus(1);
    if ((in1&PBL_BTN_UP)&&!(upsy.pvinput&PBL_BTN_UP)) upsy_change_world(-1);
    if ((in1&PBL_BTN_DOWN)&&!(upsy.pvinput&PBL_BTN_DOWN)) upsy_change_world(1);
    upsy.pvinput=in1;
  }
}

void *pbl_client_synth(int samplec) {
  return 0;//TODO Synthesizer. Write a bare bones synth for general use, in pebble repo.
}

/* Render.
 */

void *pbl_client_render() {

  // Redraw grid if dirty. This will happen every time we move earth, drawing the whole thing redundantly (it's fine).
  if (upsy.bgbits_dirty) {
    upsy.bgbits_dirty=0;
    const uint8_t *src=upsy.map;
    int yi=ROWC,dsty=0; for (;yi-->0;dsty+=TILESIZE) {
      int xi=COLC,dstx=0; for (;xi-->0;dstx+=TILESIZE,src++) {
        uint8_t tileid=*src;
        int srcx=(tileid&0x0f)*TILESIZE;
        int srcy=(tileid>>4)*TILESIZE;
        gfx_blit(upsy.texid_bgbits,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,0);
      }
    }
  }

  // Copy bgbits to fb and blot out the 3 margins.
  int bgcolor=0x402010;
  gfx_fill_rect(0,0,0,64,4,bgcolor);
  gfx_fill_rect(0,0,0,2,64,bgcolor);
  gfx_fill_rect(0,64-2,0,2,64,bgcolor);
  gfx_blit(0,upsy.texid_bgbits,2,4,0,0,-1,-1,0);
  
  { //TODO Sprites
    uint8_t tileid=0x17+animframe;
    int srcx=(tileid&0x0f)*TILESIZE;
    int srcy=(tileid>>4)*TILESIZE;
    uint8_t xform=0;
    if (spritedx<0.0) xform=GFX_XFORM_XREV;
    gfx_blit(0,upsy.texid_tiles,(int)spritex-3,34,srcx,srcy,TILESIZE,TILESIZE,xform);
  }
  {
    gfx_blit(0,upsy.texid_tiles,38,16,7*TILESIZE,2*TILESIZE,TILESIZE,TILESIZE,0);
  }
  
  { // Highlight focused column.
    int srcx=upsy.focusframe*TILESIZE;
    int srcy=TILESIZE*4;
    int dstx=2+TILESIZE*upsy.focuscol;
    gfx_blit(0,upsy.texid_tiles,dstx,4,srcx,srcy-TILESIZE,TILESIZE,TILESIZE,0);
    gfx_blit(0,upsy.texid_tiles,dstx,64-TILESIZE,srcx,srcy+TILESIZE,TILESIZE,TILESIZE,0);
    int dsty=4+TILESIZE;
    int yi=ROWC-2;
    for (;yi-->0;dsty+=TILESIZE) {
      gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,0);
    }
  }
  
  { // Status bar: Up to 16 glyphs, along the top row.
    //TODO Decide what we want up here.
    char msg[16]="L:12 S:345 T:678";
    const char *statustext=msg;
    int dstx=1,limit=16;
    for (;(limit-->0)&&*statustext;dstx+=4,statustext++) {
      char ch=*statustext;
      int srcx,srcy;
           if ((ch>='a')&&(ch<='m')) { srcx=(ch-'a')*3; srcy=6; }
      else if ((ch>='A')&&(ch<='M')) { srcx=(ch-'A')*3; srcy=6; }
      else if ((ch>='n')&&(ch<='z')) { srcx=(ch-'n')*3; srcy=9; }
      else if ((ch>='N')&&(ch<='Z')) { srcx=(ch-'N')*3; srcy=9; }
      else if ((ch>='0')&&(ch<='9')) { srcx=(ch-'0')*3; srcy=12; }
      else if (ch==':') { srcx=30; srcy=12; }
      else if (ch=='.') { srcx=33; srcy=12; }
      else continue;
      gfx_blit(0,upsy.texid_tiles,dstx,0,srcx,srcy,3,3,0);
    }
  }
  
  return gfx_finish();
}
