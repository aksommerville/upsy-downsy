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
    while ((srcp<srcc)&&((unsigned char)src[srcp]>0x20)) { kwc++; srcp++; }
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
    
    if ((kwc==8)&&!memcmp(kw,"platform",8)) {
      ASSERTARGC(3)
      map_add_platform(argv[0],argv[1],argv[2],0x0f);
      continue;
    }
    
    if ((kwc==5)&&!memcmp(kw,"flame",5)) {
      ASSERTARGC(6)
      if (map_add_flamethrower(argv[0],argv[1],argv[2],argv[3])>=0) {
        flames_add(argv[0],argv[1],argv[2],argv[3],argv[4],argv[5]);
      }
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

  if (!upsy.sceneid) { // Clear session stats.
    upsy.mortc_total=0;
    upsy.score=0;
    upsy.totaltime=0.0;
  }

  const char *src=0;
  int srcc=rom_get(&src,PBL_TID_scene,sceneid);
  if (srcc<1) {
    upsy.sceneid=0;
    upsy_play_song(4);
    upsy_save_hiscore_if();
    upsy.mortc=0;
    upsy.stagetime=0.0;
    upsy.victoryclock=0.0;
    return 0;
  }
  upsy.sceneid=sceneid;
  
  memset(upsy.map.dirt,0,sizeof(upsy.map.dirt));
  upsy.map.dirty=1;
  upsy.map.platformc=0;
  upsy.rabbit.dx=1.0;
  upsy.rabbit.state=RABBIT_STATE_INIT;
  upsy.focus.x=COLC>>1;
  upsy.hammer.w=0;
  upsy.crocodile.present=0;
  fireworks_clear();
  upsy.hawk.present=0;
  flames_clear();
  
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
  if (upsy.rabbit.state!=RABBIT_STATE_DEAD) {
    upsy.stagetime+=elapsed;
    upsy.totaltime+=elapsed;
  }
  focus_update(elapsed);
  rabbit_update(elapsed);
  hammer_update(elapsed);
  crocodile_update(elapsed);
  fireworks_update(elapsed);
  hawk_update(elapsed);
  flames_update(elapsed);
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
  flames_render();
  fireworks_render();
  focus_render();

  // Blot margins. Important to do this after drawing the proper scene, so sprites can't touch the margins.
  gfx_fill_rect(0,0,SCENEY,SCENEX,SCREENH,0);
  gfx_fill_rect(0,SCREENW-SCENEX,SCENEY,SCENEX,SCREENH,0);
  gfx_fill_rect(0,0,0,SCREENW,SCENEY,0);
  
  { // Status row.
    char msg[16]=" ##/##     #### ";
    msg[1]='0'+upsy.sceneid/10;
    msg[2]='0'+upsy.sceneid%10;
    msg[4]='0'+upsy.scenec/10;
    msg[5]='0'+upsy.scenec%10;
    msg[11]='0'+upsy.score/1000;
    msg[12]='0'+(upsy.score/100)%10;
    msg[13]='0'+(upsy.score/10)%10;
    msg[14]='0'+upsy.score%10;
    upsy_render_text(0,0,msg,sizeof(msg));
  }
  
  if (upsy.victoryclock>0.0) { // Victory stats, if play is complete.
    gfx_darken(0);
    upsy_render_text(10,10,"LEVEL CLEAR",11);
    { char msg[16]="     CLEAR t### ";
      msg[12]='0'+upsy.clear_bonus/100;
      msg[13]='0'+(upsy.clear_bonus/10)%10;
      msg[14]='0'+upsy.clear_bonus%10;
      upsy_render_text(0,18,msg,16);
    }
    { char msg[16]=" TIME #:## t### ";
      int sec=(int)upsy.stagetime;
      int min=sec/60;
      if (min>9) { min=9; sec=99; }
      else sec%=60;
      msg[6]='0'+min;
      msg[8]='0'+sec/10;
      msg[9]='0'+sec%10;
      msg[12]='0'+upsy.time_bonus/100;
      msg[13]='0'+(upsy.time_bonus/10)%10;
      msg[14]='0'+upsy.time_bonus%10;
      upsy_render_text(0,22,msg,16);
    }
    { char msg[16]="  DEATH ## t### ";
      if (upsy.mortc>99) msg[8]=msg[9]='9';
      else {
        msg[8]='0'+upsy.mortc/10;
        msg[9]='0'+upsy.mortc%10;
      }
      msg[12]='0'+upsy.death_bonus/100;
      msg[13]='0'+(upsy.death_bonus/10)%10;
      msg[14]='0'+upsy.death_bonus%10;
      upsy_render_text(0,26,msg,16);
    }
  }
}

/* Render text.
 */
 
void upsy_render_text(int dstx,int dsty,const char *src,int srcc) {
  for (;srcc-->0;src++,dstx+=4) {
    int srcx,srcy;
         if ((*src>='a')&&(*src<='m')) { srcx=((*src)-'a')*3; srcy=6; }
    else if ((*src>='A')&&(*src<='M')) { srcx=((*src)-'A')*3; srcy=6; }
    else if ((*src>='n')&&(*src<='z')) { srcx=((*src)-'n')*3; srcy=9; }
    else if ((*src>='N')&&(*src<='Z')) { srcx=((*src)-'N')*3; srcy=9; }
    else if ((*src>='0')&&(*src<='9')) { srcx=((*src)-'0')*3; srcy=12; }
    else if (*src==':') { srcx=30; srcy=12; }
    else if (*src=='.') { srcx=33; srcy=12; }
    else if (*src=='/') { srcx=36; srcy=12; }
    else continue;
    gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,3,3,0);
  }
}
