#include "platform.h"
#include "chip8.h"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    return -1;
  }
  int videoScale = atoi(argv[1]);
  int cycleDelay = atoi(argv[2]);
  const char *romFilename = argv[3];

  struct platform *p = (struct platform *)malloc(sizeof(struct platform));
  initializePlatform(p, "CHIP-8 Emulator", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);
  
  struct chip8* c8 = (struct chip8*)malloc(sizeof(struct chip8));
  initializeChip8(c8);
  
  initializeTables();
  loadROM(romFilename, c8);
  
  int videoPitch = sizeof(c8->display_mem[0]) * VIDEO_WIDTH;

  time_t start_t = clock();
  int quit = 0;
  while (!quit) {
    quit = platformProcessInput(c8->keypad);

    time_t curr_t = clock();
    if ((curr_t - start_t)/(CLOCKS_PER_SEC/1000) > cycleDelay) {
      start_t = clock();
      cycle(c8);
      platformUpdate(p, c8->display_mem, videoPitch);
    }
  }

  destroyPlatform(p);
  free(c8);
  return 0;
}