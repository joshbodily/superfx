#ifndef __TEXT_H__
#define __TEXT_H__

#include <SDL_ttf.h>

typedef struct Font {
  char name[128];
  TTF_Font* ttf;
} Font;

extern Font* g_fonts;

void init_fonts();
void create_text(const char* font_name, const char* text, SDL_Color color, GLuint texture, int* width, int* height);
//GLuint create_text(const char* font_name, const char* text, SDL_Color color, int* width, int* height);

#endif
