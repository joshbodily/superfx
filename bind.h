#pragma once

#include "input.h"
#include "types.h"

// BG
void set_background(const char* texture_name);

// Create
void new_mesh_entity(Entity* entity, const char* model_name);
void new_sprite_entity(Entity* entity, const char* texture_name, int rows, int columns);
Size new_quad_entity(Entity* entity, const char* texture_name);
void set_quad_index(Entity* entity, int index);
Size new_text_quad_entity(Entity* entity, const char* text);
//void create_mesh_entity(Entity* entity, const char* model_name, mat4_t xform);
//void create_sprite_entity(Entity* entity, const char* texture_name, mat4_t xform);

// Models 
void render(const Entity* entity);
void render_points(const Entity* entity);
void render_shadow(const Entity* entity);
// Sprites
void render_sprite(const Entity* entity);
// UI Quads
void render_quad(const Entity* entity, bool invert);

// 

// Update
void move_to(Entity* entity, float x, float y, float z);
Vec3 get_location(Entity* entity);
void print_rotation(Entity* entity);
void identity(Entity* entity);
void set_translation_world(Entity* entity, float x, float y, float z);
void translate(Entity* entity, float x, float y, float z);
Vec3 transform(Entity* entity, float x, float y, float z);
void set_transform(Entity* src, Entity* dst);
void scale(Entity* entity, float x, float y, float z);
void set_scale(Entity* entity, float x, float y, float z);
void reset_rotation(Entity* entity, float x, float y, float z);
void rotate(Entity* entity, float x, float y, float z, float angle);
void rotateQuat(Entity* entity, float x, float y, float z, float w);
void rotateXYZ(Entity* entity, float x, float y, float z);
Quat slerp(float x0, float y0, float z0, float w0, float x1, float y1, float z1, float w1, float t);

// Camera
void init_camera(Entity* entity);
void look_at(Entity* camera, Entity* target);

// Input
Input get_input();

// Collision
bool collide(Entity* arwing, Entity* object);//, vec3_t normal);
