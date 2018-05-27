#pragma once

#include "superfx.h"
#include "types.h"

// BG
Entity background_entity;

// Shaders
extern GLuint shadow_program;
extern GLuint mesh_program;
extern GLuint sprite_program;
extern GLuint quad_program;
void load_shaders();

// Meshes
extern int g_mesh_count;
Mesh* g_meshes;
void load_meshes();
Mesh* get_entity_mesh(const Entity* entity);

// Textures
extern int g_texture_count;
Texture* g_textures;
void load_textures();
Texture* get_texture(const char* name);

// Fonts
void create_text(const char* font_name, int size, const char* text, SDL_Color color, GLuint texture, int* width, int* height);
