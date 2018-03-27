#pragma once

#include "superfx.h"

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
  float maxx, maxy, maxz; // Bounding Box
  float minx, miny, minz; // Bounding Box
} Mesh; 

typedef struct {
  GLuint id;
  char name[128];
  int width;
  int height;
} Texture; 

typedef struct {
  GLuint texture_id;
  char* text;
  int width;
  int height;
  int x;
  int y;
} Quad; 

typedef struct {
  float x_axis;
  float y_axis;
  float roll;
  bool fire;
} Input;

typedef enum Type {
  NODE = 0x1,
  MODEL = 0x2,
  SPRITE = 0x3,
  CAMERA = 0x4,
  QUAD = 0x5
} Type;

typedef struct Entity Entity;
typedef struct {
  short mode;
  short mesh_index; 
} Model;

typedef struct {
  short texture_id;
} Sprite;

struct Entity {
  Type type;
  mat4_t transform;
  mat4_t inverse_world;
  union {
    Model model;
    Sprite sprite;
    Quad quad;
  } value;
};
