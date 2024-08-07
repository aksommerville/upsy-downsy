#ifndef FLAMES_H
#define FLAMES_H

#define FLAMES_LIMIT 16

struct flames {
  struct flame {
    int x,y,w,h; // w,h signed; all in tiles
    double clock;
    double period;
    double offtime;
  } v[FLAMES_LIMIT];
  int c;
};

void flames_clear();
int flames_add(int x,int y,int w,int h,int period,int phase);
void flames_update(double elapsed);
void flames_render();

#endif
