#include "bind.h"
#include "resources.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

// Camera
Entity* g_camera;

void create_sprite_entity(Entity* entity, const char* texture_name, mat4_t xform) {
  assert(entity);
  assert(texture_name);
  assert(xform);

  int texture_index = -1; 
  for (int i = 0; i < g_texture_count; ++i) {
    if (strcmp(g_textures[i].name, texture_name) == 0) {
      texture_index = i;
    }
  }
  assert(texture_index != -1);

  mat4_t transform = mat4_create(xform);
  mat4_transpose(transform, NULL);

  entity->type = SPRITE;
  entity->transform = transform;
  GLuint texture_id = g_textures[texture_index].id;
  entity->value.sprite.texture_id = texture_id;
}

void create_mesh_entity(Entity* entity, const char* model_name, mat4_t xform) {
  assert(entity);
  assert(model_name);
  assert(xform);

  int mesh_index = -1; 
  for (int i = 0; i < g_mesh_count; ++i) {
    if (strcmp(g_meshes[i].name, model_name) == 0) {
      mesh_index = i;
    }
  }
  assert(mesh_index != -1);

  mat4_t transform = mat4_create(xform);
  mat4_transpose(transform, NULL);
  mat4_t inverse_world = mat4_create(NULL);
  mat4_inverse(transform, inverse_world);

  entity->type = MODEL;
  entity->transform = transform;
  entity->inverse_world = inverse_world;
  entity->value.model.mesh_index = mesh_index;
}

void init_camera(Entity* entity) {
  assert(entity);

  mat4_t transform = mat4_create(NULL);
  mat4_identity(transform);
  entity->type = CAMERA;
  entity->transform = transform;

  g_camera = entity;
}

void new_mesh_entity(Entity* entity, const char* model_name) {
  assert(entity);
  assert(model_name);

  mat4_t identity = mat4_create(NULL);
  mat4_identity(identity);
  create_mesh_entity(entity, model_name, identity);
}

void render_shadow(const Entity* entity) {
  glUseProgram(shadow_program);

  mat4_t view, model, perspective;

  model = entity->transform;
  view = g_camera->transform;
  // TODO: Make one
  perspective = mat4_create(NULL);
  mat4_identity(perspective);
  mat4_perspective(60.0, 1.0, 1.0, 1000.0, perspective);

  mat4_t flatten = mat4_create(NULL);
  mat4_identity(flatten);
  float scale[3] = {1.0f, 1.0f, 0.0f};
  mat4_scale(flatten, scale, NULL);

  GLuint projectionID = glGetUniformLocation(shadow_program, "perspective");
  GLuint modelID = glGetUniformLocation(shadow_program, "model");
  GLuint flattenID = glGetUniformLocation(shadow_program, "flatten");
  GLuint viewID = glGetUniformLocation(shadow_program, "view");
  GLuint heightID = glGetUniformLocation(shadow_program, "height");
  assert(projectionID != -1);
  assert(modelID != -1);
  assert(flattenID != -1);
  assert(viewID != -1);

  glUniformMatrix4fv(projectionID, 1, GL_FALSE, perspective);
  glUniformMatrix4fv(modelID, 1, GL_FALSE, model);
  glUniformMatrix4fv(viewID, 1, GL_FALSE, view);
  glUniformMatrix4fv(flattenID, 1, GL_FALSE, flatten);
  glUniform1f(heightID, -model[14] + 1.0f);

  // Third pass, index array
  size_t stride = 11 * sizeof(float);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  // Get the model
  Mesh* mesh = get_entity_mesh(entity);

  glBindBuffer(GL_ARRAY_BUFFER, mesh->verticesID);
  glVertexPointer(3, GL_FLOAT, stride, 0);
  glNormalPointer(GL_FLOAT, stride, BUFFER_OFFSET(3 * sizeof(float)));
  glTexCoordPointer(2, GL_FLOAT, stride, BUFFER_OFFSET(6 * sizeof(float)));
  glColorPointer(3, GL_FLOAT, stride, BUFFER_OFFSET(8 * sizeof(float)));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indicesID);
  glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, NULL);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
}

//void render_quad(const Entity* entity, int x, int y, int w, int h, bool invert) {
void render_quad(const Entity* entity) {
  assert(entity);
  assert(entity->type == QUAD);
  assert(entity->value.quad.texture_id);

  int x = entity->value.quad.x;
  int y = entity->value.quad.y;
  int w = entity->value.quad.width;
  int h = entity->value.quad.height;

  assert(quad_program);
  glUseProgram(quad_program);

  // bind texture
  GLuint texLoc = glGetUniformLocation(quad_program, "texture");
  assert(texLoc != -1);

  glUniform1i(texLoc, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, entity->value.quad.texture_id);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  float x_normalized = (x - 128.0) / 128.0;
  float y_normalized = (y - 128.0) / 128.0;
  float width_normalized = w / 256.0;
  float height_normalized = h / 256.0;

  float top = 1.0f;
  float bottom = 0.0f;
  /*if (invert) {
    top = 0.0f;
    bottom = 1.0f;
  }*/

  glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0f, bottom); glVertex2f(x_normalized, y_normalized);
    glTexCoord2f(1.0f, bottom); glVertex2f(x_normalized + width_normalized, y_normalized);
    glTexCoord2f(0.0f, top); glVertex2f(x_normalized, y_normalized + height_normalized);
    glTexCoord2f(1.0f, top); glVertex2f(x_normalized + width_normalized,  y_normalized + height_normalized);
  glEnd();
}

void render_model(const Entity* entity, vec3_t out, GLuint mode, GLuint texture) {
  assert(mesh_program);
  glUseProgram(mesh_program);

  mat4_t model, view, perspective;
  model = entity->transform;
  view = g_camera->transform;
  perspective = mat4_create(NULL);
  mat4_identity(perspective);
  mat4_perspective(60.0, 1.0, 1.0, 1000.0, perspective);

  GLuint projectionID = glGetUniformLocation(mesh_program, "perspective");
  GLuint modelID = glGetUniformLocation(mesh_program, "model");
  GLuint viewID = glGetUniformLocation(mesh_program, "view");
  assert(projectionID != -1);
  assert(modelID != -1);
  assert(viewID != -1);

  // bind texture
  /*GLuint texLoc = glGetUniformLocation(mesh_program, "texture");
  assert(texLoc != -1);
  glUniform1i(texLoc, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/

  glUniformMatrix4fv(projectionID, 1, GL_FALSE, perspective);
  glUniformMatrix4fv(modelID, 1, GL_FALSE, model);
  glUniformMatrix4fv(viewID, 1, GL_FALSE, view);

  // Third pass, index array
  size_t stride = 11 * sizeof(float);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  // Get the model
  Mesh* mesh = get_entity_mesh(entity);

  glBindBuffer(GL_ARRAY_BUFFER, mesh->verticesID);
  glVertexPointer(3, GL_FLOAT, stride, 0);
  glNormalPointer(GL_FLOAT, stride, BUFFER_OFFSET(3 * sizeof(float)));
  glTexCoordPointer(2, GL_FLOAT, stride, BUFFER_OFFSET(6 * sizeof(float)));
  glColorPointer(3, GL_FLOAT, stride, BUFFER_OFFSET(8 * sizeof(float)));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indicesID);
  glDrawElements(mode, mesh->num_indices, GL_UNSIGNED_INT, NULL);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
}

void render(const Entity* entity) {
  assert(entity);
  assert(entity->type == MODEL);
  render_model(entity, NULL, GL_TRIANGLES, 0);
}

void render_points(const Entity* entity) {
  assert(entity);
  assert(entity->type == MODEL);
  render_model(entity, NULL, GL_POINTS, 0);
}

void move_to(Entity* entity, float x, float y, float z) {
  assert(entity);

  mat4_t xform = entity->transform;
  assert(xform);
  xform[12] = x; xform[13] = y; xform[14] = z;
}

void translate(Entity* entity, float x, float y, float z) {
  assert(entity);
  mat4_t xform = entity->transform;
  assert(xform);
  float velocity_vector[3] = {x, y, z};
  mat4_translate(xform, velocity_vector, NULL);
}

void scale(Entity* entity, float x, float y, float z) {
  assert(entity);
  mat4_t xform = entity->transform;
  assert(xform);
  float scale[3] = {x, y, z};
  mat4_scale(xform, scale, NULL);
}

void rotate(Entity* entity, float x, float y, float z, float angle) {
  assert(entity);
  mat4_t xform = entity->transform;
  assert(xform);
  float axis[3] = {x, y, z};
  mat4_rotate(xform, angle, axis, NULL);
}

void look_at(Entity* camera, Entity* target) {
  assert(camera->type == CAMERA);
  assert(camera);
  assert(target);

  mat4_t camera_xform = camera->transform;
  mat4_t target_xform = target->transform;

  float eye[3] = {camera_xform[12], camera_xform[13], camera_xform[14]};
  float center[3] = {target_xform[12], target_xform[13], target_xform[14]};
  float up[3] = {0, 0.0f, 1.0f};

  mat4_lookAt(eye, center, up, camera->transform);
}
