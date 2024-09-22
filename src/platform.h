#pragma once

#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <stddef.h>

class Platform {
public:
  Platform(const char *title, size_t window_width, size_t window_height,
           size_t texture_width, size_t texture_height);

  ~Platform();

  /*
   * Game loop update function.
   */
  void update(void const *buffer, int pitch);

  /*
   * Take events, input from window and user.
   */
  bool process_input(uint8_t *keys);

private:
  SDL_Window *window{};
  SDL_Renderer *renderer{};
  SDL_Texture *texture{};
};