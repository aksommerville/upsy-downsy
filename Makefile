all:
.SILENT:
.SECONDARY:
PRECMD=echo "  $@" ; mkdir -p $(@D) ;

PEBBLE_SDK:=../pebble/

PBLTOOL:=$(PEBBLE_SDK)/out/pbltool/pbltool

CC_WASM:=clang --target=wasm32 -c -O3 -MMD -nostdlib -Isrc -I$(PEBBLE_SDK)/src -Wno-comment -Wno-incompatible-library-redeclaration -Wno-parentheses
LD_WASM:=$(firstword $(shell which wasm-ld wasm-ld-11)) \
  --no-entry -z stack-size=4194304 --no-gc-sections --allow-undefined --export-table \
  --export=pbl_client_init --export=pbl_client_quit --export=pbl_client_update --export=pbl_client_render --export=pbl_client_synth
LDPOST_WASM:=
CC_NATIVE:=gcc -c -MMD -O3 -Isrc -I$(PEBBLE_SDK)/src -Werror -Wimplicit
AR_NATIVE:=ar

SRCFILES:=$(shell find src -type f)
CFILES:=$(filter %.c,$(SRCFILES))
DATAFILES:=$(filter src/data/%,$(SRCFILES))
OFILES_WASM:=$(patsubst src/%.c,mid/wasm/%.o,$(CFILES))
OFILES_NATIVE:=$(patsubst src/%.c,mid/native/%.o,$(CFILES))
LIB_WASM:=mid/data/code.wasm
LIB_NATIVE:=mid/libupsy.a

mid/wasm/%.o:src/%.c;$(PRECMD) $(CC_WASM) -o$@ $<
mid/native/%.o:src/%.c;$(PRECMD) $(CC_NATIVE) -o$@ $<
$(LIB_WASM):$(OFILES_WASM);$(PRECMD) $(LD_WASM) -o$@ $(OFILES_WASM) $(LDPOST_WASM)
$(LIB_NATIVE):$(OFILES_NATIVE);$(PRECMD) $(AR_NATIVE) rc $@ $(OFILES_NATIVE)

ROM:=out/upsy-downsy.pbl
HTML:=out/upsy-downsy.html
EXE_TRUE:=out/upsy-downsy.true
EXE_FAKE:=out/upsy-downsy.fake
all:$(ROM) $(HTML)
$(ROM):$(LIB_WASM) $(DATAFILES);$(PRECMD) $(PBLTOOL) pack -o$@ $(LIB_WASM) src/data
$(HTML):$(ROM);$(PRECMD) $(PBLTOOL) bundle -o$@ $(ROM)
$(EXE_TRUE):$(ROM) $(LIB_NATIVE);$(PRECMD) $(PBLTOOL) bundle -o$@ $(ROM) $(LIB_NATIVE)
$(EXE_FAKE):$(ROM);$(PRECMD) $(PBLTOOL) bundle -o$@ $(ROM)

run:$(ROM);$(PEBBLE_SDK)/out/linux/pebble $(ROM)
serve:$(ROM);$(PBLTOOL) serve #TODO
clean:;rm -r mid out
