/* crocodile.h
 */
 
#ifndef CROCODILE_H
#define CROCODILE_H

struct crocodile {
  int present;
  double x,y; // tiles
  double dx; // tile/s
  double pauseclock;
  double animclock;
  int frame;
};

void crocodile_dirt_changed();
void crocodile_update(double elapsed);
void crocodile_render();

#endif
