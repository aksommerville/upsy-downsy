#include "gfx_internal.h"

struct gfx gfx={0};

/* Quit.
 */
 
static void gfx_texture_cleanup(struct gfx_texture *tex) {
  if (tex->v) free(tex->v);
}
 
void gfx_quit() {
  if (gfx.texturev) {
    while (gfx.texturec-->0) gfx_texture_cleanup(gfx.texturev+gfx.texturec);
    free(gfx.texturev);
  }
  memset(&gfx,0,sizeof(gfx));
}

/* Init.
 */
 
int gfx_init(int fbw,int fbh) {
  if (gfx.texturec>0) {
    if ((gfx.texturev[0].w!=fbw)||(gfx.texturev[0].h!=fbh)) return -1;
    return 0;
  }
  if (gfx_texture_new(fbw,fbh)!=0) {
    gfx_quit();
    return -1;
  }
  return 0;
}

/* Delete texture.
 */
 
void gfx_texture_del(int texid) {
  if ((texid<1)||(texid>gfx.texturec)) return; // Deleting [0] is not allowed.
  struct gfx_texture *tex=gfx.texturev+texid;
  gfx_texture_cleanup(tex);
  memset(tex,0,sizeof(struct gfx_texture));
}

/* New texture.
 */
 
static struct gfx_texture *gfx_texture_alloc() {
  if (gfx.texturec<gfx.texturea) {
    struct gfx_texture *tex=gfx.texturev+gfx.texturec++;
    memset(tex,0,sizeof(struct gfx_texture));
    return tex;
  }
  struct gfx_texture *tex=gfx.texturev;
  int i=gfx.texturec;
  for (;i-->0;tex++) {
    if (!tex->v) return tex;
  }
  if (gfx.texturec>=GFX_TEXTURE_COUNT_LIMIT) return 0;
  int na=gfx.texturea+8;
  if (na>INT_MAX/sizeof(struct gfx_texture)) return 0;
  void *nv=realloc(gfx.texturev,sizeof(struct gfx_texture)*na);
  if (!nv) return 0;
  gfx.texturev=nv;
  gfx.texturea=na;
  tex=gfx.texturev+gfx.texturec++;
  memset(tex,0,sizeof(struct gfx_texture));
  return tex;
}
 
int gfx_texture_new(int w,int h) {
  if ((w<1)||(w>GFX_TEXTURE_SIZE_LIMIT)) return -1;
  if ((h<1)||(h>GFX_TEXTURE_SIZE_LIMIT)) return -1;
  struct gfx_texture *tex=gfx_texture_alloc();
  if (!tex) return -1;
  if (!(tex->v=calloc(w*h,4))) return -1;
  tex->w=w;
  tex->h=h;
  return tex-gfx.texturev;
}

/* New texture from image id.
 */
 
struct gfx_texcvt {
  int rid;
  int egg_texid;
  int w,h;
  int stride,len; // knowable from (w,h)
  void *rgba;
  int gfx_texid;
};

static void gfx_texcvt_cleanup(struct gfx_texcvt *ctx) {
  if (ctx->egg_texid>0) egg_texture_del(ctx->egg_texid);
  if (ctx->rgba) free(ctx->rgba);
}

// From only {rid}, populate {egg_texid,w,h,stride,len,rgba}.
static int gfx_texcvt_acquire_image(struct gfx_texcvt *ctx) {
  if ((ctx->egg_texid=egg_texture_new())<1) return -1;
  if (egg_texture_load_image(ctx->egg_texid,ctx->rid)<0) return -1;
  egg_texture_get_size(&ctx->w,&ctx->h,ctx->egg_texid);
  if ((ctx->w<1)||(ctx->h<1)) return -1;
  if (ctx->w>INT_MAX>>2) return -1;
  ctx->stride=ctx->w<<2;
  if (ctx->stride>INT_MAX/ctx->h) return -1;
  ctx->len=ctx->stride*ctx->h;
  if (!(ctx->rgba=malloc(ctx->len))) return -1;
  if (egg_texture_get_pixels(ctx->rgba,ctx->len,ctx->egg_texid)<0) return -1;
  return 0;
}

// From {w,h,stride,rgba}, install globally and populate {gfx_texid}.
// Hands off (rgba) and clears in (ctx) if the handoff succeeds.
static int gfx_texcvt_install(struct gfx_texcvt *ctx) {
  struct gfx_texture *tex=gfx_texture_alloc();
  if (!tex) return -1;
  tex->w=ctx->w;
  tex->h=ctx->h;
  tex->v=ctx->rgba;
  ctx->rgba=0;
  ctx->gfx_texid=tex-gfx.texturev;
  return 0;
}
 
int gfx_texture_new_image(int rid) {
  struct gfx_texcvt ctx={.rid=rid};
  int err;
  if ((err=gfx_texcvt_acquire_image(&ctx))>=0) {
    if ((err=gfx_texcvt_install(&ctx))>=0) {
      err=ctx.gfx_texid;
    }
  }
  gfx_texcvt_cleanup(&ctx);
  return err;
}

/* Finish frame.
 */

void gfx_finish() {
  if (gfx.texturec<1) return;
  const struct gfx_texture *tex=gfx.texturev+0;
  egg_texture_load_raw(1,tex->w,tex->h,tex->w<<2,tex->v,tex->w*tex->h*4);
}
