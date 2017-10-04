// gcc parse.c writepng.c -lpng -lGL

#include <dirent.h>
#include <OpenGL/gl.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
  GLuint id;
  char name[128];
  int width;
  int height;
} TextureDef; 

GLuint png_texture_load(const char * file_name, int * width, int * height);

TextureDef* parse(int* out) {
  DIR *d;
  struct dirent *dir;
  d = opendir(".");
  int count = 0;
  TextureDef* textures;
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".png") != NULL) {
        count++;
      }
    }
    rewinddir(d);
    textures = (TextureDef*)malloc(sizeof(TextureDef) * count);
    int i = 0;
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".png") != NULL) {
        TextureDef* texture = &(textures[i++]);
        strcpy(texture->name, dir->d_name);
        texture->id = png_texture_load(dir->d_name, &texture->width, &texture->height); 
      }
    }
    closedir(d);
  }

  *out = count;
  return textures;
}

/*int main() {
  int count = 0;
  TextureDef* textures = parse(&count);
  for (int i = 0; i < count; ++i) {
    printf("%s %d %dx%d\n", textures[i].name, textures[i].id, textures[i].width, textures[i].height);
  }
}*/
