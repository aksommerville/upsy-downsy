#include "upsy.h"

struct upsy upsy={0};

void egg_client_quit(int status) {
}

void egg_client_notify(int k,int v) {
}

int egg_client_init() {

  int fbw=0,fbh=0;
  egg_texture_get_size(&fbw,&fbh,1);
  if ((fbw!=SCREENW)||(fbh!=SCREENH)) return -1;

  if (rom_init()<0) return -1;
  if (gfx_init(fbw,fbh)<0) return -1;
  
  if ((upsy.texid_tiles=gfx_texture_new_image(RID_image_tiles))<0) return -1;
  if ((upsy.texid_title=gfx_texture_new_image(RID_image_title))<0) return -1;
  if ((upsy.texid_bgbits=gfx_texture_new(TILESIZE*COLC,TILESIZE*ROWC))<0) return -1;
  
  upsy.sceneid=0;
  upsy_play_song(RID_song_hello_rabbit);
  upsy_load_hiscore();
  
  #if BACKWARD_SCENES
    fprintf(stderr,"*** BACKWARD_SCENES enabled. Don't release like this! ***\n");
    prepare_scene(upsy.scenec);
  #endif
  
  return 0;
}

static void upsy_apply_scores(double elapsed) {
  if (!upsy.clear_bonus&&!upsy.time_bonus&&!upsy.death_bonus) return;
  int xferc=(int)(elapsed*100.0);
  if (xferc<1) xferc=1;
  if (upsy.clear_bonus>xferc) {
    upsy.clear_bonus-=xferc;
    upsy.score+=xferc;
    upsy_sfx(RID_sound_score_tick);
    return;
  } else if (upsy.clear_bonus>0) {
    upsy.score+=upsy.clear_bonus;
    xferc-=upsy.clear_bonus;
    upsy.clear_bonus=0;
  }
  if (upsy.time_bonus>xferc) {
    upsy.time_bonus-=xferc;
    upsy.score+=xferc;
    upsy_sfx(RID_sound_score_tick);
    return;
  } else if (upsy.time_bonus>0) {
    upsy.score+=upsy.time_bonus;
    xferc-=upsy.time_bonus;
    upsy.time_bonus=0;
  }
  if (upsy.death_bonus>xferc) {
    upsy.death_bonus-=xferc;
    upsy.score+=xferc;
    upsy_sfx(RID_sound_score_tick);
    return;
  } else if (upsy.death_bonus>0) {
    upsy.score+=upsy.death_bonus;
    xferc-=upsy.death_bonus;
    upsy.death_bonus=0;
  }
  upsy_sfx(RID_sound_score_tick);
  if (!upsy.clear_bonus&&!upsy.time_bonus&&!upsy.death_bonus) {
    upsy_save_hiscore_if();
  }
}

void egg_client_update(double elapsed) {
  int in1=egg_input_get_one(0);
  upsy.totalclock+=elapsed;
  if (in1!=upsy.pvinput) {
    if (in1&(EGG_BTN_AUX3|EGG_BTN_AUX2)) egg_terminate(0);
    if (upsy.sceneid&&(upsy.victoryclock>0.0)) {
      if ((in1&EGG_BTN_SOUTH)&&!(upsy.pvinput&EGG_BTN_SOUTH)) {
        upsy_apply_scores(999.0);
        upsy.mortc=0;
        upsy.stagetime=0.0;
        upsy.victoryclock=0.0;
        int d=1;
        #if BACKWARD_SCENES
          d=-1;
        #endif
        if (prepare_scene(upsy.sceneid+d)<0) {
          egg_terminate(1);
        }
      }
    } else if (upsy.sceneid&&(upsy.rabbit.state!=RABBIT_STATE_DEAD)) {
      if ((in1&EGG_BTN_LEFT)&&!(upsy.pvinput&EGG_BTN_LEFT)) focus_move(-1);
      if ((in1&EGG_BTN_RIGHT)&&!(upsy.pvinput&EGG_BTN_RIGHT)) focus_move(1);
      if ((in1&EGG_BTN_UP)&&!(upsy.pvinput&EGG_BTN_UP)) focus_shift(-1);
      if ((in1&EGG_BTN_DOWN)&&!(upsy.pvinput&EGG_BTN_DOWN)) focus_shift(1);
    } else {
      if ((in1&EGG_BTN_SOUTH)&&!(upsy.pvinput&EGG_BTN_SOUTH)) {
        if (prepare_scene(upsy.sceneid?upsy.sceneid:1)<0) {
          fprintf(stderr,"Failed to load scene 1. Aborting.\n");
          egg_terminate(1);
          return;
        }
      }
    }
    upsy.pvinput=in1;
  }
  if (upsy.sceneid) {
    if (upsy.victoryclock>0.0) {
      upsy.victoryclock+=elapsed;
      if (upsy.victoryclock>1.0) {
        upsy_apply_scores(elapsed);
      }
    } else {
      update_scene(elapsed);
    }
  }
}

void egg_client_render() {
  if (upsy.sceneid) {
    render_scene();
  } else {
    gfx_blit(0,upsy.texid_title,0,0,0,0,SCREENW,SCREENH,0);
    char msg[10]="LAST: 0000";
    msg[6]='0'+upsy.score/1000;
    msg[7]='0'+(upsy.score/100)%10;
    msg[8]='0'+(upsy.score/10)%10;
    msg[9]='0'+upsy.score%10;
    upsy_render_text(13,45,msg,10);
    memcpy(msg,"BEST",4);
    msg[6]='0'+upsy.hiscore/1000;
    msg[7]='0'+(upsy.hiscore/100)%10;
    msg[8]='0'+(upsy.hiscore/10)%10;
    msg[9]='0'+upsy.hiscore%10;
    upsy_render_text(13,49,msg,10);
  }
  gfx_finish();
}

void upsy_play_song(int songid) {
  if (songid==upsy.songid) return;
  upsy.songid=songid;
  egg_play_song(1,songid,1,0.5f,0.0f);
}

#define SFX_BLACKOUT_TIME 0.070

void upsy_sfx(int rid) {
  double now=egg_time_real();
  struct sfxbo *q=upsy.sfxbov;
  struct sfxbo *oldest=q;
  int i=SFXBO_COUNT;
  for (;i-->0;q++) {
    if (q->time<oldest->time) oldest=q;
    if (q->rid==rid) {
      double elapsed=now-q->time;
      if (elapsed<SFX_BLACKOUT_TIME) return;
      oldest=q; // Might as well overwrite this record.
      break;
    }
  }
  oldest->time=now;
  oldest->rid=rid;
  egg_play_sound(rid,1.0f,0.0f);
}

void upsy_save_hiscore() {
  char tmp[4]={
    '0'+(upsy.hiscore/1000)%10,
    '0'+(upsy.hiscore/100)%10,
    '0'+(upsy.hiscore/10)%10,
    '0'+upsy.hiscore%10,
  };
  egg_store_set("hiscore",7,tmp,4);
}

void upsy_load_hiscore() {
  upsy.hiscore=0;
  char src[16];
  int srcc=egg_store_get(src,sizeof(src),"hiscore",7);
  if ((srcc<1)||(srcc>sizeof(src))) return;
  int i=0; for (;i<srcc;i++) {
    if ((src[i]<'0')||(src[i]>'9')) { upsy.hiscore=0; return; }
    if (upsy.hiscore>INT_MAX/10) { upsy.hiscore=0; return; }
    upsy.hiscore*=10;
    int digit=src[i]-'0';
    if (upsy.hiscore>INT_MAX-digit) { upsy.hiscore=0; return; }
    upsy.hiscore+=digit;
  }
  if (upsy.hiscore>9999) {
    fprintf(stderr,"Hi score %d exceeds the limit of 9999 that i carefully arranged. Are you cheating?\n",upsy.hiscore);
    upsy.hiscore=9999;
  }
}

void upsy_save_hiscore_if() {
  if (upsy.score<=upsy.hiscore) return;
  upsy.hiscore=upsy.score;
  upsy_save_hiscore();
}
