#include "SDL2/SDL.h"

struct platform {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
};

void initializePlatform(struct platform *p, const char *title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
void destroyPlatform(struct platform *p);
void platformUpdate(struct platform *p, const void *buffer, int pitch);
int platformProcessInput(uint8_t *keys);