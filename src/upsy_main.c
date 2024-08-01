#include "pebble/pebble.h"

void pbl_client_quit(int status) {
  pbl_log("%s",__func__);
}

int pbl_client_init(int fbw,int fbh,int rate,int chanc) {
  pbl_log("%s",__func__);
  return 0;
}

void pbl_client_update(double elapsed,int in1,int in2,int in3,int in4) {
}

void *pbl_client_render() {
  return 0;
}

void *pbl_client_synth(int samplec) {
  return 0;//TODO Synthesizer. Write a bare bones synth for general use, in pebble repo.
}
