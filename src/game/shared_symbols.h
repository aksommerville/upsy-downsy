#ifndef SHARED_SYMBOLS_H
#define SHARED_SYMBOLS_H

#define EGGDEV_importUtil "stdlib,res"

#define CMD_scene_hawk      0x01 /* --- */
#define CMD_scene_rabbit    0x20 /* u8:x u8:y */
#define CMD_scene_carrot    0x21 /* u8:x u8:y */
#define CMD_scene_song      0x22 /* u16:songid */
#define CMD_scene_time      0x24 /* u16:ms */
#define CMD_scene_crocodile 0x25 /* u8:x u8:y */
#define CMD_scene_dirt      0xe0 /* u8*10:colh */
#define CMD_scene_platform  0xe1 /* u8:x u8:y u8:w ; Variable-length because there isn't a natural 3-byte opcode. */
#define CMD_scene_flame     0xe2 /* u8 u8 u8 u8 u16 u16 TODO */
#define CMD_scene_hammer    0xe3 /* u8 u8 u16 u16 TODO */

#endif
