#include "upsy.h"

/* Measure freedom of movement in a cardinal direction.
 * May return <0 if you are already in a state of collision.
 */
 
static double rabbit_measure_freedom(int dx,int dy) {
  
  /* Start with distance to the screen's edge. Rabbit is not allowed offscreen in any direction.
   */
  double freedom;
  if (dx<0) {
    freedom=upsy.rabbit.x-0.5;
  } else if (dx>0) {
    freedom=COLC-upsy.rabbit.x-0.5;
  } else if (dy<0) {
    freedom=upsy.rabbit.y-0.5;
  } else if (dy>0) {
    freedom=ROWC-upsy.rabbit.y-0.5;
  } else return 0.0;
  if (freedom<0.0) return freedom;
  
  /* Consider solid bodies other than dirt. Surely there will be platforms and such. TODO
   */
  if (dy<0) {
    if ((upsy.rabbit.x>=upsy.hammer.x)&&(upsy.rabbit.x<=upsy.hammer.x+upsy.hammer.w)) {
      double distance=upsy.rabbit.y-upsy.hammer.h;
      if (distance<freedom) freedom=distance;
    }
  } else if (upsy.hammer.h>upsy.rabbit.y) {
    if (dx<0) {
      double distance=upsy.rabbit.x-0.45-(double)(upsy.hammer.x+upsy.hammer.w);
      if ((distance>-1.0)&&(distance<freedom)) freedom=distance;
    } else if (dx>0) {
      double distance=(double)upsy.hammer.x-upsy.rabbit.x-0.45;
      if ((distance>-1.0)&&(distance<freedom)) freedom=distance;
    }
  }
   
  /* Consider dirt.
   */
  if (dx<0) {
    int col=(int)(upsy.rabbit.x-0.5);
    int minextent=ROWC-(int)(upsy.rabbit.y+0.45);
    while (col>=0) {
      if (upsy.map.dirt[col]>=minextent) {
        double distance=upsy.rabbit.x-0.5-(col+1.0);
        if (distance<freedom) freedom=distance;
        break;
      }
      col--;
    }
  } else if (dx>0) {
    int col=(int)(upsy.rabbit.x+0.5);
    int minextent=ROWC-(int)(upsy.rabbit.y+0.45);
    while (col<COLC) {
      if (upsy.map.dirt[col]>=minextent) {
        double distance=col-upsy.rabbit.x-0.5;
        if (distance<freedom) freedom=distance;
        break;
      }
      col++;
    }
  } else if (dy>0) {
    int xl=(int)(upsy.rabbit.x-0.45);
    int xr=(int)(upsy.rabbit.x+0.45);
    if (xl<0) xl=0;
    if (xr>=COLC) xr=COLC-1;
    if (xr<xl) xr=xl;
    int extent=(upsy.map.dirt[xl]>upsy.map.dirt[xr])?upsy.map.dirt[xl]:upsy.map.dirt[xr];
    double floor=(double)(ROWC-extent);
    double distance=floor-upsy.rabbit.y-0.5;
    if (distance<freedom) freedom=distance;
  }
  
  return freedom;
}

static double rabbit_measure_road(int dx) {
  int extent=ROWC-(int)upsy.rabbit.y-1;
  double road;
  if (dx<0) {
    int solcol=(int)(upsy.rabbit.x-0.45);
    while ((solcol>=0)&&(upsy.map.dirt[solcol]==extent)) solcol--;
    road=upsy.rabbit.x-0.45-(solcol+1.0);
  } else {
    int solcol=(int)(upsy.rabbit.x+0.45);
    while ((solcol<COLC)&&(upsy.map.dirt[solcol]==extent)) solcol++;
    road=(double)solcol-upsy.rabbit.x-0.45;
  }
  double free=rabbit_measure_freedom(dx,0);
  if (free<road) return free;
  return road;
}

/* Check my horizontal freedom and enter WALK or CHILL state.
 * Call when we have solid footing.
 */
 
static void rabbit_examine_road() {
  double lfree=rabbit_measure_road(-1);
  double rfree=rabbit_measure_road(1);
  if (lfree+rfree>1.5) {
    upsy.rabbit.state=RABBIT_STATE_WALK;
    upsy.rabbit.xlo=upsy.rabbit.x-lfree;
    upsy.rabbit.xhi=upsy.rabbit.x+rfree;
  } else {
    upsy.rabbit.state=RABBIT_STATE_CHILL;
  }
}

/* Rabbit has been squashed. Blood will appear at his feet.
 * Find the column that squashed me, and center on it, so we don't show blood dripping down from nowhere.
 */
 
static void rabbit_amend_position_for_death() {
  int footrow=(int)(upsy.rabbit.y+1.0);
  int footextent=ROWC-footrow;
  int nearcol=(int)(upsy.rabbit.x);
  const uint8_t *b=upsy.map.dirt;
  if ((nearcol>=0)&&(nearcol<COLC)&&(upsy.map.dirt[nearcol]==footextent)) {
    upsy.rabbit.x=nearcol+0.5;
  } else if ((nearcol>=1)&&(upsy.map.dirt[nearcol-1]==footextent)) {
    upsy.rabbit.x=nearcol-0.5;
  } else if ((nearcol<COLC-1)&&(upsy.map.dirt[nearcol+1]==footextent)) {
    upsy.rabbit.x=nearcol+1.5;
  }
}

/* Kill rabbit.
 */
 
void rabbit_squash() {
  upsy_sfx_squash();
  upsy_play_song(3);
  rabbit_amend_position_for_death();
  upsy.rabbit.state=RABBIT_STATE_DEAD;
  upsy.rabbit.frame=0;
  upsy.rabbit.animclock=0.200;
}

/* Dirt changed.
 */
 
void rabbit_dirt_changed() {
  if (upsy.rabbit.state==RABBIT_STATE_DEAD) return;
  
  double yfree=rabbit_measure_freedom(0,1);
  
  if (yfree>0.0) {
    upsy.rabbit.state=RABBIT_STATE_FALL;
  
  } else if (yfree<0.0) {
    upsy.rabbit.y+=yfree;
    if (rabbit_measure_freedom(0,-1)<-0.5) {
      rabbit_squash();
      return;
    }
    rabbit_examine_road();
    
  } else {
    rabbit_examine_road();
  }
}

/* Update.
 */
 
void rabbit_update(double elapsed) {
  if ((upsy.rabbit.animclock-=elapsed)<=0.0) {
    switch (upsy.rabbit.state) {
      case RABBIT_STATE_DEAD: {
          upsy.rabbit.animclock+=0.200;
          if (++(upsy.rabbit.frame)>=6) upsy.rabbit.frame=5;
        } break;
      default: {
          upsy.rabbit.animclock+=0.250;
          if (++(upsy.rabbit.frame)>=4) upsy.rabbit.frame=0;
        }
    }
  }
  switch (upsy.rabbit.state) {
  
    case RABBIT_STATE_INIT: {
        if (rabbit_measure_freedom(0,1)>0.0) {
          upsy.rabbit.state=RABBIT_STATE_FALL;
        } else {
          rabbit_examine_road();
        }
      } break;
  
    case RABBIT_STATE_WALK: {
        if (!upsy.rabbit.dx) upsy.rabbit.dx=RABBIT_WALK_SPEED;
        upsy.rabbit.x+=upsy.rabbit.dx*elapsed;
        if (upsy.rabbit.dx>0.0) {
          double free=rabbit_measure_road(1);
          if (free<=0.0) {
            upsy.rabbit.x+=free;
            upsy.rabbit.dx=-upsy.rabbit.dx;
          }
        } else {
          double free=rabbit_measure_road(-1);
          if (free<=0.0) {
            upsy.rabbit.x-=free;
            upsy.rabbit.dx=-upsy.rabbit.dx;
          }
        }
      } break;
      
    case RABBIT_STATE_FALL: {
        upsy.rabbit.y+=elapsed*3.0;//TODO Should gravity accelerate?
        double yfree=rabbit_measure_freedom(0,1);
        if (yfree<=0.0) {
          upsy.rabbit.y+=yfree;
          rabbit_examine_road();
        }
      } break;
  }
  if (upsy.rabbit.state!=RABBIT_STATE_DEAD) {
    int cx=(int)(upsy.rabbit.x);
    int cy=(int)(upsy.rabbit.y);
    if ((cx==upsy.map.carrotx)&&(cy==upsy.map.carroty)) {
      upsy.victoryclock=UPSY_VICTORY_TIME;
      upsy_sfx_victory();
      upsy.rabbit.frame=0;
    }
  }
}
  
/* Render.
 */
 
void rabbit_render() {
  int dstx=SCENEX+((int)(upsy.rabbit.x*TILESIZE))-(TILESIZE>>1);
  int dsty=SCENEY+((int)(upsy.rabbit.y*TILESIZE))-(TILESIZE>>1);
  uint8_t tileid=0x17;
  switch (upsy.rabbit.state) {
    case RABBIT_STATE_DEAD: {
        tileid=0x28+upsy.rabbit.frame;
        dsty+=TILESIZE;
      } break;
    case RABBIT_STATE_WALK: {
        tileid+=upsy.rabbit.frame;
      } break;
  }
  int srcx=(tileid&15)*TILESIZE;
  int srcy=(tileid>>4)*TILESIZE;
  uint8_t xform=0;
  if (upsy.rabbit.dx<0.0) xform=GFX_XFORM_XREV;
  gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,xform);
}
