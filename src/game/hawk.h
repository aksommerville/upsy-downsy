#ifndef HAWK_H
#define HAWK_H

struct hawk {
  int present;
  double x,y;
  double dx;
  double animclock;
  int frame;
  int attack;
};

void hawk_update(double elapsed);
void hawk_render();

#endif
