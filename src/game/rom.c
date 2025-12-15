#include "upsy.h"

/* Welcome one resource.
 * Return >0 to record in TOC.
 */
 
static int rom_welcome(int tid,int rid,const void *v,int c) {
  switch (tid) {
    case EGG_TID_scene: {
        if (rid>upsy.scenec) upsy.scenec=rid; // Assume the IDs are contiguous.
      } return 1;
  }
  return 0;
}

/* Initialize ROM and res TOC.
 */
 
int rom_init() {
  if ((upsy.romc=egg_rom_get(0,0))<1) return -1;
  if (!(upsy.rom=malloc(upsy.romc))) return -1;
  if (egg_rom_get(upsy.rom,upsy.romc)!=upsy.romc) return -1;
  struct rom_reader reader;
  if (rom_reader_init(&reader,upsy.rom,upsy.romc)<0) return -1;
  struct rom_entry res;
  while (rom_reader_next(&res,&reader)>0) {
    int err=rom_welcome(res.tid,res.rid,res.v,res.c);
    if (err<0) return err;
    if (!err) continue; // Zero to leave unlisted.
    if (upsy.resc>=upsy.resa) {
      int na=upsy.resa+32;
      if (na>INT_MAX/sizeof(struct rom_entry)) return -1;
      void *nv=realloc(upsy.resv,sizeof(struct rom_entry)*na);
      if (!nv) return -1;
      upsy.resv=nv;
      upsy.resa=na;
    }
    upsy.resv[upsy.resc++]=res; // rom_reader_next returns them naturally sorted, so just append.
  }
  return 0;
}

/* Search TOC.
 */
 
int rom_search(int tid,int rid) {
  int lo=0,hi=upsy.resc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
    const struct rom_entry *q=upsy.resv+ck;
         if (tid<q->tid) hi=ck;
    else if (tid>q->tid) lo=ck+1;
    else if (rid<q->rid) hi=ck;
    else if (rid>q->rid) lo=ck+1;
    else return ck;
  }
  return -lo-1;
}

/* Get resource.
 */
 
int rom_get(void *dstpp,int tid,int rid) {
  int p=rom_search(tid,rid);
  if (p<0) return 0;
  const struct rom_entry *res=upsy.resv+p;
  *(const void**)dstpp=res->v;
  return res->c;
}
