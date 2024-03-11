#include <stdint.h>
#include <stdio.h>
#include "stdlib.h"
#include "time.h"

#define START_ADDRESS	0x200
#define FONTSET_SIZE	80
#define FONTSET_START	0x50
#define VIDEO_WIDTH	64
#define VIDEO_HEIGHT	32

static uint8_t fontset[FONTSET_SIZE] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, //0
  0x20, 0x60, 0x20, 0x20, 0x70, //1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
  0x90, 0x90, 0xF0, 0x10, 0x10, //4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
  0xF0, 0x10, 0x20, 0x40, 0x40,	//7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
  0xF0, 0x90, 0xF0, 0x90, 0x90, //A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
  0xF0, 0x80, 0x80, 0x80, 0xF0, //C
  0xE0, 0x90, 0x90, 0x90, 0xE0, //D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
  0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

struct chip8 {
  uint8_t registers[16];
  uint8_t memory[4096];
  uint16_t index;
  uint16_t PC;
  uint16_t stack[16];
  uint8_t SP;
  uint8_t delay_timer;
  uint8_t sound_timer;
  uint8_t keypad[16];
  uint32_t display_mem[VIDEO_WIDTH*VIDEO_HEIGHT];
  uint16_t opcode;
};

typedef void c8_instruction(struct chip8* c8, uint16_t instruction);
//declare function tables globally so we don't have to reinitialize tables in every table function
static c8_instruction *table[0xF + 1];
static c8_instruction *table0[0xE + 1];
static c8_instruction *table8[0xE + 1];
static c8_instruction *tableE[0xE + 1];
static c8_instruction *tableF[0x65 + 1];

void loadROM(const char* filename, struct chip8* c8);
void initializeChip8(struct chip8* c8);
uint8_t getRandom();
void cycle(struct chip8 *c8);

void initializeTables();

void Table0(struct chip8* c8, uint16_t instruction);
void Table8(struct chip8* c8, uint16_t instruction);
void TableE(struct chip8* c8, uint16_t instruction);
void TableF(struct chip8* c8, uint16_t instruction);