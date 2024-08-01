#include "upsy.h"

/* Prepare scene.
 */
 
int prepare_scene(int sceneid) {
  //TODO Load scenes from a resource.
  //...define that resource type. etc etc...
  upsy.sceneid=sceneid;
  
  memset(upsy.map,0,sizeof(upsy.map));
  memset(upsy.map+6*COLC,0x03,COLC);
  memset(upsy.map+7*COLC,0x08,COLC*(ROWC-7));
  upsy.bgbits_dirty=1;
  
  upsy.focus.x=COLC>>1;
  
  upsy.rabbit.x=COLC*0.5;
  upsy.rabbit.y=5.5;
  upsy.rabbit.dx=1.0;
  
  //TODO sprites
  
  return 0;
}

/* Trivial animations.
 */

static void animate_everything(double elapsed) {
  if ((upsy.focus.animclock-=elapsed)<=0.0) {
    upsy.focus.animclock+=0.200;
    if (++(upsy.focus.animframe)>=3) upsy.focus.animframe=0;
  }
  if ((upsy.rabbit.animclock-=elapsed)<=0.0) {
    upsy.rabbit.animclock+=0.125;
    if (++(upsy.rabbit.animframe)>=4) upsy.rabbit.animframe=0;
  }
  if ((upsy.crocodile.animclock-=elapsed)<=0.0) {
    upsy.crocodile.animclock+=0.200;
    if (++(upsy.crocodile.animframe)>=4) upsy.crocodile.animframe=0;
  }
}

/* Update the rabbit. Don't worry about animation, it's done.
 */
 
static void rabbit_landed() {
  //TODO WALK or CHILL?
  upsy.rabbit.state=RABBIT_STATE_WALK;
}

static int rabbit_measure_freedom() {
  int y=(int)upsy.rabbit.y;
  if ((y<0)||(y>=ROWC-1)) return 0;
  int x=(int)upsy.rabbit.x;
  if ((x<0)||(x>=COLC)) return 0;
  int w=0;
  const uint8_t *p=upsy.map+y*COLC+x;
  while ((x+w<COLC)&&tile_is_sky(p[w])&&tile_is_dirt(p[w+COLC])) w++;
  while ((x>0)&&tile_is_sky(p[-1])&&tile_is_dirt(p[COLC-1])) { x--; p--; w++; }
  return w;
}

static int rabbit_collide_or_step_off() {
  int xa=(int)(upsy.rabbit.x-0.45);
  int xz=(int)(upsy.rabbit.x+0.45);
  if (xa<0) return 1;
  if (xz>=COLC) return 1;
  int y=(int)(upsy.rabbit.y);
  if (y<0) return 1;
  if (y>=ROWC-1) return 1;
  const uint8_t *p=upsy.map+y*COLC+xa;
  int i=xz-xa+1;
  for (;i-->0;p++) {
    if (tile_is_dirt(p[0])) return 1;
    if (tile_is_sky(p[COLC])) return 1;
  }
  return 0;
}
 
static void update_rabbit(double elapsed) {
  switch (upsy.rabbit.state) {
  
    case RABBIT_STATE_CHILL: {
      } break;
  
    case RABBIT_STATE_WALK: {
        double step=upsy.rabbit.dx*elapsed;
        upsy.rabbit.x+=step;
        if (
          (upsy.rabbit.x<0.5)||
          (upsy.rabbit.x>COLC-0.5)||
          rabbit_collide_or_step_off()
        ) {
          upsy.rabbit.x-=step;
          int floorc=rabbit_measure_freedom();
          if (floorc<2) {
            upsy.rabbit.state=RABBIT_STATE_CHILL;
          } else {
            upsy.rabbit.dx=-upsy.rabbit.dx;
          }
        }
      } break;
  
    case RABBIT_STATE_FALL: {
        upsy.rabbit.y+=5.0*elapsed;
        int x=(int)(upsy.rabbit.x);
        int y=(int)(upsy.rabbit.y+0.5);
        if (y>=ROWC) {
          upsy.rabbit.y=ROWC-0.5;
          rabbit_landed();
        } else if (tile_is_dirt(upsy.map[y*COLC+x])) {
          upsy.rabbit.y=y-0.5;
          rabbit_landed();
        }
      } break;
  }
}

/* Update.
 */

void update_scene(double elapsed) {
  animate_everything(elapsed);
  update_rabbit(elapsed);
}

/* Move focus.
 */
 
void move_focus(int d) {
  upsy.focus.x+=d;
  if (upsy.focus.x<0) upsy.focus.x=COLC-1;
  else if (upsy.focus.x>=COLC) upsy.focus.x=0;
  //TODO sound effect
}

/* Map changed.
 * Review the whole map for neighbor joins.
 * Check for sprites that need to fall or get squashed.
 */
 
static void map_changed() {
  upsy.bgbits_dirty=1;
  uint8_t *p=upsy.map;
  int y=0; for (;y<ROWC;y++) {
    int x=0; for (;x<COLC;x++,p++) {
      if (tile_is_dirt(*p)) {
        if (!y||tile_is_dirt(p[-COLC])) { // 7,8,9,10
          int l=(!x||tile_is_dirt(p[-1]));
          int r=((x>=COLC-1)||tile_is_dirt(p[1]));
          if (l&&r) *p=0x08;
          else if (l) *p=0x09;
          else if (r) *p=0x07;
          else *p=0x0a;
        } else { // 2,4,5,6
          int l=(!x||tile_is_dirt(p[-1]));
          int r=((x>=COLC-1)||tile_is_dirt(p[1]));
          if (l&&r) *p=0x03;
          else if (l) *p=0x06;
          else if (r) *p=0x04;
          else *p=0x05;
        }
      }
    }
  }
  { // Check Rabbit.
    int rx=(int)upsy.rabbit.x;
    int ry=(int)upsy.rabbit.y;
    if ((rx>=0)&&(ry>=0)&&(rx<COLC)&&(ry<ROWC)) {
      if (tile_is_dirt(upsy.map[ry*COLC+rx])) {
        if ((ry>0)&&tile_is_sky(upsy.map[(ry-1)*COLC+rx])) {
          upsy.rabbit.y-=1.0;
        } else {
          upsy.rabbit.state=RABBIT_STATE_DEAD;
          //TODO sound effect "squash"
          //TODO blood and guts
          //TODO game over
        }
      } else if ((ry<ROWC-1)&&tile_is_sky(upsy.map[(ry+1)*COLC+rx])) {
        upsy.rabbit.state=RABBIT_STATE_FALL;
        //TODO sound effect "falling rabbit"
      }
    }
  }
}

/* Raise or lower earth.
 */
 
int tile_is_dirt(uint8_t tileid) {
  return ((tileid>=0x03)&&(tileid<=0x0a));
}

int tile_is_sky(uint8_t tileid) {
  return (tileid<=0x02);
}
 
void change_world(int d) {
  if (upsy.rabbit.state==RABBIT_STATE_DEAD) {
    // No changing things after the scene is final.
    return;
  }
  int x=upsy.focus.x;
  int dirtc=0;
  while ((dirtc<ROWC)&&(tile_is_dirt(upsy.map[(ROWC-dirtc-1)*COLC+x]))) dirtc++;
  int dirtlimit=dirtc;
  while ((dirtlimit<ROWC)&&(tile_is_sky(upsy.map[(ROWC-dirtlimit-1)*COLC+x]))) dirtlimit++;
  if (d<0) {
    if (dirtc>=dirtlimit) {
      //TODO sound effect "reject"
      return;
    }
    dirtc++;
    upsy.map[(ROWC-dirtc)*COLC+x]=0x03;
    map_changed();
    //TODO sound effect "more dirt"
  } else if (d>0) {
    if (dirtc<=0) {
      //TODO sound effect "reject"
      return;
    }
    upsy.map[(ROWC-dirtc)*COLC+x]=0x00;
    map_changed();
    //TODO sound effect "less dirt"
  }
}
