#pragma once

#include "superfx.h"

typedef struct Quat {
  float x;
  float y;
  float z;
  float w;
} Quat;

typedef struct Vec3 {
  float x;
  float y;
  float z;
} Vec3;

typedef struct {
  int width;
  int height;
} Size;

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
  int index;
  int rows;
  int columns;
  char* text;
} Quad;

typedef struct {
  float left_trigger;
  float right_trigger;
  float left_x_axis;
  float left_y_axis;
  float right_x_axis;
  float right_y_axis;
  bool up;
  bool down;
  bool left;
  bool right;
  bool back;
  bool start;
  bool xbox;
  bool y;
  bool x;
  bool b;
  bool a;
  bool left_shoulder;
  bool right_shoulder;
  bool left_joystick;
  bool right_joystick;

  bool start_pressed;
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
  int index;
  int rows;
  int columns;
} Sprite;

struct Entity {
  Type type;
  float transform[16];
  float inverse_world[16];
  union {
    Model model;
    Sprite sprite;
    Quad quad;
  } value;
};
