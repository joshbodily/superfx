#ifndef PARSE_H
#define PARSE_H

#include <OpenGL/gl.h>
#include "parse.h"
#include "entity.h"

extern TextureDef* g_textures;

void parse_textures();
GLuint png_texture_load(const char * file_name, int * width, int * height);
void parse_models();
int parse_level(Entity *entity, const char* file);

// Camera
void new_camera(Entity* entity);

// Models
void new_mesh_entity(Entity* entity, const char* modelName);
void create_mesh_entity(Entity* entity, const char* modelName, mat4_t xform);
Mesh* get_model_mesh(const Entity* entity);

// Sprites
void create_sprite_entity(Entity* entity, const char* textureName, mat4_t xform);

#endif
