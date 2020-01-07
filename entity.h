#ifndef ENTITY_H
#define ENTITY_H

#include <gl-matrix.h>
#ifdef DARWIN
  #include <OpenGL/gl.h>
#else
  #include <GL/gl.h>
#endif

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
} TextureDef; 

typedef struct {
  float x_axis;
  float y_axis;
  float roll;
  char fire;
} Input;

typedef enum Type {
  NODE = 0x1,
  MODEL = 0x2,
  SPRITE = 0x3,
  CAMERA = 0x4
} Type;

/*typedef struct Node Node;
struct Node {
  //vec3_t position;
  //vec3_t rotation;
  Node* parent;
};*/

typedef struct Entity Entity;
typedef struct {
  // Mesh stuff
  int flags;
  short mode;
  short modelIndex; 
  void (*update)(Entity*);
} Model;

typedef struct {
  // Mesh is implied to be a quad
  // Sprite stuff
  short textureId;
  void (*update)(Entity*);
} Sprite;

struct Entity {
  Type type;
  //Node* node;
  mat4_t transform;
  mat4_t inverse_world;
  union {
    Model model;
    Sprite sprite;
  } value;
};

#endif
