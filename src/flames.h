#ifndef FLAMES_H
#define FLAMES_H

#define FLAMES_LIMIT 16

struct flames {
  struct flame {
    int x,y,w; // w signed; all in tiles
    double clock;
    int state;
  } v[FLAMES_LIMIT];
  int c;
};

void flames_clear();
int flames_add(int x,int y,int w);
void flames_update(double elapsed);
void flames_render();

#endif
