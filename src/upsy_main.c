#include "upsy.h"

struct upsy upsy={0};

void pbl_client_quit(int status) {
}

static int load_image(int imageid) {
  const void *src=0;
  int srcc=rom_get(&src,PBL_TID_image,imageid);
  struct image *image=image_decode(src,srcc);
  if (!image) {
    pbl_log("Failed to decode image:%d",imageid);
    return -2;
  }
  if (image_force_rgba(image)<0) {
    image_del(image);
    return -1;
  }
  int texid=gfx_texture_new_rgba(image->w,image->h,image->stride,image->v,image_get_pixels_length(image));
  image_del(image);
  return texid;
}

int pbl_client_init(int fbw,int fbh,int rate,int chanc) {
  if ((fbw!=64)||(fbh!=64)) return -1;
  pbl_set_synth_limit(sizeof(upsy.audio)/sizeof(upsy.audio[0]));
  if (rom_init()<0) return -1;
  if (gfx_init(fbw,fbh)<0) return -1;
  if (lofi_init(rate,chanc)<0) return -1;
  
  if ((upsy.texid_tiles=load_image(1))<0) return -1;
  if ((upsy.texid_title=load_image(3))<0) return -1;
  if ((upsy.texid_bgbits=gfx_texture_new(TILESIZE*COLC,TILESIZE*ROWC))<0) return -1;
  
  lofi_wave_init_sine(0);
  lofi_wave_init_square(1);
  lofi_wave_init_saw(2);
  lofi_wave_init_triangle(3);
  { uint8_t coefv[]={0x80,0x40,0x20,0x10,0x08}; lofi_wave_init_harmonics(4,coefv,sizeof(coefv)); }
  { uint8_t coefv[]={0xa0,0x00,0x40,0x00,0x10}; lofi_wave_init_harmonics(5,coefv,sizeof(coefv)); }
  { uint8_t coefv[]={0x40,0x50,0x30,0x10,0x08,0x10,0x08,0x10}; lofi_wave_init_harmonics(6,coefv,sizeof(coefv)); }
  { uint8_t coefv[]={0xff,0xc0,0x80,0x40,0x20,0x10}; lofi_wave_init_harmonics(7,coefv,sizeof(coefv)); }
  
  upsy.sceneid=0;
  upsy_play_song(4);
  //if (prepare_scene(1)<0) return -1;
  
  return 0;
}

void pbl_client_update(double elapsed,int in1,int in2,int in3,int in4) {
  in1|=in2|in3|in4;
  if (in1!=upsy.pvinput) {
    if (upsy.sceneid&&(upsy.rabbit.state!=RABBIT_STATE_DEAD)) {
      if ((in1&PBL_BTN_LEFT)&&!(upsy.pvinput&PBL_BTN_LEFT)) focus_move(-1);
      if ((in1&PBL_BTN_RIGHT)&&!(upsy.pvinput&PBL_BTN_RIGHT)) focus_move(1);
      if ((in1&PBL_BTN_UP)&&!(upsy.pvinput&PBL_BTN_UP)) focus_shift(-1);
      if ((in1&PBL_BTN_DOWN)&&!(upsy.pvinput&PBL_BTN_DOWN)) focus_shift(1);
    } else {
      if ((in1&PBL_BTN_SOUTH)&&!(upsy.pvinput&PBL_BTN_SOUTH)) {
        if (prepare_scene(1)<0) {
          pbl_log("Failed to load scene 1. Aborting.");
          pbl_terminate(1);
          return;
        }
      }
    }
    upsy.pvinput=in1;
  }
  if (upsy.sceneid) {
    update_scene(elapsed);
  }
}

void *pbl_client_synth(int samplec) {
  lofi_update(upsy.audio,samplec);
  return upsy.audio;
}

void *pbl_client_render() {
  if (upsy.sceneid) {
    render_scene();
  } else {
    gfx_blit(0,upsy.texid_title,0,0,0,0,64,64,0);
  }
  return gfx_finish();
}

void upsy_play_song(int songid) {
  if (songid==upsy.songid) return;
  const void *serial=0;
  int serialc=rom_get(&serial,PBL_TID_song,songid);
  lofi_play_song(serial,serialc);
  upsy.songid=songid;
}
