#include "superfx.h"
#include <assert.h>
#include <dirent.h>
#include "text.h"

Font* g_fonts;

void init_fonts() {
  TTF_Init();

  DIR *d;
  struct dirent *dir;
  d = opendir("fonts");
  int count = 0;
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".ttf") != NULL) {
        count++;
      }
    }
    rewinddir(d);
    g_fonts = (Font*)malloc(sizeof(Font) * count + 1);
    int i = 0;
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".ttf") != NULL) {
        Font* font = &(g_fonts[i++]);
        font->ttf = TTF_OpenFont("fonts/MEGAMAN10.ttf", 32);
        strcpy(font->name, dir->d_name);
        SDL_Log("Loading font %s %x", font->name, font->ttf);
        //char path[128];
        //sprintf(path, "models/%s", model->name);
      }
    }
    closedir(d);
    g_fonts[i].ttf = NULL;
  }
}

TTF_Font* get_font(const char* name) {
  int i = 0;
  Font* iter = g_fonts;
  while (iter->ttf) {
    //SDL_Log("Checking font %s", iter->name);
    if (strcmp(iter->name, name) == 0) {
      return iter->ttf;
    }
    ++iter;
  }
  return NULL;
}

void create_text(const char* font_name, const char* text, SDL_Color color, GLuint texture, int* width, int* height) {
  TTF_Font* ttf = get_font(font_name);
  //SDL_Color color = {255, 255, 255, 255};
  if (ttf == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "TTF_OpenFont error\n");
  }
  SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(ttf, text, color, 512);
  if (surface == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "TTF_RenderText_Blended  error\n");
  }
  // Bind the SDL_Texture in GL
  //GLuint texture;
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  *width = surface->w; // power_two_floor(surface->w)*2;
  *height = surface->h; // power_two_floor(surface->h)*2;
  SDL_Surface *glsurface = SDL_CreateRGBSurface(0, *width, *height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
  SDL_BlitSurface(surface, NULL, glsurface, NULL);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, glsurface->pixels);
  //return texture;
}

// Draw the text
// renderQuad(ttfTextureId, 0, 0, w, h, TRUE);
