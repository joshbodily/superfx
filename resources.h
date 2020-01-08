#pragma once

#include "superfx.h"
#include "types.h"
#include <glib.h>

// BG
Entity background_entity;

// Shaders
extern GLuint shadow_program;
//extern GLuint mesh_program;
extern GLuint texture_program;
extern GLuint color_program;
extern GLuint sprite_program;
extern GLuint quad_program;
int compile_shaders(const char* vert_file, const char* frag_file, GLuint* program);
void load_shaders();

// Meshes
extern GList* g_meshes_list;
int get_mesh_index(const char* model_name);
void create_mesh(const char* path, Vertex vertices[], int num_vertices, Face faces[], int num_faces);
void load_meshes();
Mesh* get_entity_mesh(const Entity* entity);

// Textures
extern int g_texture_count;
Texture* g_textures;
void load_textures();
Texture* get_texture(const char* name);

// Fonts
void create_text(const char* font_name, int size, const char* text, SDL_Color color, GLuint texture, int* width, int* height);
