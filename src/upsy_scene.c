#include "upsy.h"

/* Read arguments from a scene line.
 */
 
static int read_scene_args(int *argc,int *argv,int arga,const char *src,int srcc) {
  int srcp=0;
  *argc=0;
  while (srcp<srcc) {
    if (src[srcp]==0x0a) break;
    if ((unsigned char)src[srcp]<=0x20) { srcp++; continue; }
    if ((*argc)>=arga) return -1;
    argv[*argc]=0;
    int positive=1;
    if (src[srcp]=='-') {
      positive=0;
      srcp++;
    }
    while ((srcp<srcc)&&(src[srcp]>='0')&&(src[srcp]<='9')) {
      argv[*argc]*=10;
      argv[*argc]+=src[srcp++]-'0';
      if (argv[*argc]>999999) return -1; // arbitrary limit, easier than proper overflow checking
    }
    if (!positive) argv[*argc]*=-1;
    (*argc)++;
    if (srcp>=srcc) break;
    if ((unsigned char)src[srcp]>0x20) return -1;
  }
  return srcp;
}

/* Decode and apply scene.
 */
 
static int decode_and_apply_scene(const char *src,int srcc) {
  int srcp=0;
  while (srcp<srcc) {
    if (src[srcp]=='#') {
      while ((srcp<srcc)&&(src[srcp++]!=0x0a)) ;
      continue;
    }
    if ((unsigned char)src[srcp]<=0x20) {
      srcp++;
      continue;
    }
    const char *kw=src+srcp;
    int kwc=0;
    while ((srcp<srcc)&&((unsigned char)src[srcp++]>0x20)) kwc++;
    int argv[16];
    int argc=0;
    int err=read_scene_args(&argc,argv,sizeof(argv)/sizeof(argv[0]),src+srcp,srcc-srcp);
    if (err<0) return -1;
    srcp+=err;
    #define ASSERTARGC(expect) if (argc!=expect) { \
      pbl_log("scene:%d provided %d arguments for '%.*s', expected %d",upsy.sceneid,argc,kwc,kw,expect); \
      return -1; \
    }
    
    if ((kwc==4)&&!memcmp(kw,"dirt",4)) {
      ASSERTARGC(COLC)
      while (argc-->0) upsy.map.dirt[argc]=argv[argc];
      continue;
    }
    
    if ((kwc==6)&&!memcmp(kw,"rabbit",6)) {
      ASSERTARGC(2)
      upsy.rabbit.x=argv[0]+0.5;
      upsy.rabbit.y=argv[1]+0.5;
      continue;
    }
    
    if ((kwc==6)&&!memcmp(kw,"carrot",6)) {
      ASSERTARGC(2)
      upsy.map.carrotx=argv[0];
      upsy.map.carroty=argv[1];
      continue;
    }
    
    if ((kwc==4)&&!memcmp(kw,"song",4)) {
      ASSERTARGC(1)
      upsy_play_song(argv[0]);
      continue;
    }
    
    if ((kwc==6)&&!memcmp(kw,"hammer",6)) {
      ASSERTARGC(4)
      upsy.hammer.x=argv[0];
      upsy.hammer.w=argv[1];
      if ((upsy.hammer.x<0)||(upsy.hammer.w<0)||(upsy.hammer.w==1)||(upsy.hammer.x+upsy.hammer.w>COLC)) return -1;
      if ((upsy.hammer.period=argv[2]/1000.0)<1.0) return -1;
      upsy.hammer.clock=argv[3]/1000.0;
      upsy.hammer.h=1.0;
      upsy.hammer.dh=0.0;
      continue;
    }
    
    if ((kwc==9)&&!memcmp(kw,"crocodile",9)) {
      ASSERTARGC(2)
      upsy.crocodile.x=argv[0]+0.5;
      upsy.crocodile.y=argv[1]+0.5;
      upsy.crocodile.present=1;
      upsy.crocodile.dx=2.0;
      upsy.crocodile.frame=0;
      upsy.crocodile.pauseclock=1.0;
      continue;
    }
    
    if ((kwc==4)&&!memcmp(kw,"hawk",4)) {
      ASSERTARGC(0)
      upsy.hawk.present=1;
      upsy.hawk.x=COLC*0.5;
      upsy.hawk.y=0.5;
      upsy.hawk.dx=1.5;
      upsy.hawk.attack=0;
      continue;
    }
    
    pbl_log("Unexpected command '%.*s' in scene:%d",kwc,kw,upsy.sceneid);
    return -1;
    #undef ARG
  }
  return 0;
}

/* Prepare scene.
 */
 
int prepare_scene(int sceneid) {

  const char *src=0;
  int srcc=rom_get(&src,PBL_TID_scene,sceneid);
  if (srcc<1) {
    upsy.sceneid=0;
    upsy_play_song(4);
    return 0;
  }
  upsy.sceneid=sceneid;
  
  memset(upsy.map.dirt,0,sizeof(upsy.map.dirt));
  upsy.map.dirty=1;
  upsy.rabbit.dx=1.0;
  upsy.rabbit.state=RABBIT_STATE_INIT;
  upsy.focus.x=COLC>>1;
  upsy.hammer.w=0;
  upsy.crocodile.present=0;
  fireworks_clear();
  upsy.hawk.present=0;
  
  if (decode_and_apply_scene(src,srcc)<0) {
    pbl_log("Failed to decode scene:%d",sceneid);
    upsy.sceneid=0;
    upsy_play_song(4);
    return -1;
  }
  return 0;
}

/* Update.
 */

void update_scene(double elapsed) {
  focus_update(elapsed);
  rabbit_update(elapsed);
  hammer_update(elapsed);
  crocodile_update(elapsed);
  fireworks_update(elapsed);
  hawk_update(elapsed);
}

/* Render scene.
 */
 
void render_scene() {

  // The major things.
  map_render();
  rabbit_render();
  crocodile_render();
  hammer_render();
  hawk_render();
  fireworks_render();
  focus_render();

  // Blot margins. Important to do this after drawing the proper scene, so sprites can't touch the margins.
  gfx_fill_rect(0,0,0,SCREENW,SCENEY,0);
  gfx_fill_rect(0,0,SCENEY,SCENEX,SCREENH,0);
  gfx_fill_rect(0,SCREENW-SCENEX,0,SCENEY,SCREENH,0);
  
  { // Status row.
    //TODO status
  }
}
