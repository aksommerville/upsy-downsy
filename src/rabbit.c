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
    int solcol=(int)upsy.rabbit.x;
    while ((solcol>=0)&&(upsy.map.dirt[solcol]==extent)) solcol--;
    road=upsy.rabbit.x-0.45-(solcol+1.0);
  } else {
    int solcol=(int)upsy.rabbit.x;
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

/* Dirt changed.
 */
 
void rabbit_dirt_changed() {
  
  double yfree=rabbit_measure_freedom(0,1);
  
  if (yfree>0.0) {
    upsy.rabbit.state=RABBIT_STATE_FALL;
  
  } else if (yfree<0.0) {
    upsy.rabbit.y+=yfree;
    //TODO Check for squash.
    rabbit_examine_road();
  
  } else if (rabbit_measure_freedom(0,1)>0.0) {
    upsy.rabbit.state=RABBIT_STATE_FALL;
    
  } else {
    rabbit_examine_road();
  }
}

/* Update.
 */
 
void rabbit_update(double elapsed) {
  if ((upsy.rabbit.animclock-=elapsed)<=0.0) {
    upsy.rabbit.animclock+=0.250;
    if (++(upsy.rabbit.frame)>=4) upsy.rabbit.frame=0;
  }
  switch (upsy.rabbit.state) {
  
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
          /*
          double lfree=rabbit_measure_road(-1);
          double rfree=rabbit_measure_road(1);
          pbl_log("%s road=%f,%f",__func__,lfree,rfree);
          if (lfree+rfree>2.0) {
            upsy.rabbit.state=RABBIT_STATE_WALK;
            upsy.rabbit.xlo=upsy.rabbit.x-lfree;
            upsy.rabbit.xhi=upsy.rabbit.x+rfree;
          } else {
            upsy.rabbit.state=RABBIT_STATE_CHILL;
          }*/
        }
      } break;
  }
}
  
/* Render.
 */
 
void rabbit_render() {
  int dstx=SCENEX+((int)(upsy.rabbit.x*TILESIZE))-(TILESIZE>>1);
  int dsty=SCENEY+((int)(upsy.rabbit.y*TILESIZE))-(TILESIZE>>1);
  uint8_t tileid=0x17;
  if (upsy.rabbit.state==RABBIT_STATE_WALK) tileid+=upsy.rabbit.frame;
  int srcx=(tileid&15)*TILESIZE;
  int srcy=(tileid>>4)*TILESIZE;
  uint8_t xform=0;
  if (upsy.rabbit.dx<0.0) xform=GFX_XFORM_XREV;
  gfx_blit(0,upsy.texid_tiles,dstx,dsty,srcx,srcy,TILESIZE,TILESIZE,xform);
}
