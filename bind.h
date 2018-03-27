#pragma once

#include "types.h"

// Create
void new_mesh_entity(Entity* entity, const char* model_name);
void create_mesh_entity(Entity* entity, const char* model_name, mat4_t xform);
void create_sprite_entity(Entity* entity, const char* texture_name, mat4_t xform);

// Render
void render(const Entity* entity);
void render_points(const Entity* entity);
void render_shadow(const Entity* entity);
void render_quad(const Entity* entity);

// Update
void move_to(Entity* entity, float x, float y, float z);
void translate(Entity* entity, float x, float y, float z);
void scale(Entity* entity, float x, float y, float z);
void rotate(Entity* entity, float x, float y, float z, float angle);

// Camera
void init_camera(Entity* entity);
void look_at(Entity* camera, Entity* target);
