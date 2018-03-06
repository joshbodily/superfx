// gcc parse.c writepng.c -lpng -lGL
#include <dirent.h>
#include <OpenGL/gl.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "entity.h"
#include "parse.h"

#define SHADOW_FLAG 0x1

void parse_model(const char* path, Mesh* mesh, float* minx, float* miny, float* minz, float* maxx, float* maxy, float* maxz);
GLuint png_texture_load(const char * file_name, int * width, int * height);

extern Entity* g_camera;

int g_modelCount = 0;
Mesh* g_models;
int g_textureCount = 0;
TextureDef* g_textures;

void create_sprite_entity(Entity* entity, const char* textureName, mat4_t xform) {
  assert(entity);

  short textureIndex = -1; 
  for (int i = 0; i < g_textureCount; ++i) {
    if (strcmp(g_textures[i].name, textureName) == 0) {
      textureIndex = i;
    }
  }
  assert(textureIndex != -1);

  mat4_t transform = mat4_create(xform);
  mat4_transpose(transform, NULL);

  entity->type = SPRITE;
  entity->transform = transform;
  GLuint textureId = g_textures[textureIndex].id;
  entity->value.sprite.textureId = textureId;

  //char buffer[256];
  //mat4_str(node->transform, buffer);
  //printf(">> %s %s %d\n", textureName, buffer, textureIndex);
}

Mesh* get_model_mesh(const Entity* entity) {
  assert(entity->value.model.modelIndex < g_modelCount);
  return g_models + entity->value.model.modelIndex;
}

void create_mesh_entity(Entity* entity, const char* modelName, mat4_t xform) {
  assert(entity);

  short modelIndex = -1; 
  for (int i = 0; i < g_modelCount; ++i) {
    if (strcmp(g_models[i].name, modelName) == 0) {
      modelIndex = i;
    }
  }
  assert(modelIndex != -1);

  mat4_t transform = mat4_create(xform);
  mat4_transpose(transform, NULL);
  mat4_t inverse_world = mat4_create(NULL);
  mat4_inverse(transform, inverse_world);

  entity->type = MODEL;
  entity->transform = transform;
  entity->inverse_world = inverse_world;
  entity->value.model.modelIndex = modelIndex;
  entity->value.model.mode = GL_TRIANGLES;
  //entity->value.model.flags = 0x0;
  entity->value.model.flags = 0x1;

  /*char buffer[256];
  mat4_str(node->transform, buffer);
  printf(">> %s\n", buffer);
  mat4_str(node->inverse_world, buffer);
  printf("Inverse >>%s\n", buffer);*/
}

void new_camera(Entity* entity) {
  assert(entity);
  mat4_t transform = mat4_create(NULL);
  mat4_identity(transform);
  entity->type = CAMERA;
  entity->transform = transform;
  g_camera = entity;
}

void new_mesh_entity(Entity* entity, const char* modelName) {
  mat4_t identity = mat4_create(NULL);
  mat4_identity(identity);
  create_mesh_entity(entity, modelName, identity);
}

int parse_level(Entity *entity, const char* file) {
  FILE* fp;
  char* line = NULL;
  size_t len = 0;
  char url[128];
  float xform[16];

  int vertices = 0;
  int faces = 0;

  fp = fopen(file, "r");

  // Parse header
  getline(&line, &len, fp);
  int count = 0;
  assert(sscanf(line, "%d", &count) == 1);

  int parse = 0;
  for (int i = 0; i < count; ++i) {
    assert(getline(&line, &len, fp) != -1);
    assert(getline(&line, &len, fp) != -1);
    assert(sscanf(line, "url: %s", url) == 1);
    assert(getline(&line, &len, fp) != -1);
    assert(sscanf(line, "matrix: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", 
          &(xform[0]), &(xform[1]), &(xform[2]), &(xform[3]),
          &(xform[4]), &(xform[5]), &(xform[6]), &(xform[7]),
          &(xform[8]), &(xform[9]), &(xform[10]), &(xform[11]),
          &(xform[12]), &(xform[13]), &(xform[14]), &(xform[15])) == 16);
    //xform[0] *= 5.0f;
    //xform[5] *= 5.0f;
    //xform[10] *= 5.0f;
    //xform[3] *= 5.0f;
    //xform[7] *= 5.0f;
    //xform[11] *= 5.0f;
    create_mesh_entity(entity, url, xform);
    entity += 1;
  }

  return count;
}

void parse_textures() {
  DIR *d;
  struct dirent *dir;
  d = opendir("textures");
  int count = 0;
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".png") != NULL) {
        count++;
      }
    }
    rewinddir(d);
    g_textures = (TextureDef*)malloc(sizeof(TextureDef) * count);
    int i = 0;
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".png") != NULL) {
        TextureDef* texture = &(g_textures[i++]);
        strcpy(texture->name, dir->d_name);
        char path[128];
        sprintf(path, "textures/%s", texture->name);
        texture->id = png_texture_load(path, &texture->width, &texture->height); 
      }
    }
    closedir(d);
  }

  g_textureCount = count;
}

void parse_models() {
  DIR *d;
  struct dirent *dir;
  d = opendir("models");
  int count = 0;
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".ply") != NULL) {
        count++;
      }
    }
    rewinddir(d);
    g_models = (Mesh*)malloc(sizeof(Mesh) * count);
    int i = 0;
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".ply") != NULL) {
        Mesh* model = &(g_models[i++]);
        strcpy(model->name, dir->d_name);
        char path[128];
        sprintf(path, "models/%s", model->name);
        parse_model(path, model, &model->minx, &model->miny, &model->minz, &model->maxx, &model->maxy, &model->maxz);
        printf("%s >> %f %f %f => %f %f %f\n", path, model->minx, model->miny, model->minz, model->maxx, model->maxy, model->maxz);
        for (int i = 0; i < model->num_vertices; ++i) {
          
        }
      }
    }
    closedir(d);
  }

  g_modelCount = count;
}

void parse_model(const char* path, Mesh* model, float* minx, float* miny, float* minz, float* maxx, float* maxy, float* maxz) {
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
  *maxx = *maxy = *maxz = 0.0f;
  *minx = *miny = *minz = 99.9f;
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
    if (x > *maxx) { *maxx = x; }
    if (y > *maxy) { *maxy = y; }
    if (z > *maxz) { *maxz = z; }
    if (x < *minx) { *minx = x; }
    if (y < *miny) { *miny = y; }
    if (z < *minz) { *minz = z; }
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

  // Convert vertex normals to face normals
  /*for (int i = 0; i < faces; ++i) {
    int index1 = model->indices[3 * i];
    int index2 = model->indices[3 * i + 1];
    int index3 = model->indices[3 * i + 2];

    float A[3], B[3], C[3], cross[3], BA[3], CA[3];
    A[0] = model->vertices[index1 * 11]; A[1] = model->vertices[index1 * 11 + 1]; A[2] = model->vertices[index1 * 11 + 2];
    B[0] = model->vertices[index2 * 11]; B[1] = model->vertices[index2 * 11 + 1]; B[2] = model->vertices[index2 * 11 + 2];
    C[0] = model->vertices[index3 * 11]; C[1] = model->vertices[index3 * 11 + 1]; C[2] = model->vertices[index3 * 11 + 2];
    BA[0] = B[0] - A[0]; BA[1] = B[1] - A[1]; BA[2] = B[2] - A[2];
    CA[0] = C[0] - A[0]; CA[1] = C[1] - A[1]; CA[2] = C[2] - A[2];
    //vec3_t vec3_cross (vec3_t vec, vec3_t vec2, vec3_t dest);
    vec3_cross(BA, CA, cross);
    vec3_normalize(cross, cross);

    model->vertices[index1 * 11 + 3] = cross[0];
    model->vertices[index1 * 11 + 4] = cross[1];
    model->vertices[index1 * 11 + 5] = cross[2];

    model->vertices[index2 * 11 + 3] = cross[0];
    model->vertices[index2 * 11 + 4] = cross[1];
    model->vertices[index2 * 11 + 5] = cross[2];

    model->vertices[index3 * 11 + 3] = cross[0];
    model->vertices[index3 * 11 + 4] = cross[1];
    model->vertices[index3 * 11 + 5] = cross[2];
  }*/

  glGenBuffers(1, &(model->verticesID));
  glBindBuffer(GL_ARRAY_BUFFER, model->verticesID);
  glBufferData(GL_ARRAY_BUFFER, model->num_vertices * sizeof(GLfloat), model->vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &(model->indicesID));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->indicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->num_indices * sizeof(GLuint), model->indices, GL_STATIC_DRAW);
}
