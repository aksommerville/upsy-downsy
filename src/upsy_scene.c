#include "upsy.h"

/* Prepare scene.
 */
 
int prepare_scene(int sceneid) {
  //TODO Load scenes from a resource.
  //...define that resource type. etc etc...
  upsy.sceneid=sceneid;
  upsy_play_song(1);
  
  memset(upsy.map.dirt,ROWC-6,COLC);
  upsy.map.dirt[3]++;
  upsy.map.dirt[4]--;
  upsy.map.dirty=1;
  
  upsy.focus.x=COLC>>1;
  
  upsy.rabbit.x=(COLC>>1)+0.5;
  upsy.rabbit.y=5.5;
  upsy.rabbit.dx=1.0;
  
  //TODO sprites
  
  return 0;
}

/* Update.
 */

void update_scene(double elapsed) {
  focus_update(elapsed);
  rabbit_update(elapsed);
}

/* Render scene.
 */
 
void render_scene() {
  map_render();
  rabbit_render();
  focus_render();
  //TODO other sprites

  // Blot margins. Important to do this after drawing the proper scene, so sprites can't touch the margins.
  gfx_fill_rect(0,0,0,SCREENW,SCENEY,0);
  gfx_fill_rect(0,0,SCENEY,SCENEX,SCREENH,0);
  gfx_fill_rect(0,SCREENW-SCENEX,0,SCENEY,SCREENH,0);
  
  { // Status row.
    //TODO status
  }
}
