#ifndef FIREWORKS_H
#define FIREWORKS_H

#define FIREWORKS_CIRCLE_COUNT 7

struct fireworks {
  int present;
  double ttl;
  struct fireworks_circle {
    double x,y;
    double dx,dy;
    double clock; // each circle animates independently
    uint8_t tileid;
  } circlev[FIREWORKS_CIRCLE_COUNT];
};

void fireworks_clear();
void fireworks_start(double x,double y);
void fireworks_update(double elapsed);
void fireworks_render();

#endif
