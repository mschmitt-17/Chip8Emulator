#include "chip8.h"

void loadROM(const char* filename, struct chip8* c8) {
  FILE* fp = fopen(filename, "r");
  int i = 0;
  do {
    char c = fgetc(fp);
    if (feof(fp)) {
      break;
    }
    c8->memory[START_ADDRESS + i] = c;
    i++;
  } while(1);
  fclose(fp);
}

void initializeChip8(struct chip8* c8) {
  c8->PC = START_ADDRESS;
  for (int i = 0; i < FONTSET_SIZE; i++) {
    c8->memory[FONTSET_START+i] = fontset[i];
  }
  for (int i = 0; i < VIDEO_WIDTH*VIDEO_HEIGHT; i++) {
    c8->display_mem[i] = 0x0;
  } 
  srand(time(0));
}

uint8_t getRandom() {
  return (rand()%256);
}

void cycle(struct chip8 *c8) {
  //opcode is 2 locations in memory
  uint16_t opcode = (c8->memory[c8->PC] << 8) | c8->memory[(c8->PC) + 1];
  c8->PC += 2;
  table[(opcode & 0xF000) >> 12](c8, opcode);
  if (c8->delay_timer > 0) {
    c8->delay_timer--;
  }
  if (c8->sound_timer > 0) {
    c8->sound_timer--;
  }
}

//00E0: Clear the Display
void OP_00E0(struct chip8* c8, uint16_t instruction) {
  for (long i = 0; i < 64*32; i++) {
    c8->display_mem[i] = 0;
  }
}

//00EE: Return from a subroutine
void OP_00EE(struct chip8* c8, uint16_t instruction) {
  c8->PC = c8->stack[c8->SP];
  c8->SP--;
}

//1nnn: jump to nnn
void OP_1nnn(struct chip8* c8, uint16_t instruction) {
  c8->PC = instruction & 0x0FFF; 
}

//2nnn: call subroutine at address nnn
void OP_2nnn(struct chip8* c8, uint16_t instruction) {
  c8->SP++;
  c8->stack[c8->SP] = c8->PC;
  c8->PC = instruction & 0x0FFF;
}

//3xkk: skip next instruction if register Vx contains kk
void OP_3xkk(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t kk = instruction & 0x00FF;
  if (c8->registers[register_offset_1] == kk) {
    c8->PC += 2;
  }
}

//4xkk: skip next instruction if register Vx doesn't contain kk
void OP_4xkk(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t kk = instruction & 0x00FF;
  if (c8->registers[register_offset_1] != kk) {
    c8->PC += 2;
  }
}

//5xy0: skip next instruction if register Vx equals register Vy
void OP_5xy0(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t register_offset_2 = (instruction & 0x00F0) >> 4;
  if (c8->registers[register_offset_1] == c8->registers[register_offset_2]) {
    c8->PC += 2;
  }
}

//6xkk: load kk into register Vx
void OP_6xkk(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t kk = instruction & 0x00FF;
  c8->registers[register_offset_1] = kk;
}

//7xkk: set Vx = Vx + kk
void OP_7xkk(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t kk = instruction & 0x00FF;
  c8->registers[register_offset_1] = c8->registers[register_offset_1] + kk;
}

void Table8(struct chip8* c8, uint16_t instruction);

//8xy0: set Vx = Vy
void OP_8xy0(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t register_offset_2 = (instruction & 0x00F0) >> 4;
  c8->registers[register_offset_1] = c8->registers[register_offset_2];
}

//8xy1: set Vx = Vx OR Vy
void OP_8xy1(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t register_offset_2 = (instruction & 0x00F0) >> 4;
  c8->registers[register_offset_1] = c8->registers[register_offset_1] | c8->registers[register_offset_2];	
}

//8xy2: set Vx = Vx AND Vy	
void OP_8xy2(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t register_offset_2 = (instruction & 0x00F0) >> 4;
  c8->registers[register_offset_1] = c8->registers[register_offset_1] & c8->registers[register_offset_2];	
}

//8xy3: set Vx = Vx XOR Vy
void OP_8xy3(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t register_offset_2 = (instruction & 0x00F0) >> 4;
  c8->registers[register_offset_1] = c8->registers[register_offset_1] ^ c8->registers[register_offset_2];	
}

//8xy4: set Vx = Vx + Vy, set Vf if overflow has occurred
void OP_8xy4(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t register_offset_2 = (instruction & 0x00F0) >> 4;
  uint16_t result = c8->registers[register_offset_1] + c8->registers[register_offset_2];
  if (result & 0x0F00) {
    c8->registers[0xF] = 1;
  } else {
    c8->registers[0xF] = 0;
  }
  c8->registers[register_offset_1] = result & 0x00FF;
}

//8xy5: set Vx = Vx - Vy, set Vf if Vx > Vy
void OP_8xy5(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t register_offset_2 = (instruction & 0x00F0) >> 4;
  if (c8->registers[register_offset_1] > c8->registers[register_offset_2]) {
    c8->registers[0xF] = 1;
  } else {
    c8->registers[0xF] = 0;
  }
  c8->registers[register_offset_1] = c8->registers[register_offset_1] - c8->registers[register_offset_2];	
}

//8xy6: right shift Vx by one and store the shifted bit in VF
void OP_8xy6(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  if (c8->registers[register_offset_1] & 0x01) {
    c8->registers[0xF] = 1;
  } else {
    c8->registers[0xF] = 0;
  }
  c8->registers[register_offset_1] = c8->registers[register_offset_1] >> 1;
}

//8xy7: set Vx = Vy - Vx, set Vf if Vy > Vx
void OP_8xy7(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t register_offset_2 = (instruction & 0x00F0) >> 4;
  if (c8->registers[register_offset_2] > c8->registers[register_offset_1]) {
    c8->registers[0xF] = 1;
  } else {
    c8->registers[0xF] = 0;
  }
  c8->registers[register_offset_1] = c8->registers[register_offset_2] - c8->registers[register_offset_1];	
}

//8xyE: left shift Vx by one and store the shifted bit in VF
void OP_8xyE(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  if (c8->registers[register_offset_1] & 0x80) {
    c8->registers[0xF] = 1;
  } else {
    c8->registers[0xF] = 0;
  }
  c8->registers[register_offset_1] = c8->registers[register_offset_1] << 1;
}

//9xy0: skip next instruction if Vx != Vy
void OP_9xy0(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t register_offset_2 = (instruction & 0x00F0) >> 4;
  if (c8->registers[register_offset_1] != c8->registers[register_offset_2]) {
    c8->PC += 2;
  }
}

//Annn: set index to nnn
void OP_Annn(struct chip8* c8, uint16_t instruction) {
  c8->index = instruction & 0x0FFF;
}

//Bnnn: jump to location nnn + V0
void OP_Bnnn(struct chip8* c8, uint16_t instruction) {
  c8->PC = (instruction & 0x0FFF) + c8->registers[0];
}

//Cxkk: set Vx = random byte AND kk
void OP_Cxkk(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t kk = instruction & 0x00FF;
  uint8_t random_byte = getRandom();
  c8->registers[register_offset_1] = random_byte & kk; 
}

//Dxyn: Display n bit sprite starting at memory location I at (Vx, Vy), VF = collision
void OP_Dxyn(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t register_offset_2 = (instruction & 0x00F0) >> 4;
  uint8_t bytes_to_read = instruction & 0x000F;

  uint8_t col = c8->registers[register_offset_1];
  uint8_t row = c8->registers[register_offset_2];
  //initialize register for collision to 0, if we have a collision we will set it to 1,
  //otherwise we will set it to 0 anyway
  c8->registers[0xF] = 0;

  for (int i = 0; i < bytes_to_read; i++) {
    uint8_t current_byte = c8->memory[(c8->index) + i];
    //each sprite is 8 pixels wide
    for (int j = 0; j < 8; j++) {
      //since each sprite is drawn starting at MSB, check MSB and left shift
      if (current_byte & 0x80) {
        if (c8->display_mem[(((row+i)%32)*64)+((col+j)%64)]) {
	  c8->registers[0xF] = 1;
	} 
	c8->display_mem[(((row+i)%32)*64)+((col+j)%64)] = 
		c8->display_mem[(((row+i)%32)*64)+((col+j)%64)] ^ 0xFFFFFFFF;
      }
      //omit XORing with 0 since this will result in the same result
      current_byte = current_byte << 1;
    }
  }
}

void TableE(struct chip8* c8, uint16_t instruction);

//ExA1: skip next instruction if key with the value of Vx is not pressed
void OP_Ex9E(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  if (c8->keypad[c8->registers[register_offset_1]]) {
    c8->PC += 2;
  } 
}

//Ex9E: skip next instruction if key with the value of Vx is pressed
void OP_ExA1(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  if (!(c8->keypad[c8->registers[register_offset_1]])) {
    c8->PC += 2;
  }
}

void TableF(struct chip8* c8, uint16_t instruction);

//Fx07: set Vx = delay timer value
void OP_Fx07(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  c8->registers[register_offset_1] = c8->delay_timer;
}

//Fx0A: wait for a key press, store the value of the key in Vx
void OP_Fx0A(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t valid_press = 0;
  for (int i = 0; i < 16; i++) {
    if (c8->keypad[i]) {
      c8->registers[register_offset_1] = i;
      valid_press = 1;
    }
  }
  //decrement PC so the instruction gets repeated (wait for key press)
  if (!valid_press) {
    c8->PC -= 2;
  }
}

//Fx15: set delay timer = Vx
void OP_Fx15(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  c8->delay_timer = c8->registers[register_offset_1];
}

//Fx18: set sound timer = Vx
void OP_Fx18(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  c8->sound_timer = c8->registers[register_offset_1];
}

//Fx1E: set index = index + Vx
void OP_Fx1E(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  c8->index += c8->registers[register_offset_1];
}

//Fx29: set index = location of sprite for digit Vx
void OP_Fx29(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  if (c8->registers[register_offset_1] <= 0xF) {
    //hex sprites are all 5 bytes
    c8->index = FONTSET_START + (c8->registers[register_offset_1] * 5);
  }
}

//Fx33: store BCD representation of Vx in memory locations I, I+1, and I+2
void OP_Fx33(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  uint8_t Vx_value = c8->registers[register_offset_1];
  uint8_t hundreds = 0;
  uint8_t tens = 0;
  uint8_t ones = 0;
  while (Vx_value >= 100) {
    Vx_value -= 100;
    hundreds++;
  }
  while (Vx_value >= 10) {
    Vx_value -= 10;
    tens++;
  }
  while (Vx_value >= 1) {
    Vx_value -= 1;
    ones++;
  }
  c8->memory[c8->index] = hundreds;
  c8->memory[(c8->index) + 1] = tens;
  c8->memory[(c8->index) + 2] = ones;
}

//Fx55: store registers V0 through Vx in memory starting at I
void OP_Fx55(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  for (int i = 0; i <= register_offset_1; i++) {
    c8->memory[(c8->index) + i] = c8->registers[i];
  }
}

//Fx65: read registers V0 through Vx from memory starting at location I
void OP_Fx65(struct chip8* c8, uint16_t instruction) {
  uint8_t register_offset_1 = (instruction & 0x0F00) >> 8;
  for (int i = 0; i <= register_offset_1; i++) {
    c8->registers[i] = c8->memory[(c8->index) + i];
  }
}

void initializeTables() {
  //default table
  table[0x0] = Table0; 
  table[0x1] = OP_1nnn; 
  table[0x2] = OP_2nnn;
  table[0x3] = OP_3xkk;
  table[0x4] = OP_4xkk;
  table[0x5] = OP_5xy0;
  table[0x6] = OP_6xkk;
  table[0x7] = OP_7xkk;
  table[0x8] = Table8;
  table[0x9] = OP_9xy0;
  table[0xA] = OP_Annn;
  table[0xB] = OP_Bnnn;
  table[0xC] = OP_Cxkk;
  table[0xD] = OP_Dxyn;
  table[0xE] = TableE;
  table[0xF] = TableF;
  //table0
  table0[0x0] = OP_00E0;
  table0[0xE] = OP_00EE;
  //table8
  table8[0x0] = OP_8xy0;
  table8[0x1] = OP_8xy1;
  table8[0x2] = OP_8xy2;
  table8[0x3] = OP_8xy3;
  table8[0x4] = OP_8xy4;
  table8[0x5] = OP_8xy5;
  table8[0x6] = OP_8xy6;
  table8[0x7] = OP_8xy7;
  table8[0xE] = OP_8xyE;
  //tableE
  tableE[0xE] = OP_Ex9E;
  tableE[0x1] = OP_ExA1;
  //tableF
  tableF[0x7] = OP_Fx07;
  tableF[0xA] = OP_Fx0A;
  tableF[0x15] = OP_Fx15;
  tableF[0x18] = OP_Fx18;
  tableF[0x1E] = OP_Fx1E;
  tableF[0x29] = OP_Fx29;
  tableF[0x33] = OP_Fx33;
  tableF[0x55] = OP_Fx55;
  tableF[0x65] = OP_Fx65;
}

void Table0(struct chip8* c8, uint16_t instruction) {
  table0[instruction & 0x000F](c8, instruction);
}

void Table8(struct chip8* c8, uint16_t instruction) {
  table8[instruction & 0x000F](c8, instruction);
}

void TableE(struct chip8* c8, uint16_t instruction) {
  tableE[instruction & 0x000F](c8, instruction);
}

void TableF(struct chip8* c8, uint16_t instruction) {
  tableF[instruction & 0x00FF](c8, instruction);
}