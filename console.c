#include <GL/gl.h>
#include <ctype.h>
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
#include <assert.h>
#include "console.h"
#include "text.h"

Console g_console;
lua_State *L;

void renderQuad(GLuint texture, int x, int y, int w, int h, short invert);
void process_console_command();

short console_handle_key(SDL_Event* event) {
  bool key_down = event->type == SDL_KEYDOWN;
  if (event->key.keysym.sym == SDLK_BACKQUOTE && key_down) {
    g_console.visible = !g_console.visible;
    g_console.cursor_location = 2;
    g_console.current_line[0] = '>';
    g_console.current_line[1] = ' ';
    g_console.current_line[2] = '\0';
    if (g_console.visible) return false;
  }

  if (event->key.keysym.sym == SDLK_LSHIFT) {
    g_console.lshift = key_down;
  }
  if (event->key.keysym.sym == SDLK_RSHIFT) {
    g_console.rshift = key_down;
  }

  if (g_console.visible && key_down) {
    char c = event->key.keysym.sym;
    // Delete
    if (event->key.keysym.sym == SDLK_BACKSPACE) {
      g_console.cursor_location--;
      if (g_console.cursor_location <= 2) g_console.cursor_location = 2;
      //g_console.current_line[g_console.cursor_location] = '\0';
    }
    // Letters
    if (isalpha(c)) {
      g_console.current_line[g_console.cursor_location++] = (g_console.lshift || g_console.rshift) ? c - 0x20 : c;
    // Other Chars
    } else if (isprint(c)) {
      if (g_console.lshift || g_console.rshift) {
        switch (c) {
          case '1': c = '!'; break;
          case '2': c = '@'; break;
          case '3': c = '#'; break;
          case '4': c = '$'; break;
          case '5': c = '%'; break;
          case '6': c = '^'; break;
          case '7': c = '&'; break;
          case '8': c = '*'; break;
          case '9': c = '('; break;
          case '0': c = ')'; break;
          case '-': c = '_'; break;
          case '=': c = '+'; break;
          case '[': c = '{'; break;
          case ']': c = '}'; break;
          case '\\': c = '|'; break;
          case ';' : c = ':'; break;
          case '\'': c = '"'; break;
          case ',' : c = '<'; break;
          case '.' : c = '>'; break;
          case '/' : c = '?'; break;
        }
      }
      g_console.current_line[g_console.cursor_location++] = c;
    } else if (c == SDLK_RETURN) {
      process_console_command();
    }
  }
  g_console.current_line[g_console.cursor_location] = '\0';
  g_console.dirty = true;

  return true;
}

void process_console_command() {
  if (!g_console.visible) return;

	int error;
  error = luaL_loadbuffer(L, &g_console.current_line[2], strlen(g_console.current_line) - 2, "console"); 
  if (error) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    error = lua_pcall(L, 0, 0, 0);
    if (error) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", lua_tostring(L, -1));
      lua_pop(L, 1);
    }
  }
  g_console.cursor_location = 2;
  g_console.current_line[2] = '\0';
}

void render_console(GLuint texture) {
  if (g_console.visible && g_console.dirty) {
    SDL_Color white = {255, 255, 255, 255};
    //SDL_Log("Console: %s", g_console.current_line);
    create_text("MEGAMAN10.ttf", g_console.current_line, white, texture, &g_console.w, &g_console.h);
    g_console.dirty = false;
  }
  if (g_console.visible) {
    renderQuad(texture, 0, 256 - g_console.h/2, g_console.w, g_console.h, true);
  }
}
