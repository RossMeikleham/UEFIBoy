#include "memory.h"
#include "mbc.h"
#include "hdma.h"

#include <stdio.h>
#include <stdbool.h>

#include "../memory_layout.h"
#include "../rom_info.h"
#include "../graphics.h"
#include "../sprite_priorities.h"
#include "../interrupts.h"
#include "../bits.h"
#include "../sound.h"
#include "../serial_io.h"
#include "../lcd.h"

#include "../../non_core/joypad.h"
#include "../../non_core/logger.h"
#include "../../non_core/files.h"

#include <string.h>

  
static uint8_t mem[0xDFFF - 0x8000];

uint8_t *oam_mem_ptr;

// OAM Ram 0xFE00 - 0xFE9F
uint8_t oam_mem[0xA0] = {
    0xBB, 0xD8, 0xC4, 0x04, 0xCD, 0xAC, 0xA1, 0xC7,
    0x7D, 0x85, 0x15, 0xF0, 0xAD, 0x19, 0x11, 0x6A,
    0xBA, 0xC7, 0x76, 0xF8, 0x5C, 0xA0, 0x67, 0x0A,
    0x7B, 0x75, 0x56, 0x3B, 0x65, 0x5C, 0x4D, 0xA3,
    0x00, 0x05, 0xD7, 0xC9, 0x1B, 0xCA, 0x11, 0x6D,
    0x38, 0xE7, 0x13, 0x2A, 0xB1, 0x10, 0x72, 0x4D,
    0xA7, 0x47, 0x13, 0x89, 0x7C, 0x62, 0x5F, 0x90,
    0x64, 0x2E, 0xD3, 0xEF, 0xAB, 0x01, 0x15, 0x85,
    0xE8, 0x2A, 0x6E, 0x4A, 0x1F, 0xBE, 0x49, 0xB1,
    0xE6, 0x0F, 0x93, 0xE2, 0xB6, 0x87, 0x5D, 0x35,
    0xD8, 0xD4, 0x4A, 0x45, 0xCA, 0xB3, 0x33, 0x74,
    0x18, 0xC1, 0x16, 0xFB, 0x8F, 0xA4, 0x8E, 0x70,
    0xCD, 0xB4, 0x4A, 0xDC, 0xE6, 0x34, 0x32, 0x41,
    0xF9, 0x84, 0x6A, 0x99, 0xEC, 0x92, 0xF1, 0x8B,
    0x5D, 0xA5, 0x09, 0xCF, 0x3A, 0x93, 0xBC, 0xE0,
    0x15, 0x19, 0xE4, 0xB6, 0x9A, 0x04, 0x3B, 0xC1,
    0x96, 0xB7, 0x56, 0x85, 0x6A, 0xAA, 0x1E, 0x2A,
    0x80, 0xEE, 0xE7, 0x46, 0x76, 0x8B, 0x0D, 0xBA,
    0x24, 0x40, 0x42, 0x05, 0x0E, 0x04, 0x20, 0xA6,
    0x5E, 0xC1, 0x97, 0x7E, 0x44, 0x05, 0x01, 0xA9
};

// 0xFF00 - 0xFFFF
static uint8_t io_mem_dmg[0x100]= {
		0xCF, 0x00, 0x7E, 0xFF, 0xD3, 0x00, 0x00, 0xF8,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1,
		0x80, 0xBF, 0xF3, 0xFF, 0xBF, 0xFF, 0x3F, 0x00,
		0xFF, 0xBF, 0x7F, 0xFF, 0x9F, 0xFF, 0xBF, 0xFF,
		0xFF, 0x00, 0x00, 0xBF, 0x77, 0xF3, 0xF1, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x71, 0x72, 0xD5, 0x91, 0x58, 0xBB, 0x2A, 0xFA,
		0xCF, 0x3C, 0x54, 0x75, 0x48, 0xCF, 0x8F, 0xD9,
		0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFC,
		0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x2B, 0x0B, 0x64, 0x2F, 0xAF, 0x15, 0x60, 0x6D,
		0x61, 0x4E, 0xAC, 0x45, 0x0F, 0xDA, 0x92, 0xF3,
		0x83, 0x38, 0xE4, 0x4E, 0xA7, 0x6C, 0x38, 0x58,
		0xBE, 0xEA, 0xE5, 0x81, 0xB4, 0xCB, 0xBF, 0x7B,
		0x59, 0xAD, 0x50, 0x13, 0x5E, 0xF6, 0xB3, 0xC1,
		0xDC, 0xDF, 0x9E, 0x68, 0xD7, 0x59, 0x26, 0xF3,
		0x62, 0x54, 0xF8, 0x36, 0xB7, 0x78, 0x6A, 0x22,
		0xA7, 0xDD, 0x88, 0x15, 0xCA, 0x96, 0x39, 0xD3,
		0xE6, 0x55, 0x6E, 0xEA, 0x90, 0x76, 0xB8, 0xFF,
		0x50, 0xCD, 0xB5, 0x1B, 0x1F, 0xA5, 0x4D, 0x2E,
		0xB4, 0x09, 0x47, 0x8A, 0xC4, 0x5A, 0x8C, 0x4E,
		0xE7, 0x29, 0x50, 0x88, 0xA8, 0x66, 0x85, 0x4B,
		0xAA, 0x38, 0xE7, 0x6B, 0x45, 0x3E, 0x30, 0x37,
		0xBA, 0xC5, 0x31, 0xF2, 0x71, 0xB4, 0xCF, 0x29,
		0xBC, 0x7F, 0x7E, 0xD0, 0xC7, 0xC3, 0xBD, 0xCF,
		0x59, 0xEA, 0x39, 0x01, 0x2E, 0x00, 0x69, 0x00
};


static uint8_t io_mem_cgb[0x100] = {
    0xCF, 0x00, 0x7C, 0xFF, 0x44, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE1,
    0x80, 0xBF, 0xF3, 0xFF, 0xBF, 0xFF, 0x3F, 0x00, 0xFF, 0xBF, 0x7F, 0xFF, 0x9F, 0xFF, 0xBF, 0xFF,
    0xFF, 0x00, 0x00, 0xBF, 0x77, 0xF3, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
    0x91, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x7E, 0xFF, 0xFE,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xC1, 0x00, 0xFE, 0xFF, 0xFF, 0xFF,
    0xF8, 0xFF, 0x00, 0x00, 0x00, 0x8F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
    0x45, 0xEC, 0x42, 0xFA, 0x08, 0xB7, 0x07, 0x5D, 0x01, 0xF5, 0xC0, 0xFF, 0x08, 0xFC, 0x00, 0xE5,
    0x0B, 0xF8, 0xC2, 0xCA, 0xF4, 0xF9, 0x0D, 0x7F, 0x44, 0x6D, 0x19, 0xFE, 0x46, 0x97, 0x33, 0x5E,
    0x08, 0xFF, 0xD1, 0xFF, 0xC6, 0x8B, 0x24, 0x74, 0x12, 0xFC, 0x00, 0x9F, 0x94, 0xB7, 0x06, 0xD5,
    0x40, 0x7A, 0x20, 0x9E, 0x04, 0x5F, 0x41, 0x2F, 0x3D, 0x77, 0x36, 0x75, 0x81, 0x8A, 0x70, 0x3A,
    0x98, 0xD1, 0x71, 0x02, 0x4D, 0x01, 0xC1, 0xFF, 0x0D, 0x00, 0xD3, 0x05, 0xF9, 0x00, 0x0B, 0x00
};


uint8_t *io_mem;

/* Gameboy colour has 8 internal RAM banks, bank 0 is from 0xC000 - 0xCFFF and is
 * fixed in both color gameboy and original gameboy. Banks 1-7 are switchable in 0xD000 - 0xDFFF in 
 * Colour gameboy but is fixed to bank 1 on the original gameboy */
static uint8_t cgb_ram_bank = 1;

static uint8_t cgb_ram_banks[6][0x1000];

/* The Gameboy color has 2 VRAM banks, stores
 * either 1 for VRAM bank 1 or 0 for VRAM bank 1
 * VRAM is located at memory 0x8000 - 0x97FF */
static int cgb_vram_bank = 0;

/* Holds secondary VRAM for cgb */
static uint8_t vram_bank_1[0x2000]; 

/* 64 Bytes of background palette memory (Gameboy Color only)
 * Holds 8 different background palettes, each with 4 colors.
 * Each color is represented by 2 bytes, */
static uint8_t bg_palette_mem[0x40] = {
     0xFF, 0x7F, 0xBF, 0x03, 0x1F, 0x00, 0x00, 0x00,
     0xFF, 0x7F, 0x80, 0x69, 0x1F, 0x00, 0x00, 0x00,
     0xFF, 0x7F, 0xF7, 0x63, 0x1F, 0x00, 0x00, 0x00,
     0xFF, 0x7F, 0xBF, 0x03, 0x80, 0x69, 0x00, 0x00,
     0xFF, 0x7F, 0x94, 0x7E, 0x80, 0x69, 0x00, 0x00,
     0xFF, 0x7F, 0xF7, 0x63, 0x80, 0x69, 0x00, 0x00,
     0xC0, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0xC0, 0x00};
bool bg_palette_dirty = true;
  
    
/* 64 Bytes of background palette memory (Gameboy Color only)
 * Holds 8 difference background palettes, each with 3 colors. 
 * (color 0 is always transparent)
 * Each color is represented by 2 bytes */
static uint8_t sprite_palette_mem[0x40];
bool sprite_palette_dirty = true;


/*  Gameboy bootstrap ROM for startup.
 *  Modified from the original so that the CPU doesn't
 *  hang if the ROM checksum checksum is incorrect */
static uint8_t const dmg_boot_rom[0x100] = {
  0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb,
  0x21, 0x26, 0xff, 0x0e, 0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3,
  0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0, 0x47, 0x11, 0x04, 0x01,
  0x21, 0x10, 0x80, 0x1a, 0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b,
  0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22,
  0x23, 0x05, 0x20, 0xf9, 0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99,
  0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18,
  0xf3, 0x67, 0x3e, 0x64, 0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04,
  0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20,
  0xf7, 0x1d, 0x20, 0xf2, 0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62,
  0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06, 0x7b, 0xe2, 0x0c, 0x3e,
  0x87, 0xe2, 0xf0, 0x42, 0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20,
  0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17,
  0xc1, 0xcb, 0x11, 0x17, 0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 
  0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83,
  0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e,
  0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63,
  0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e,
  0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c, 0x21, 0x04, 0x01, 0x11,
  0xa8, 0x00, 0x1a, 0x13, 0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20,
  0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xfb, 0x86, 0x00, 0x00,
  0x3e, 0x01, 0xe0, 0x50
};

/*  Gameboy Color bootstrap ROM 
 *  this actually consists of 2 roms, 
 *  The first 256 bytes are mapped to locations 0x00 - 0xFF
 *  and the last 1792 bytes are mapped to location 0x200 - 0x8FF */
static uint8_t cgb_boot_rom[] = {
  0x31, 0xfe, 0xff, 0x3e, 0x02, 0xc3, 0x7c, 0x00, 0xd3, 0x00, 0x98, 0xa0,
  0x12, 0xd3, 0x00, 0x80, 0x00, 0x40, 0x1e, 0x53, 0xd0, 0x00, 0x1f, 0x42,
  0x1c, 0x00, 0x14, 0x2a, 0x4d, 0x19, 0x8c, 0x7e, 0x00, 0x7c, 0x31, 0x6e,
  0x4a, 0x45, 0x52, 0x4a, 0x00, 0x00, 0xff, 0x53, 0x1f, 0x7c, 0xff, 0x03,
  0x1f, 0x00, 0xff, 0x1f, 0xa7, 0x00, 0xef, 0x1b, 0x1f, 0x00, 0xef, 0x1b,
  0x00, 0x7c, 0x00, 0x00, 0xff, 0x03, 0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d,
  0x00, 0x0b, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08,
  0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e, 0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd,
  0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc,
  0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5,
  0x42, 0x3c, 0x58, 0x43, 0xe0, 0x70, 0x3e, 0xfc, 0xe0, 0x47, 0xcd, 0x75,
  0x02, 0xcd, 0x00, 0x02, 0x26, 0xd0, 0xcd, 0x03, 0x02, 0x21, 0x00, 0xfe,
  0x0e, 0xa0, 0xaf, 0x22, 0x0d, 0x20, 0xfc, 0x11, 0x04, 0x01, 0x21, 0x10,
  0x80, 0x4c, 0x1a, 0xe2, 0x0c, 0xcd, 0xc6, 0x03, 0xcd, 0xc7, 0x03, 0x13,
  0x7b, 0xfe, 0x34, 0x20, 0xf1, 0x11, 0x72, 0x00, 0x06, 0x08, 0x1a, 0x13,
  0x22, 0x23, 0x05, 0x20, 0xf9, 0xcd, 0xf0, 0x03, 0x3e, 0x01, 0xe0, 0x4f,
  0x3e, 0x91, 0xe0, 0x40, 0x21, 0xb2, 0x98, 0x06, 0x4e, 0x0e, 0x44, 0xcd,
  0x91, 0x02, 0xaf, 0xe0, 0x4f, 0x0e, 0x80, 0x21, 0x42, 0x00, 0x06, 0x18,
  0xf2, 0x0c, 0xbe, 0x20, 0xfe, 0x23, 0x05, 0x20, 0xf7, 0x21, 0x34, 0x01,
  0x06, 0x19, 0x78, 0x86, 0x2c, 0x05, 0x20, 0xfb, 0x86, 0x20, 0xfe, 0xcd,
  0x1c, 0x03, 0x18, 0x02, 0x00, 0x00, 0xcd, 0xd0, 0x05, 0xaf, 0xe0, 0x70,
  0x3e, 0x11, 0xe0, 0x50, 0x21, 0x00, 0x80, 0xaf,
  0x22, 0xcb, 0x6c, 0x28, 0xfb, 0xc9, 0x2a, 0x12, 0x13, 0x0d, 0x20, 0xfa,
  0xc9, 0xe5, 0x21, 0x0f, 0xff, 0xcb, 0x86, 0xcb, 0x46, 0x28, 0xfc, 0xe1,
  0xc9, 0x11, 0x00, 0xff, 0x21, 0x03, 0xd0, 0x0e, 0x0f, 0x3e, 0x30, 0x12,
  0x3e, 0x20, 0x12, 0x1a, 0x2f, 0xa1, 0xcb, 0x37, 0x47, 0x3e, 0x10, 0x12,
  0x1a, 0x2f, 0xa1, 0xb0, 0x4f, 0x7e, 0xa9, 0xe6, 0xf0, 0x47, 0x2a, 0xa9,
  0xa1, 0xb0, 0x32, 0x47, 0x79, 0x77, 0x3e, 0x30, 0x12, 0xc9, 0x3e, 0x80,
  0xe0, 0x68, 0xe0, 0x6a, 0x0e, 0x6b, 0x2a, 0xe2, 0x05, 0x20, 0xfb, 0x4a,
  0x09, 0x43, 0x0e, 0x69, 0x2a, 0xe2, 0x05, 0x20, 0xfb, 0xc9, 0xc5, 0xd5,
  0xe5, 0x21, 0x00, 0xd8, 0x06, 0x01, 0x16, 0x3f, 0x1e, 0x40, 0xcd, 0x4a,
  0x02, 0xe1, 0xd1, 0xc1, 0xc9, 0x3e, 0x80, 0xe0, 0x26, 0xe0, 0x11, 0x3e,
  0xf3, 0xe0, 0x12, 0xe0, 0x25, 0x3e, 0x77, 0xe0, 0x24, 0x21, 0x30, 0xff,
  0xaf, 0x0e, 0x10, 0x22, 0x2f, 0x0d, 0x20, 0xfb, 0xc9, 0xcd, 0x11, 0x02,
  0xcd, 0x62, 0x02, 0x79, 0xfe, 0x38, 0x20, 0x14, 0xe5, 0xaf, 0xe0, 0x4f,
  0x21, 0xa7, 0x99, 0x3e, 0x38, 0x22, 0x3c, 0xfe, 0x3f, 0x20, 0xfa, 0x3e,
  0x01, 0xe0, 0x4f, 0xe1, 0xc5, 0xe5, 0x21, 0x43, 0x01, 0xcb, 0x7e, 0xcc,
  0x89, 0x05, 0xe1, 0xc1, 0xcd, 0x11, 0x02, 0x79, 0xd6, 0x30, 0xd2, 0x06,
  0x03, 0x79, 0xfe, 0x01, 0xca, 0x06, 0x03, 0x7d, 0xfe, 0xd1, 0x28, 0x21,
  0xc5, 0x06, 0x03, 0x0e, 0x01, 0x16, 0x03, 0x7e, 0xe6, 0xf8, 0xb1, 0x22,
  0x15, 0x20, 0xf8, 0x0c, 0x79, 0xfe, 0x06, 0x20, 0xf0, 0x11, 0x11, 0x00,
  0x19, 0x05, 0x20, 0xe7, 0x11, 0xa1, 0xff, 0x19, 0xc1, 0x04, 0x78, 0x1e,
  0x83, 0xfe, 0x62, 0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x07, 0x7b,
  0xe0, 0x13, 0x3e, 0x87, 0xe0, 0x14, 0xfa, 0x02, 0xd0, 0xfe, 0x00, 0x28,
  0x0a, 0x3d, 0xea, 0x02, 0xd0, 0x79, 0xfe, 0x01, 0xca, 0x91, 0x02, 0x0d,
  0xc2, 0x91, 0x02, 0xc9, 0x0e, 0x26, 0xcd, 0x4a, 0x03, 0xcd, 0x11, 0x02,
  0xcd, 0x62, 0x02, 0x0d, 0x20, 0xf4, 0xcd, 0x11, 0x02, 0x3e, 0x01, 0xe0,
  0x4f, 0xcd, 0x3e, 0x03, 0xcd, 0x41, 0x03, 0xaf, 0xe0, 0x4f, 0xcd, 0x3e,
  0x03, 0xc9, 0x21, 0x08, 0x00, 0x11, 0x51, 0xff, 0x0e, 0x05, 0xcd, 0x0a,
  0x02, 0xc9, 0xc5, 0xd5, 0xe5, 0x21, 0x40, 0xd8, 0x0e, 0x20, 0x7e, 0xe6,
  0x1f, 0xfe, 0x1f, 0x28, 0x01, 0x3c, 0x57, 0x2a, 0x07, 0x07, 0x07, 0xe6,
  0x07, 0x47, 0x3a, 0x07, 0x07, 0x07, 0xe6, 0x18, 0xb0, 0xfe, 0x1f, 0x28,
  0x01, 0x3c, 0x0f, 0x0f, 0x0f, 0x47, 0xe6, 0xe0, 0xb2, 0x22, 0x78, 0xe6,
  0x03, 0x5f, 0x7e, 0x0f, 0x0f, 0xe6, 0x1f, 0xfe, 0x1f, 0x28, 0x01, 0x3c,
  0x07, 0x07, 0xb3, 0x22, 0x0d, 0x20, 0xc7, 0xe1, 0xd1, 0xc1, 0xc9, 0x0e,
  0x00, 0x1a, 0xe6, 0xf0, 0xcb, 0x49, 0x28, 0x02, 0xcb, 0x37, 0x47, 0x23,
  0x7e, 0xb0, 0x22, 0x1a, 0xe6, 0x0f, 0xcb, 0x49, 0x20, 0x02, 0xcb, 0x37,
  0x47, 0x23, 0x7e, 0xb0, 0x22, 0x13, 0xcb, 0x41, 0x28, 0x0d, 0xd5, 0x11,
  0xf8, 0xff, 0xcb, 0x49, 0x28, 0x03, 0x11, 0x08, 0x00, 0x19, 0xd1, 0x0c,
  0x79, 0xfe, 0x18, 0x20, 0xcc, 0xc9, 0x47, 0xd5, 0x16, 0x04, 0x58, 0xcb,
  0x10, 0x17, 0xcb, 0x13, 0x17, 0x15, 0x20, 0xf6, 0xd1, 0x22, 0x23, 0x22,
  0x23, 0xc9, 0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99, 0x0e, 0x0c,
  0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18, 0xf3, 0xc9,
  0x3e, 0x01, 0xe0, 0x4f, 0xcd, 0x00, 0x02, 0x11, 0x07, 0x06, 0x21, 0x80,
  0x80, 0x0e, 0xc0, 0x1a, 0x22, 0x23, 0x22, 0x23, 0x13, 0x0d, 0x20, 0xf7,
  0x11, 0x04, 0x01, 0xcd, 0x8f, 0x03, 0x01, 0xa8, 0xff, 0x09, 0xcd, 0x8f,
  0x03, 0x01, 0xf8, 0xff, 0x09, 0x11, 0x72, 0x00, 0x0e, 0x08, 0x23, 0x1a,
  0x22, 0x13, 0x0d, 0x20, 0xf9, 0x21, 0xc2, 0x98, 0x06, 0x08, 0x3e, 0x08,
  0x0e, 0x10, 0x22, 0x0d, 0x20, 0xfc, 0x11, 0x10, 0x00, 0x19, 0x05, 0x20,
  0xf3, 0xaf, 0xe0, 0x4f, 0x21, 0xc2, 0x98, 0x3e, 0x08, 0x22, 0x3c, 0xfe,
  0x18, 0x20, 0x02, 0x2e, 0xe2, 0xfe, 0x28, 0x20, 0x03, 0x21, 0x02, 0x99,
  0xfe, 0x38, 0x20, 0xed, 0x21, 0xd8, 0x08, 0x11, 0x40, 0xd8, 0x06, 0x08,
  0x3e, 0xff, 0x12, 0x13, 0x12, 0x13, 0x0e, 0x02, 0xcd, 0x0a, 0x02, 0x3e,
  0x00, 0x12, 0x13, 0x12, 0x13, 0x13, 0x13, 0x05, 0x20, 0xea, 0xcd, 0x62,
  0x02, 0x21, 0x4b, 0x01, 0x7e, 0xfe, 0x33, 0x20, 0x0b, 0x2e, 0x44, 0x1e,
  0x30, 0x2a, 0xbb, 0x20, 0x49, 0x1c, 0x18, 0x04, 0x2e, 0x4b, 0x1e, 0x01,
  0x2a, 0xbb, 0x20, 0x3e, 0x2e, 0x34, 0x01, 0x10, 0x00, 0x2a, 0x80, 0x47,
  0x0d, 0x20, 0xfa, 0xea, 0x00, 0xd0, 0x21, 0xc7, 0x06, 0x0e, 0x00, 0x2a,
  0xb8, 0x28, 0x08, 0x0c, 0x79, 0xfe, 0x4f, 0x20, 0xf6, 0x18, 0x1f, 0x79,
  0xd6, 0x41, 0x38, 0x1c, 0x21, 0x16, 0x07, 0x16, 0x00, 0x5f, 0x19, 0xfa,
  0x37, 0x01, 0x57, 0x7e, 0xba, 0x28, 0x0d, 0x11, 0x0e, 0x00, 0x19, 0x79,
  0x83, 0x4f, 0xd6, 0x5e, 0x38, 0xed, 0x0e, 0x00, 0x21, 0x33, 0x07, 0x06,
  0x00, 0x09, 0x7e, 0xe6, 0x1f, 0xea, 0x08, 0xd0, 0x7e, 0xe6, 0xe0, 0x07,
  0x07, 0x07, 0xea, 0x0b, 0xd0, 0xcd, 0xe9, 0x04, 0xc9, 0x11, 0x91, 0x07, 0x21, 0x00, 0xd9, 0xfa, 0x0b, 0xd0, 0x47, 0x0e, 0x1e, 0xcb, 0x40, 0x20, 0x02, 0x13, 0x13, 0x1a, 0x22, 0x20, 0x02, 0x1b, 0x1b, 0xcb, 0x48, 0x20, 0x02, 0x13, 0x13, 0x1a, 0x22, 0x13, 0x13, 0x20, 0x02, 0x1b, 0x1b, 0xcb,
  0x50, 0x28, 0x05, 0x1b, 0x2b, 0x1a, 0x22, 0x13, 0x1a, 0x22, 0x13, 0x0d,
  0x20, 0xd7, 0x21, 0x00, 0xd9, 0x11, 0x00, 0xda, 0xcd, 0x64, 0x05, 0xc9,
  0x21, 0x12, 0x00, 0xfa, 0x05, 0xd0, 0x07, 0x07, 0x06, 0x00, 0x4f, 0x09,
  0x11, 0x40, 0xd8, 0x06, 0x08, 0xe5, 0x0e, 0x02, 0xcd, 0x0a, 0x02, 0x13,
  0x13, 0x13, 0x13, 0x13, 0x13, 0xe1, 0x05, 0x20, 0xf0, 0x11, 0x42, 0xd8,
  0x0e, 0x02, 0xcd, 0x0a, 0x02, 0x11, 0x4a, 0xd8, 0x0e, 0x02, 0xcd, 0x0a,
  0x02, 0x2b, 0x2b, 0x11, 0x44, 0xd8, 0x0e, 0x02, 0xcd, 0x0a, 0x02, 0xc9,
  0x0e, 0x60, 0x2a, 0xe5, 0xc5, 0x21, 0xe8, 0x07, 0x06, 0x00, 0x4f, 0x09,
  0x0e, 0x08, 0xcd, 0x0a, 0x02, 0xc1, 0xe1, 0x0d, 0x20, 0xec, 0xc9, 0xfa,
  0x08, 0xd0, 0x11, 0x18, 0x00, 0x3c, 0x3d, 0x28, 0x03, 0x19, 0x20, 0xfa,
  0xc9, 0xcd, 0x1d, 0x02, 0x78, 0xe6, 0xff, 0x28, 0x0f, 0x21, 0xe4, 0x08,
  0x06, 0x00, 0x2a, 0xb9, 0x28, 0x08, 0x04, 0x78, 0xfe, 0x0c, 0x20, 0xf6,
  0x18, 0x2d, 0x78, 0xea, 0x05, 0xd0, 0x3e, 0x1e, 0xea, 0x02, 0xd0, 0x11,
  0x0b, 0x00, 0x19, 0x56, 0x7a, 0xe6, 0x1f, 0x5f, 0x21, 0x08, 0xd0, 0x3a,
  0x22, 0x7b, 0x77, 0x7a, 0xe6, 0xe0, 0x07, 0x07, 0x07, 0x5f, 0x21, 0x0b,
  0xd0, 0x3a, 0x22, 0x7b, 0x77, 0xcd, 0xe9, 0x04, 0xcd, 0x28, 0x05, 0xc9,
  0xcd, 0x11, 0x02, 0xfa, 0x43, 0x01, 0xcb, 0x7f, 0x28, 0x04, 0xe0, 0x4c,
  0x18, 0x28, 0x3e, 0x04, 0xe0, 0x4c, 0x3e, 0x01, 0xe0, 0x6c, 0x21, 0x00,
  0xda, 0xcd, 0x7b, 0x05, 0x06, 0x10, 0x16, 0x00, 0x1e, 0x08, 0xcd, 0x4a,
  0x02, 0x21, 0x7a, 0x00, 0xfa, 0x00, 0xd0, 0x47, 0x0e, 0x02, 0x2a, 0xb8,
  0xcc, 0xda, 0x03, 0x0d, 0x20, 0xf8, 0xc9, 0x01, 0x0f, 0x3f, 0x7e, 0xff,
  0xff, 0xc0, 0x00, 0xc0, 0xf0, 0xf1, 0x03, 0x7c, 0xfc, 0xfe, 0xfe, 0x03,
  0x07, 0x07, 0x0f, 0xe0, 0xe0, 0xf0, 0xf0, 0x1e, 0x3e, 0x7e, 0xfe, 0x0f,
  0x0f, 0x1f, 0x1f, 0xff, 0xff, 0x00, 0x00, 0x01, 0x01, 0x01, 0x03, 0xff,
  0xff, 0xe1, 0xe0, 0xc0, 0xf0, 0xf9, 0xfb, 0x1f, 0x7f, 0xf8, 0xe0, 0xf3,
  0xfd, 0x3e, 0x1e, 0xe0, 0xf0, 0xf9, 0x7f, 0x3e, 0x7c, 0xf8, 0xe0, 0xf8,
  0xf0, 0xf0, 0xf8, 0x00, 0x00, 0x7f, 0x7f, 0x07, 0x0f, 0x9f, 0xbf, 0x9e,
  0x1f, 0xff, 0xff, 0x0f, 0x1e, 0x3e, 0x3c, 0xf1, 0xfb, 0x7f, 0x7f, 0xfe, 0xde, 0xdf, 0x9f, 0x1f, 0x3f, 0x3e, 0x3c, 0xf8, 0xf8, 0x00, 0x00, 0x03,
  0x03, 0x07, 0x07, 0xff, 0xff, 0xc1, 0xc0, 0xf3, 0xe7, 0xf7, 0xf3, 0xc0,
  0xc0, 0xc0, 0xc0, 0x1f, 0x1f, 0x1e, 0x3e, 0x3f, 0x1f, 0x3e, 0x3e, 0x80,
  0x00, 0x00, 0x00, 0x7c, 0x1f, 0x07, 0x00, 0x0f, 0xff, 0xfe, 0x00, 0x7c,
  0xf8, 0xf0, 0x00, 0x1f, 0x0f, 0x0f, 0x00, 0x7c, 0xf8, 0xf8, 0x00, 0x3f,
  0x3e, 0x1c, 0x00, 0x0f, 0x0f, 0x0f, 0x00, 0x7c, 0xff, 0xff, 0x00, 0x00,
  0xf8, 0xf8, 0x00, 0x07, 0x0f, 0x0f, 0x00, 0x81, 0xff, 0xff, 0x00, 0xf3,
  0xe1, 0x80, 0x00, 0xe0, 0xff, 0x7f, 0x00, 0xfc, 0xf0, 0xc0, 0x00, 0x3e,
  0x7c, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x16, 0x36, 0xd1,
  0xdb, 0xf2, 0x3c, 0x8c, 0x92, 0x3d, 0x5c, 0x58, 0xc9, 0x3e, 0x70, 0x1d,
  0x59, 0x69, 0x19, 0x35, 0xa8, 0x14, 0xaa, 0x75, 0x95, 0x99, 0x34, 0x6f,
  0x15, 0xff, 0x97, 0x4b, 0x90, 0x17, 0x10, 0x39, 0xf7, 0xf6, 0xa2, 0x49,
  0x4e, 0x43, 0x68, 0xe0, 0x8b, 0xf0, 0xce, 0x0c, 0x29, 0xe8, 0xb7, 0x86,
  0x9a, 0x52, 0x01, 0x9d, 0x71, 0x9c, 0xbd, 0x5d, 0x6d, 0x67, 0x3f, 0x6b,
  0xb3, 0x46, 0x28, 0xa5, 0xc6, 0xd3, 0x27, 0x61, 0x18, 0x66, 0x6a, 0xbf,
  0x0d, 0xf4, 0x42, 0x45, 0x46, 0x41, 0x41, 0x52, 0x42, 0x45, 0x4b, 0x45,
  0x4b, 0x20, 0x52, 0x2d, 0x55, 0x52, 0x41, 0x52, 0x20, 0x49, 0x4e, 0x41,
  0x49, 0x4c, 0x49, 0x43, 0x45, 0x20, 0x52, 0x7c, 0x08, 0x12, 0xa3, 0xa2,
  0x07, 0x87, 0x4b, 0x20, 0x12, 0x65, 0xa8, 0x16, 0xa9, 0x86, 0xb1, 0x68,
  0xa0, 0x87, 0x66, 0x12, 0xa1, 0x30, 0x3c, 0x12, 0x85, 0x12, 0x64, 0x1b,
  0x07, 0x06, 0x6f, 0x6e, 0x6e, 0xae, 0xaf, 0x6f, 0xb2, 0xaf, 0xb2, 0xa8,
  0xab, 0x6f, 0xaf, 0x86, 0xae, 0xa2, 0xa2, 0x12, 0xaf, 0x13, 0x12, 0xa1,
  0x6e, 0xaf, 0xaf, 0xad, 0x06, 0x4c, 0x6e, 0xaf, 0xaf, 0x12, 0x7c, 0xac,
  0xa8, 0x6a, 0x6e, 0x13, 0xa0, 0x2d, 0xa8, 0x2b, 0xac, 0x64, 0xac, 0x6d,
  0x87, 0xbc, 0x60, 0xb4, 0x13, 0x72, 0x7c, 0xb5, 0xae, 0xae, 0x7c, 0x7c,
  0x65, 0xa2, 0x6c, 0x64, 0x85, 0x80, 0xb0, 0x40, 0x88, 0x20, 0x68, 0xde,
  0x00, 0x70, 0xde, 0x20, 0x78, 0x20, 0x20, 0x38, 0x20, 0xb0, 0x90, 0x20,
  0xb0, 0xa0, 0xe0, 0xb0, 0xc0, 0x98, 0xb6, 0x48, 0x80, 0xe0, 0x50, 0x1e,
  0x1e, 0x58, 0x20, 0xb8, 0xe0, 0x88, 0xb0, 0x10, 0x20, 0x00, 0x10, 0x20,
  0xe0, 0x18, 0xe0, 0x18, 0x00, 0x18, 0xe0, 0x20, 0xa8, 0xe0, 0x20, 0x18,
  0xe0, 0x00, 0x20, 0x18, 0xd8, 0xc8, 0x18, 0xe0, 0x00, 0xe0, 0x40, 0x28,
  0x28, 0x28, 0x18, 0xe0, 0x60, 0x20, 0x18, 0xe0, 0x00, 0x00, 0x08, 0xe0,
  0x18, 0x30, 0xd0, 0xd0, 0xd0, 0x20, 0xe0, 0xe8, 0xff, 0x7f, 0xbf, 0x32,
  0xd0, 0x00, 0x00, 0x00, 0x9f, 0x63, 0x79, 0x42, 0xb0, 0x15, 0xcb, 0x04,
  0xff, 0x7f, 0x31, 0x6e, 0x4a, 0x45, 0x00, 0x00, 0xff, 0x7f, 0xef, 0x1b,
  0x00, 0x02, 0x00, 0x00, 0xff, 0x7f, 0x1f, 0x42, 0xf2, 0x1c, 0x00, 0x00,
  0xff, 0x7f, 0x94, 0x52, 0x4a, 0x29, 0x00, 0x00, 0xff, 0x7f, 0xff, 0x03,
  0x2f, 0x01, 0x00, 0x00, 0xff, 0x7f, 0xef, 0x03, 0xd6, 0x01, 0x00, 0x00,
  0xff, 0x7f, 0xb5, 0x42, 0xc8, 0x3d, 0x00, 0x00, 0x74, 0x7e, 0xff, 0x03,
  0x80, 0x01, 0x00, 0x00, 0xff, 0x67, 0xac, 0x77, 0x13, 0x1a, 0x6b, 0x2d,
  0xd6, 0x7e, 0xff, 0x4b, 0x75, 0x21, 0x00, 0x00, 0xff, 0x53, 0x5f, 0x4a,
  0x52, 0x7e, 0x00, 0x00, 0xff, 0x4f, 0xd2, 0x7e, 0x4c, 0x3a, 0xe0, 0x1c,
  0xed, 0x03, 0xff, 0x7f, 0x5f, 0x25, 0x00, 0x00, 0x6a, 0x03, 0x1f, 0x02,
  0xff, 0x03, 0xff, 0x7f, 0xff, 0x7f, 0xdf, 0x01, 0x12, 0x01, 0x00, 0x00,
  0x1f, 0x23, 0x5f, 0x03, 0xf2, 0x00, 0x09, 0x00, 0xff, 0x7f, 0xea, 0x03,
  0x1f, 0x01, 0x00, 0x00, 0x9f, 0x29, 0x1a, 0x00, 0x0c, 0x00, 0x00, 0x00,
  0xff, 0x7f, 0x7f, 0x02, 0x1f, 0x00, 0x00, 0x00, 0xff, 0x7f, 0xe0, 0x03,
  0x06, 0x02, 0x20, 0x01, 0xff, 0x7f, 0xeb, 0x7e, 0x1f, 0x00, 0x00, 0x7c,
  0xff, 0x7f, 0xff, 0x3f, 0x00, 0x7e, 0x1f, 0x00, 0xff, 0x7f, 0xff, 0x03,
  0x1f, 0x00, 0x00, 0x00, 0xff, 0x03, 0x1f, 0x00, 0x0c, 0x00, 0x00, 0x00,
  0xff, 0x7f, 0x3f, 0x03, 0x93, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42,
  0x7f, 0x03, 0xff, 0x7f, 0xff, 0x7f, 0x8c, 0x7e, 0x00, 0x7c, 0x00, 0x00,
  0xff, 0x7f, 0xef, 0x1b, 0x80, 0x61, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x7c,
  0xe0, 0x03, 0x1f, 0x7c, 0x1f, 0x00, 0xff, 0x03, 0x40, 0x41, 0x42, 0x20,
  0x21, 0x22, 0x80, 0x81, 0x82, 0x10, 0x11, 0x12, 0x12, 0xb0, 0x79, 0xb8,
  0xad, 0x16, 0x17, 0x07, 0xba, 0x05, 0x7c, 0x13, 0x00, 0x00, 0x00, 0x00
};



// Header in MMM01 Roms are placed at the end
// of the ROM instead of at the beginning
void check_mmm01_format(unsigned char *file_data, size_t size) {
    if (size < 0x8000) {
        return;
    }
    
    unsigned char const *header_data = file_data + (size - 0x8000);
    unsigned char rom_code = header_data[0x147];
    
    // If Header is at the end place it at the front
    if (header_data[0x104] == 0xCE && header_data[0x105] == 0xED &&
        header_data[0x106] == 0x66 && header_data[0x107] == 0x66 &&
        header_data[0x108] == 0xCC && header_data[0x109] == 0x0D &&
        rom_code >= 0xB && rom_code <= 0xD) {
    
            unsigned char temp[0x8000];
            memcpy(temp, header_data, 0x8000);
            memmove(file_data + 0x8000, file_data, size - 0x8000);
            memcpy(file_data, temp, 0x8000); 
    
    }
}

int load_rom(char const *filename, uint8_t header[0x50], int const dmg_mode) {

    oam_mem_ptr = oam_mem;
 
    cgb = !dmg_mode;
    io_mem  = cgb ? io_mem_cgb : io_mem_dmg;

    uint8_t rom_bank_info = header[CARTRIDGE_ROM_SIZE - 0x100];
    int rom_banks = 0;
    if (rom_bank_info <= 8) {
        rom_banks = 2 << rom_bank_info; 
    } else {
        switch (rom_bank_info) {
            case 0x52: rom_banks = 72; break;
            case 0x53: rom_banks = 80; break;
            case 0x54: rom_banks = 96; break;
            default: {
                log_message(LOG_ERROR, "Unsupported value for ROM size in header: 0x%X\n"
                            , rom_bank_info);
                return 0;
            }
        }
    }

    uint8_t ram_bank_info = header[CARTRIDGE_RAM_SIZE - 0x100];
    int ram_banks;
    switch (ram_bank_info) {
        case 0: ram_banks = 0;  break;
        case 1: ram_banks = 1;  break; 
        case 2: ram_banks = 1;  break;
        case 3: ram_banks = 4;  break;
        case 4: ram_banks = 16; break;
        case 5: ram_banks = 8;  break;
        default: {
            log_message(LOG_ERROR, "Unsupported value for RAM size in header: 0x%X\n"
                        , ram_bank_info);
            return 0;
        }
    }

    // Setup the memory bank controller 
    if(!setup_MBC(header[CARTRIDGE_TYPE - 0x100], ram_banks, rom_banks, filename)) {
        return 0;
    }

    size_t rom_size = rom_banks * ROM_BANK_SIZE;
    size_t read_size;
    if (!(read_size = load_rom_from_file(filename, ROM_banks))) {
        log_message(LOG_ERROR, "failed to load ROM\n");
        return 0;
    }
    
    check_mmm01_format(ROM_banks, read_size);

    // Data read in doesn't match header information
    if (read_size != rom_size) {

        log_message(LOG_ERROR, "Error: Cartridge header info on its size (%lu bytes) \
            doesn't match file size (%lu bytes)\n",rom_size, read_size);
        return 0;
    }

   
    is_booting = 1; 
    return 1;
} 

uint8_t *get_bg_palette() {
    return bg_palette_mem;
}

uint8_t *get_sprite_palette() {
    return sprite_palette_mem;
}

/* Write to OAM given OAM address 0x0 - 0xA0
 * Does nothing if address > 0xA0 */
static void oam_set_mem(uint8_t addr, uint8_t val) {
    
    // Check not unusable RAM (i.e. not 0xFEA0 - 0xFEFF)
    if (addr < 0xA0) {
        oam_mem[addr] = val;
        /* If Object X position is written to, reorganise
         * sprite priorities for rendering */
        if((addr - 1) % 4 == 0) {
            /* In CGB mode priorities are purely sprite numbers not x-positions
               so just write 0 to the x position table so they are organised this way */
            if (cgb && (is_booting || cgb_features)) { 
                val = 0;
            }
            update_sprite_prios(addr/4 ,val);
        }
    }
}

/* Transfer 160 bytes to sprite memory starting from
 * address XX00 */
static void dma_transfer(uint8_t val) {        
    uint16_t source_addr = val << 8;
    for (int i = 0; i < 0xA0; i++) {
        oam_mem[i] = get_mem(source_addr + i);
    }
}


/* Keypad is written to, update register with state */
static void joypad_write(uint8_t joypad_state) {
    
    joypad_state |= 0xF; // unset all keys

    // Check directional keys
    if (joypad_state & BIT_5) {
        joypad_state &= ~(down_pressed() << 3);
        joypad_state &= ~(up_pressed() << 2);
        joypad_state &= ~(left_pressed() << 1);
        joypad_state &= ~(right_pressed());
    } 
    // Check other 4 keys
    else if (joypad_state & BIT_4) {
        joypad_state &= ~(start_pressed() << 3);
        joypad_state &= ~(select_pressed() << 2);
        joypad_state &= ~(b_pressed() << 1);
        joypad_state &= ~(a_pressed());
    }
    
    /* Raise joypad interrupt if a key
     * was pressed */
    if ((joypad_state & 0xF) != 0xF) {
        raise_interrupt(JOYPAD_INT);
    }

    io_mem[P1_REG] = joypad_state;
}

/* Write to IO memory given address 0 - 0xFF */
void io_write_mem(uint8_t addr, uint8_t val) {

    if (addr >= 0x10 && addr <= 0x3F) {
        io_mem[addr] = val;
        write_apu(addr + 0xFF00, val); 
        return;
    }

    switch (addr) {
        /* Check Joypad values */
        case P1_REG  : io_mem[addr] = val; joypad_write(val); break;
        /*  Attempting to set DIV reg resets it to 0 */
        case DIV_REG  : io_mem[addr] = 0 ;break; 
        
        case LCDC_REG: {
            uint8_t current_lcdc = io_mem[addr];
            uint8_t new_lcdc = val;
            io_mem[addr] = val;
            if (!(current_lcdc & BIT_5) && (new_lcdc & BIT_5)) {
                reset_window_line();
            }
            if (new_lcdc & BIT_7) {
                enable_screen();
            } else {
                disable_screen();
            }
            break;
        }

        case STAT_REG: {
            uint8_t current_stat = io_mem[addr] & 0x7;
            uint8_t new_stat = (val & 0x78) | (current_stat & 0x7);
            io_mem[addr] = new_stat;
            uint8_t lcdc = io_mem[LCDC_REG];
            uint8_t lcd_interrupt_signal = get_interrupt_signal();
            uint8_t mode = get_lcd_mode();
            lcd_interrupt_signal &= ((new_stat >> 3) & 0x0F);
            set_interrupt_signal(lcd_interrupt_signal);

            if (lcdc & BIT_7) {
                if ((new_stat & BIT_3) && (mode == 0)) {
                    if (lcd_interrupt_signal == 0) {
                        raise_interrupt(LCD_INT);
                    }
                    lcd_interrupt_signal |= BIT_0;
                }
                if ((new_stat & BIT_4) && (mode == 1)) {
                    if (lcd_interrupt_signal == 0) {
                        raise_interrupt(LCD_INT);
                    }
                    lcd_interrupt_signal |= BIT_1;
                }
                if ((new_stat & BIT_5) && (mode == 2)) {
                    if (lcd_interrupt_signal == 0) {
                        raise_interrupt(LCD_INT);
                    }
                }
                check_lcd_coincidence();
            }

            break;
        }

        case LY_REG : {
            uint8_t ly = io_mem[LY_REG];
            if ((ly & BIT_7) && !(val & BIT_7)) {
                disable_screen();
            }
            break;
        }

        case LYC_REG : {
            uint8_t current_lyc = io_mem[addr];
            if (current_lyc != val) {
                io_mem[addr] = val;
                uint8_t lcdc = io_mem[LCDC_REG];
                if (lcdc & BIT_7) {
                    check_lcd_coincidence();
                }
            }
            break;
        } 

        /*  Perform direct memory transfer  */
        case DMA_REG  : io_mem[addr] = val; dma_transfer(val); break;
        /*  Check if serial transfer starting*/
        case SC_REG : io_mem[addr] = val; start_transfer(&(io_mem[0x2]), &(io_mem[0x1])); break;

        /* Color Gameboy registers */
        case HDMA1_REG : if (cgb && (cgb_features || is_booting)) {
            /* Source address must be from either 0x0000 -> 0x7FFFF
             * or 0xA000 -> 0xDFFF */
            io_mem[addr] = val;
            if ((val > 0x7F && val < 0xA0)) {
                val = 0;
            }
                hdma_source = (val << 8) | (hdma_source & 0xF0);
            } 
            break;

        
        case HDMA2_REG : if (cgb && (cgb_features || is_booting)) {
            io_mem[addr] = val;
            val &= 0xF0;
            // Bits 3-0 in HDMA 2 unused
            hdma_source = (hdma_source & 0xFF00) | val;
        }
        break;

        case HDMA3_REG : if (cgb && (cgb_features || is_booting)) {
            io_mem[addr] = val;
            /*  Destination address must be from 0x8000 -> 0x9FFF */
            // Bits 7 - 5 in HDMA 3 unused
            val &= 0x1F;
            hdma_dest = (val << 8 ) | (hdma_dest & 0xF0);
            hdma_dest |= 0x8000;
        }

        break;

        case HDMA4_REG : if (cgb && (cgb_features || is_booting)) {
            io_mem[addr] = val;
            // Bits 3-0 in HDMA 4 unused
            val &= 0xF0;
            hdma_dest = (hdma_dest & 0x1F00) | val;
            hdma_dest |= 0x8000;
        }
        break;

        case HDMA5_REG : if (cgb && (cgb_features || is_booting)) {
            io_mem[addr] = val;
            check_cgb_dma(val); 
        }
        break;


        case VBANK_REG : if(cgb && (cgb_features || is_booting)) {
                            // Select VRAM bank 0 or 1
                            cgb_vram_bank = val & 0x1;
                            io_mem[addr] = val & 0x1;
                         //Forcibly set to 0 in DMG mode on A Color Gameboy
                         } else if (cgb) {
                            io_mem[addr] = 0;
                         }

                         break;
        
        case BGPI : 
                    io_mem[addr] = val;
                    if (cgb && (cgb_features || is_booting)) {
                        io_mem[BGPD] = bg_palette_mem[val & 0x3F];
                    } else {
                        io_mem[BGPD] = 0xC0;
                    }
                    break;

        case BGPD : if (cgb && (cgb_features || is_booting)) {
                        io_mem[addr] = val;
                        /* Write data to Gameboy background palette.
                         * Use the Background Palette Index to select the location
                         * to write the value to in Background Palette memory */
                        uint8_t bgpi = io_mem[BGPI];
                       // uint8_t address = bgpi & 0x3F
                       // uint8_t index = bgpi & 0x3F;
                        int old_palette_mem = bg_palette_mem[bgpi & 0x3F];
                        bg_palette_mem[bgpi & 0x3F] = val;
                        bg_palette_dirty |= (old_palette_mem != val);

                        /* Check if Auto Increment bit is set in Background Palette Index,
                           and increment the index if so. Index is between 0x0 and 0x3F */
                        if (bgpi & 0x80) {
                            uint8_t address = bgpi & 0x3F;
                            address++;
                            address &= 0x3F;
                            bgpi = (bgpi & 0x80) | address;
                            io_mem[BGPI] = bgpi;
                            io_mem[SPPD] = bg_palette_mem[bgpi & 0x3F];
                        } 

                    } else {
                        io_mem[addr] = 0xFF;
                    }
                    break;

        case SPPI : 
                    io_mem[addr] = val;
                    if (cgb && (cgb_features || is_booting)) {
                        io_mem[SPPD] = sprite_palette_mem[val & 0x3F];
                    } else {
                        io_mem[BGPD] = 0xC0;
                    }
                    break;
        
        case SPPD : if (cgb && (cgb_features || is_booting)) {
                        io_mem[addr] = val;
                        /* Write data to Gameboy sprite palette.
                         * Use the Sprite Palette Index to select the location
                         * to write the value to in Sprite Palette memory */
                        uint8_t sppi = io_mem[SPPI];
                        uint8_t old_val = sprite_palette_mem[sppi & 0x3F];
                        sprite_palette_mem[sppi & 0x3F] = val;
                        sprite_palette_dirty |= (old_val != val);
                        
                        /* Check if Auto Increment bit is set in Sprite Palette Index,
                           and increment the index if so. Index is between 0x0 and 0x3F */
                        if (sppi & 0x80) {
                            uint8_t address = sppi & 0x3F;
                            address++;
                            address &= 0x3F;
                            sppi = (sppi & 0x80) | address;
                            io_mem[SPPI] = sppi;
                            io_mem[SPPD] = sprite_palette_mem[sppi & 0x3F];
                        } 


                    } else {
                        io_mem[SPPD] = 0xFF;
                    }
                    break;

        case SRAM_BANK: if (cgb && (cgb_features || is_booting)) {
            val &= 0x7;
            io_mem[addr] = val;
            if (val == 0) {
                val = 1;
            }
            cgb_ram_bank = val;
            }
            break;

        // Can only set bit 0 to Prepare for a speed change
        case 0x4D: if (cgb && (cgb_features || is_booting)) {
            io_mem[addr] = ((io_mem[addr] & 0x80) | (val & 0x1));
        }
        break;

        // Undocumented Registers
        case 0x6C:
            io_mem[addr] = (cgb && (is_booting || cgb_features)) ? val & 0x1 : cgb ? 0xFF : val;
            break;

        case 0x74:
            io_mem[addr] = (cgb && !(is_booting || cgb_features)) ? 0xFF : val;
            break;

        case 0x75:
            // Bits 4-6 Readable/Writeable
            io_mem[addr] = cgb ? val & 0x60 : val;
            break;

        case 0x76:
            io_mem[addr] = cgb ? 0 : val;
            break;

        case 0x77:
            io_mem[addr] = cgb ? 0 : val;
            break;

        case BOOT_ROM_DISABLE: 
            is_booting = 0;
            break;

       default:
            io_mem[addr] = val;
            break;
    }
}


int interrupt_about_to_raise() {
    return io_mem[0xFF] & io_mem[0x0F] & 0x1F;
}

/* Directly inject a value into IO memory without performing
 * any checks or operations on the data. Should be used by
 * controllers that have direct access to modifying this memory
 * and not the CPU. */
void io_write_override(uint8_t addr, uint8_t val) {
   io_mem[addr] = val;
}


/*  Write an 8 bit value to the given 16 bit address */
void set_mem(uint16_t addr, uint8_t const val) {
  
    //Check if memory bank controller chip is being accessed 
    if (addr < 0x8000 || ((uint16_t)(addr - 0xA000) < 0x2000)) {
        write_MBC(addr, val); 
        return;
    }

    // Write to "ordinary" memory (0x8000 - 0xFDFF)
    if (addr < 0xFE00) {
       
        //  Check if mirrored memory being written to
        // 0xE000 - 0xFDFF, repeat to 0xC000 - 0xDDFF
        if ((uint16_t)(addr - ECHO_RAM_START) < 0x1DFF) { 
	    addr -= 0x2000;
        }

        // Check if writting to alternative VRAM with Gameboy Color
        if (cgb && cgb_vram_bank && addr >= 0x8000 && addr < 0xA000) {
            vram_bank_1[addr - 0x8000] = val;
            return;
        }

        if (cgb && cgb_ram_bank > 1 && addr >= 0xD000 && addr <= 0xDFFF) {
           cgb_ram_banks[cgb_ram_bank - 2][addr - 0xD000] = val;
           return;
        }

        mem[addr - 0x8000] = val;
        return;
    }
    

    // Write to Object Attribute Table (0xFE00 - 0xFEFF)
    if ((uint16_t)(addr - 0xFE00) < 0x100) {
        oam_set_mem(addr - 0xFE00, val);
        return;
    }
    /*  IO being written to */
    if (addr >= 0xFF00) {
        io_write_mem(addr - 0xFF00, val);
    }
}


uint8_t get_vram1(uint16_t addr) {
    return vram_bank_1[addr - 0x8000];
}

uint8_t get_vram(uint16_t addr, int bank) {
    if (bank) {
        return vram_bank_1[addr - 0x8000];
    } else {
        return mem[addr - 0x8000];
    }
}

uint8_t get_current_vram(uint16_t addr) {
    if (cgb && cgb_vram_bank) {
        return vram_bank_1[addr - 0x8000];
    } else {
        return mem[addr - 0x8000];
    }
}

uint8_t get_vram0(uint16_t addr) {
    return mem[addr - 0x8000];
}

// Read contents from given 16 bit memory address
uint8_t get_mem(uint16_t addr) {
   
    if (is_booting) {
        if (cgb) {
            if (addr < 0x100) {
                return cgb_boot_rom[addr];
            } else if (addr < 0x900 && addr > 0x1FF) {
                return cgb_boot_rom[addr - 0x100];
            }            
        } else if (addr < 0x100) {
            return dmg_boot_rom[addr];
        }
    } 
    // Check if reading from Memory Bank Controller
    if (addr < 0x8000 || ((uint16_t)(addr - 0xA000) < 0x2000)) {
        return read_MBC(addr);   
    }

    if ((uint16_t)(addr - ECHO_RAM_START) < 0x1DFF) { 
	    addr -= 0x2000;
    }
    
    // Read from "ordinary" memory (0x8000 - 0xFDFF)
    if (addr < 0xFE00) {

        // Check if reading from alternative VRAM with Gameboy Color
        if(cgb && cgb_vram_bank && (is_booting || cgb_features) && addr >= 0x8000 && addr < 0xA000) {
            return vram_bank_1[addr - 0x8000];
        }
        
        if (cgb && cgb_ram_bank > 1 && addr >= 0xD000 && addr <= 0xDFFF) {
           return cgb_ram_banks[cgb_ram_bank - 2][addr - 0xD000];
        }

        return mem[addr - 0x8000];
	}
    // Read from Object Attribute Table (0xFE00 - 0xFEFF) 
    if ((uint16_t)(addr - 0xFE00) < 0x100) {
        return oam_get_mem(addr - 0xFE00);
    }
    // Read from IO mem
    if (addr >= 0xFF10 && addr <= 0xFF3F) {
        return read_apu(addr);
    }
    return io_mem[addr - 0xFF00];

}


/* Write 16bit value starting at the given memory address 
 * into memory.  Written in little-endian byte order */
void set_mem_16(uint16_t const loc, uint16_t const val) {
    set_mem(loc + 1, val >> 8);
    set_mem(loc, val & 0xFF);
}


/* Read contents of 2 memory locations starting at the
 * given address. Returned as little-endian byte order 16 bit value */
uint16_t get_mem_16(uint16_t const loc) {
    return (get_mem(loc + 1) << 8) |
            get_mem(loc);
}


// read a value from gameboy color background palette RAM
uint8_t read_bg_color_palette(int addr) {
    return bg_palette_mem[addr];
}

// Read a vlue from gameboy color sprite palette RAM
uint8_t read_sprite_color_palette(int addr) {
    return sprite_palette_mem[addr];
}


void teardown_memory() {
    teardown_MBC();
}

