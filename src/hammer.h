/* hammer.h
 */
 
#ifndef HAMMER_H
#define HAMMER_H

struct hammer {
  uint8_t x,w; // tiles; w==0 means no hammer, otherwise >=2
  double h; // tiles
  double period; // s
  double clock; // counts up, stops when dh nonzero
  double dh;
};

void hammer_update(double elapsed);
void hammer_render();

#endif
