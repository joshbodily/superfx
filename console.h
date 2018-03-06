#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <SDL2/SDL.h>
#include <lua.h>
#include <stdbool.h>

typedef struct Console {
  int cursor_location;
  char current_line[512];
  char scrollback[10][512];
  int scrollback_index;
  bool visible;
  bool lshift;
  bool rshift;
  bool dirty;
  int w;
  int h;
} Console;

extern Console g_console;

short console_handle_key(SDL_Event* event);
void render_console(GLuint texture);

extern lua_State *L;

#endif
