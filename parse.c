// gcc parse.c writepng.c -lpng -lGL

#include <dirent.h>
#include <OpenGL/gl.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
  GLuint verticesID;
  GLuint indicesID;
  int num_vertices;
  GLfloat* vertices;
  int num_indices;
  GLuint* indices;
  char name[128];
  int width;
  int height;
} ModelDef; 

typedef struct {
  GLuint id;
  char name[128];
  int width;
  int height;
} TextureDef; 

void parse_model(const char* path, ModelDef* model);
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

void print_model(ModelDef* model) {
  printf("name: %s\n", model->name);
  printf("verticesID: %d\n", model->verticesID);
  printf("indicesID: %d\n", model->indicesID);
  printf("vertices %lu\n", model->num_vertices);
  printf("indices %lu\n", model->num_indices);

  printf("first %lu\n", model->indices[0]);
  printf("last %lu\n", model->indices[65]);

  printf("first %f\n", model->vertices[0]);
  printf("last %f\n", model->vertices[681]);
}

ModelDef* parse_models(int* out) {
  DIR *d;
  struct dirent *dir;
  d = opendir(".");
  int count = 0;
  ModelDef* models;
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".ply") != NULL) {
        count++;
      }
    }
    rewinddir(d);
    models = (ModelDef*)malloc(sizeof(ModelDef) * count);
    int i = 0;
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".ply") != NULL) {
        ModelDef* model = &(models[i++]);
        strcpy(model->name, dir->d_name);
        parse_model(dir->d_name, model);
print_model(model);
      }
    }
    closedir(d);
  }

  *out = count;
  return models;
}

void parse_model(const char* path, ModelDef* model) {
  FILE* fp;
  char* line = NULL;
  size_t len = 0;

  int vertices = 0;
  int faces = 0;

  fp = fopen(path, "r");

  // Parse header
  int parse = 0;
  while (getline(&line, &len, fp) != -1) {
    int temp = 0;
    int count = sscanf(line, "element vertex %d", &temp);
    if (count == 1) { vertices = temp; }
    count = sscanf(line, "element face %d", &temp);
    if (count == 1) { faces = temp; }
    if (strcmp(line, "end_header\n") == 0) { 
      break; 
    }
  }


  // Parse vertices
  model->vertices = (GLfloat*)malloc(sizeof(GLfloat) * vertices * 11);
  model->num_vertices = vertices * 11;
  float x, y, z, nx, ny, nz, u, v, r, g, b;
  for (int i = 0; i < vertices; ++i) {
    getline(&line, &len, fp);
    size_t count = sscanf(line, "%f %f %f %f %f %f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz, &u, &v, &r, &g, &b);
    model->vertices[i * 11] = x;
    model->vertices[i * 11 + 1] = y;
    model->vertices[i * 11 + 2] = z;
    model->vertices[i * 11 + 3] = nx;
    model->vertices[i * 11 + 4] = ny;
    model->vertices[i * 11 + 5] = nz;
    model->vertices[i * 11 + 6] = u;
    model->vertices[i * 11 + 7] = v;
    model->vertices[i * 11 + 8] = r / 255.0;
    model->vertices[i * 11 + 9] = g / 255.0;
    model->vertices[i * 11 + 10] = b / 255.0;
    assert(count == 11);
  }

  // Parse faces
  model->indices = (GLuint*)malloc(sizeof(GLuint) * faces * 3);
  model->num_indices = faces * 3;
  int f1, f2, f3;
  for (int i = 0; i < faces; ++i) {
    getline(&line, &len, fp);
    size_t count = sscanf(line, "3 %d %d %d", &(model->indices[3 * i]), &(model->indices[3 * i + 1]), &(model->indices[3 * i + 2]));
    assert(count == 3);
  }

  glGenBuffers(1, &(model->verticesID));
  glBindBuffer(GL_ARRAY_BUFFER, model->verticesID);
  glBufferData(GL_ARRAY_BUFFER, model->num_vertices * sizeof(GLfloat), model->vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &(model->indicesID));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->indicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->num_indices * sizeof(GLuint), model->indices, GL_STATIC_DRAW);

}
