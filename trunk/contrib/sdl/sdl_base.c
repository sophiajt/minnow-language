#include "SDL.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define SCREEN_DEPTH 8

void *create_sdl_window_(int width, int height, int depth) {
  SDL_Surface *screen;

  SDL_Init(SDL_INIT_VIDEO);
  screen = SDL_SetVideoMode(width, height, depth, SDL_SWSURFACE);
  if (screen == NULL) {
    //TODO: Throw exception here instead
    fprintf(stderr, "Unable to set resolution to %ix%i - %s\n", width, height, SDL_GetError());
    exit(1);
  }
  return screen;
}

//Taken from: http://www.libsdl.org/intro.en/usingvideo.html
void draw_pixel_(void *screen_v, int R, int G, int B, int x, int y)
{
  SDL_Surface *screen = (SDL_Surface*) screen_v;
  Uint32 color = SDL_MapRGB(screen->format, R, G, B);

  if ( SDL_MUSTLOCK(screen) ) {
      if ( SDL_LockSurface(screen) < 0 ) {
          return;
      }
  }
  switch (screen->format->BytesPerPixel) {
    case 1: { /* Assuming 8-bpp */
      Uint8 *bufp;

      bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
      *bufp = color;
    }
    break;

    case 2: { /* Probably 15-bpp or 16-bpp */
      Uint16 *bufp;

      bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
      *bufp = color;
    }
    break;

    case 3: { /* Slow 24-bpp mode, usually not used */
      Uint8 *bufp;

      bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
      *(bufp+screen->format->Rshift/8) = R;
      *(bufp+screen->format->Gshift/8) = G;
      *(bufp+screen->format->Bshift/8) = B;
    }
    break;

    case 4: { /* Probably 32-bpp */
      Uint32 *bufp;

      bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
      *bufp = color;
    }
    break;
  }
  if ( SDL_MUSTLOCK(screen) ) {
      SDL_UnlockSurface(screen);
  }
  SDL_UpdateRect(screen, x, y, 1, 1);
  SDL_Flip(screen);
}

/*
int main(int argc, char *argv[]) {
     SDL_Surface *screen;
     Uint8       *p;
     int         x = 10; //x coordinate of our pixel
     int         y = 20; //y coordinate of our pixel
     
     screen = create_sdl_window_(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH);
     draw_pixel_(screen, 0xff, 0, 0, x, y);
     
     SDL_Event event;
     do {
        SDL_WaitEvent(&event);
     } while (event.type != SDL_QUIT);
}
*/
