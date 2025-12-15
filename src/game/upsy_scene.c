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
 
static int decode_and_apply_scene(const void *src,int srcc) {
  struct cmdlist_reader reader={.v=src,.c=srcc};
  struct cmdlist_entry cmd;
  while (cmdlist_reader_next(&cmd,&reader)>0) {
    // All commads are fixed length, but some are odd sizes that cmdlist doesn't accomodate, so we use variable-length opcodes for them.
    // To keep it simple and safe, assert every command length.
    #define ARGC(n) if (cmd.argc!=n) { \
      fprintf(stderr,"scene:%d command 0x%02x, expected %d bytes arg, found %d.\n",upsy.sceneid,cmd.opcode,n,cmd.argc); \
      return -1; \
    }
    #define S8(n) (((n)&0x80)?((n)|~0xff):(n))
    switch (cmd.opcode) {
      case CMD_scene_dirt: ARGC(COLC) memcpy(upsy.map.dirt,cmd.arg,COLC); break;
      case CMD_scene_rabbit: ARGC(2) upsy.rabbit.x=cmd.arg[0]+0.5; upsy.rabbit.y=cmd.arg[1]+0.5; break;
      case CMD_scene_carrot: ARGC(2) upsy.map.carrotx=cmd.arg[0]; upsy.map.carroty=cmd.arg[1]; break;
      case CMD_scene_song: ARGC(2) upsy_play_song((cmd.arg[0]<<8)|cmd.arg[1]); break;
      case CMD_scene_hammer: {
          ARGC(6)
          upsy.hammer.x=cmd.arg[0];
          upsy.hammer.w=cmd.arg[1];
          if ((upsy.hammer.x<0)||(upsy.hammer.w<0)||(upsy.hammer.w==1)||(upsy.hammer.x+upsy.hammer.w>COLC)) return -1;
          if ((upsy.hammer.period=((cmd.arg[2]<<8)|cmd.arg[3])/1000.0)<1.0) return -1;
          upsy.hammer.clock=((cmd.arg[4]<<8)|cmd.arg[5])/1000.0;
          upsy.hammer.h=1.0;
          upsy.hammer.dh=0.0;
        } break;
      case CMD_scene_crocodile: {
          ARGC(2)
          upsy.crocodile.x=cmd.arg[0]+0.5;
          upsy.crocodile.y=cmd.arg[1]+0.5;
          upsy.crocodile.present=1;
          upsy.crocodile.dx=2.0;
          upsy.crocodile.frame=0;
          upsy.crocodile.pauseclock=1.0;
        } break;
      case CMD_scene_hawk: {
          ARGC(0)
          upsy.hawk.present=1;
          upsy.hawk.x=COLC*0.5;
          upsy.hawk.y=0.5;
          upsy.hawk.dx=1.5;
          upsy.hawk.attack=0;
        } break;
      case CMD_scene_platform: {
          ARGC(3)
          map_add_platform(cmd.arg[0],cmd.arg[1],cmd.arg[2],0x0f);
        } break;
      case CMD_scene_flame: {
          ARGC(8)
          if (map_add_flamethrower(cmd.arg[0],cmd.arg[1],S8(cmd.arg[2]),S8(cmd.arg[3]))>=0) {
            flames_add(cmd.arg[0],cmd.arg[1],S8(cmd.arg[2]),S8(cmd.arg[3]),(cmd.arg[4]<<8)|cmd.arg[5],(cmd.arg[6]<<8)|cmd.arg[7]);
          }
        } break;
      case CMD_scene_time: {
          ARGC(2)
          upsy.map.timetarget=((cmd.arg[0]<<8)|cmd.arg[1])/1000.0;
        } break;
      default: {
          fprintf(stderr,"Unexpected command 0x%02x in scene:%d\n",cmd.opcode,upsy.sceneid);
          return -1;
        }
    }
    #undef S8
    #undef ARGC
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
  int srcc=rom_get(&src,EGG_TID_scene,sceneid);
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
  upsy.map.timetarget=10.0;
  upsy.rabbit.dx=1.0;
  upsy.rabbit.state=RABBIT_STATE_INIT;
  upsy.focus.x=COLC>>1;
  upsy.hammer.w=0;
  upsy.crocodile.present=0;
  fireworks_clear();
  upsy.hawk.present=0;
  flames_clear();
  
  if (decode_and_apply_scene(src,srcc)<0) {
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
    char msg[5];
    msg[0]='0'+upsy.sceneid/10;
    msg[1]='0'+upsy.sceneid%10;
    msg[2]='/';
    msg[3]='0'+upsy.scenec/10;
    msg[4]='0'+upsy.scenec%10;
    upsy_render_text(1,0,msg,5);
    msg[0]='0'+upsy.score/1000;
    msg[1]='0'+(upsy.score/100)%10;
    msg[2]='0'+(upsy.score/10)%10;
    msg[3]='0'+upsy.score%10;
    upsy_render_text(SCREENW-16,0,msg,4);
  }
  
  if (upsy.victoryclock>0.0) { // Victory stats, if play is complete.
    gfx_darken(0);
    upsy_render_text(10,10,"LEVEL CLEAR",11);
    { char msg[15]="    CLEAR t### ";
      msg[11]='0'+upsy.clear_bonus/100;
      msg[12]='0'+(upsy.clear_bonus/10)%10;
      msg[13]='0'+upsy.clear_bonus%10;
      upsy_render_text(2,18,msg,sizeof(msg));
    }
    { char msg[15]="TIME #:## t### ";
      int sec=(int)upsy.stagetime;
      int min=sec/60;
      if (min>9) { min=9; sec=99; }
      else sec%=60;
      msg[5]='0'+min;
      msg[7]='0'+sec/10;
      msg[8]='0'+sec%10;
      msg[11]='0'+upsy.time_bonus/100;
      msg[12]='0'+(upsy.time_bonus/10)%10;
      msg[13]='0'+upsy.time_bonus%10;
      upsy_render_text(2,22,msg,sizeof(msg));
    }
    { char msg[15]=" DEATH ## t### ";
      if (upsy.mortc>99) msg[7]=msg[8]='9';
      else {
        msg[7]='0'+upsy.mortc/10;
        msg[8]='0'+upsy.mortc%10;
      }
      msg[11]='0'+upsy.death_bonus/100;
      msg[12]='0'+(upsy.death_bonus/10)%10;
      msg[13]='0'+upsy.death_bonus%10;
      upsy_render_text(2,26,msg,sizeof(msg));
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
