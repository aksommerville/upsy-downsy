#ifndef FOCUS_H
#define FOCUS_H

struct focus {
  int x; // tiles
  double clock;
  int frame;
};

void focus_move(int d);
void focus_shift(int d);
void focus_update(double elapsed);

void focus_render();

#endif
