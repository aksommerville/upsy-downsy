#ifndef RABBIT_H
#define RABBIT_H

#define RABBIT_STATE_INIT 0
#define RABBIT_STATE_CHILL 1
#define RABBIT_STATE_WALK 2
#define RABBIT_STATE_FALL 3
#define RABBIT_STATE_DEAD 4

#define RABBIT_WALK_SPEED 3.0

struct rabbit {
  double x,y; // tiles
  double dx; // tile/s
  double xlo,xhi; // Limits when walking.
  double animclock;
  int frame;
  int state;
};

void rabbit_dirt_changed();

void rabbit_update(double elapsed);

void rabbit_render();

#endif
