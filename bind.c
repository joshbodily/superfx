#include "bind.h"
#include "resources.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

// Camera
Entity* g_camera;
float g_mat_perspective[16];
float g_mat_shadow[16];

void new_quad_entity(Entity* entity, const char* texture_name) {
  assert(entity);
  assert(texture_name);

  mat4_identity(entity->transform);
  mat4_identity(entity->inverse_world);

  int texture_index = -1; 
  for (int i = 0; i < g_texture_count; ++i) {
    if (strcmp(g_textures[i].name, texture_name) == 0) {
      texture_index = i;
    }
  }
  assert(texture_index != -1 && "Could not find texture");

  entity->value.quad.index = 0;
  entity->value.quad.rows = 2;
  entity->value.quad.columns = 2;

  entity->type = QUAD;
  GLuint texture_id = g_textures[texture_index].id;
  entity->value.quad.texture_id = texture_id;
}

void set_quad_index(Entity* entity, int index) {
  assert(entity);
  assert(entity->type == QUAD || entity->type == SPRITE);
  entity->value.quad.index = index; 
}

void new_text_quad_entity(Entity* entity, const char* text) {
  assert(entity);
  assert(text);

  mat4_identity(entity->transform);
  mat4_identity(entity->inverse_world);

  GLuint texture = 0;
  glGenTextures(1, &texture);
  SDL_Color color = {255, 255, 255};
  int width, height;
  create_text("fonts/MEGAMAN10.ttf", 32, text, color, texture, &width, &height);

  entity->value.quad.index = 0;
  entity->value.quad.rows = 1;
  entity->value.quad.columns = 1;

  entity->type = QUAD;
  entity->transform[0] = width;
  entity->transform[5] = height;
  entity->value.quad.texture_id = texture;
}

void new_sprite_entity(Entity* entity, const char* texture_name) {
  assert(entity);
  assert(texture_name);

  mat4_identity(entity->transform);
  mat4_identity(entity->inverse_world);

  int texture_index = -1; 
  for (int i = 0; i < g_texture_count; ++i) {
    if (strcmp(g_textures[i].name, texture_name) == 0) {
      texture_index = i;
    }
  }
  assert(texture_index != -1 && "Could not find texture");

  entity->type = SPRITE;
  entity->value.sprite.index = 0;
  entity->value.sprite.rows = 1;
  entity->value.sprite.columns = 12;

  GLuint texture_id = g_textures[texture_index].id;
  entity->value.sprite.texture_id = texture_id;
}

void init_camera(Entity* entity) {
  assert(entity);

  // TODO: Move?
  mat4_identity(g_mat_perspective);
  mat4_perspective(60.0, 1.0, 1.0, 1000.0, g_mat_perspective);

  // TODO: Move?
  mat4_identity(g_mat_shadow);
  float scale[3] = {1.0f, 1.0f, 0.0f};
  mat4_scale(g_mat_shadow, scale, NULL);

  mat4_identity(entity->transform);
  entity->type = CAMERA;
  g_camera = entity;
}

void new_mesh_entity(Entity* entity, const char* model_name) {
  assert(entity);
  assert(model_name);

  mat4_identity(entity->transform);
  mat4_identity(entity->inverse_world);

  int mesh_index = -1; 
  for (int i = 0; i < g_mesh_count; ++i) {
    if (strcmp(g_meshes[i].name, model_name) == 0) {
      mesh_index = i;
    }
  }
  assert(mesh_index != -1 && "Could not find model");

  entity->type = MODEL;
  entity->value.model.mesh_index = mesh_index;
}

void render_shadow(const Entity* entity) {
  glUseProgram(shadow_program);


  GLuint projectionID = glGetUniformLocation(shadow_program, "perspective");
  GLuint modelID = glGetUniformLocation(shadow_program, "model");
  GLuint flattenID = glGetUniformLocation(shadow_program, "flatten");
  GLuint viewID = glGetUniformLocation(shadow_program, "view");
  GLuint heightID = glGetUniformLocation(shadow_program, "height");
  assert(projectionID != -1);
  assert(modelID != -1);
  assert(flattenID != -1);
  assert(viewID != -1);

  glUniformMatrix4fv(projectionID, 1, GL_FALSE, g_mat_perspective);
  //glUniformMatrix4fv(modelID, 1, GL_FALSE, model);
  glUniformMatrix4fv(modelID, 1, GL_FALSE, entity->transform);
  glUniformMatrix4fv(viewID, 1, GL_FALSE, g_camera->transform);
  glUniformMatrix4fv(flattenID, 1, GL_FALSE, g_mat_shadow);
  glUniform1f(heightID, -entity->transform[14] + 1.0f);

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

void render_sprite(const Entity* entity) {
  assert(sprite_program);
  assert(entity->type == SPRITE);
  assert(entity->value.sprite.texture_id);

  glUseProgram(sprite_program);

  GLuint projectionID = glGetUniformLocation(sprite_program, "perspective");
  GLuint modelID = glGetUniformLocation(sprite_program, "model");
  GLuint viewID = glGetUniformLocation(sprite_program, "view");
  assert(projectionID != -1);
  assert(modelID != -1);
  assert(viewID != -1);

  glUniformMatrix4fv(projectionID, 1, GL_FALSE, g_mat_perspective);
  glUniformMatrix4fv(modelID, 1, GL_FALSE, entity->transform);
  glUniformMatrix4fv(viewID, 1, GL_FALSE, g_camera->transform);

  GLuint texLoc = glGetUniformLocation(sprite_program, "texture");
  assert(texLoc != -1);
  glUniform1i(texLoc, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, entity->value.sprite.texture_id);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  int index = entity->value.sprite.index % (entity->value.sprite.columns * entity->value.sprite.rows);
  float u1 = (index % entity->value.sprite.columns) / (float)entity->value.sprite.columns;
  float u2 = u1 + (1.0f / (float)entity->value.sprite.columns);
  float v2 = 1.0f; //1.0f - (index / entity->value.quad.columns) / (float)entity->value.quad.rows;
  float v1 = 0.0f; //1.0f - v2 + (1.0f / (float)entity->value.sprite.rows);

  glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(u2, v2); glVertex3f(0.5f, 0.0f, 0.5f); // top right
    glTexCoord2f(u1, v2); glVertex3f(-0.5f, 0.0f, 0.5f); // top left
    glTexCoord2f(u2, v1); glVertex3f(0.5f, 0.0f, -0.5f); // bottom right
    glTexCoord2f(u1, v1); glVertex3f(-0.5f, 0.0f, -0.5f); // bottom left
  glEnd();
}

void render_quad(const Entity* entity, bool invert) {
  glClear(GL_DEPTH_BUFFER_BIT);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  assert(entity);
  assert(entity->type == QUAD);
  assert(entity->value.quad.texture_id);
  assert(entity->value.quad.columns > 0);

  int x = entity->transform[12];
  int y = entity->transform[13];
  int w = entity->transform[0];
  int h = entity->transform[5];

  float u1 = (entity->value.quad.index % entity->value.quad.columns) / (float)entity->value.quad.columns;
  float u2 = u1 + (1.0f / (float)entity->value.quad.columns);

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
  if (invert) {
    top = 0.0f;
    bottom = 1.0f;
  }

  glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(u1, bottom); glVertex2f(x_normalized, y_normalized); // bottom left
    glTexCoord2f(u2, bottom); glVertex2f(x_normalized + width_normalized, y_normalized); // bottom right
    glTexCoord2f(u1, top); glVertex2f(x_normalized, y_normalized + height_normalized); // top left
    glTexCoord2f(u2, top); glVertex2f(x_normalized + width_normalized,  y_normalized + height_normalized); // top right
  glEnd();

  /*glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0f, bottom); glVertex2f(-1, -1);
    glTexCoord2f(1.0f, bottom); glVertex2f(1, -1);
    glTexCoord2f(0.0f, top); glVertex2f(-1, 1);
    glTexCoord2f(1.0f, top); glVertex2f(1, 1);
  glEnd();*/
}

void render_model(const Entity* entity, vec3_t out, GLuint mode, GLuint texture) {
  assert(mesh_program);
  glUseProgram(mesh_program);

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

  glUniformMatrix4fv(projectionID, 1, GL_FALSE, g_mat_perspective);
  glUniformMatrix4fv(modelID, 1, GL_FALSE, entity->transform);
  glUniformMatrix4fv(viewID, 1, GL_FALSE, g_camera->transform);

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

Vec3 get_location(Entity* entity) {
  assert(entity);
  Vec3 ret;
  ret.x = entity->transform[12];
  ret.y = entity->transform[13];
  ret.z = entity->transform[14];
  return ret;
}

void move_to(Entity* entity, float x, float y, float z) {
  assert(entity);

  mat4_t xform = entity->transform;
  assert(xform);
  xform[12] = x; xform[13] = y; xform[14] = z;

  mat4_inverse(entity->transform, entity->inverse_world);
}

void identity(Entity* entity) {
  mat4_identity(entity->transform);
}

void set_translation_world(Entity* entity, float x, float y, float z) {
  assert(entity);
  mat4_t xform = entity->transform;
  assert(xform);
  xform[12] = x;
  xform[13] = y;
  xform[14] = z;
}

void translate(Entity* entity, float x, float y, float z) {
  assert(entity);
  mat4_t xform = entity->transform;
  assert(xform);
  float velocity_vector[3] = {x, y, z};
  mat4_translate(xform, velocity_vector, NULL);
}

void set_transform(Entity* src, Entity* dst) {
  assert(src);
  assert(dst);
  mat4_set(src->transform, dst->transform);
}

Vec3 transform(Entity* entity, float x, float y, float z) {
  assert(entity);
  mat4_t xform = entity->transform;
  assert(xform);
  float vec[3] = {x, y, z};
  mat4_multiplyVec3(xform, vec, NULL);
  Vec3 ret;
  ret.x = vec[0];
  ret.y = vec[1];
  ret.z = vec[2];
  return ret;
}

void scale(Entity* entity, float x, float y, float z) {
  assert(entity);
  mat4_t xform = entity->transform;
  assert(xform);
  float scale[3] = {x, y, z};
  mat4_scale(xform, scale, NULL);
  mat4_inverse(entity->transform, entity->inverse_world);
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

Input get_input() {
  return g_input;
}

bool collide(Entity* dynamic_object, Entity* object) { //, vec3_t normal) {
  assert(dynamic_object->type == MODEL);
  assert(object->type == MODEL);

  static float inverse_world[16];
  const int STRIDE = 11;

  mat4_identity(inverse_world);
  mat4_t dynamic_object_transform = dynamic_object->transform;
  mat4_t object_inverse = object->inverse_world;
  mat4_multiply(object_inverse, dynamic_object_transform, inverse_world);

  //Mesh* arwing_mesh = &models[arwing->modelIndex];
  Mesh* object_mesh = get_entity_mesh(object);
  assert(object_mesh);

  // for (int i = 0; i < ...) { TODO
  float arwing1[3] = { -0.762492, -0.313925, 0.985007 };
  float arwing2[3] = { 0.001113, -0.050108, -0.985007 };
  float arwing3[3] = { 0.762492, -0.313925, 0.985007 };
  mat4_multiplyVec3(inverse_world, arwing1, NULL);
  mat4_multiplyVec3(inverse_world, arwing2, NULL);
  mat4_multiplyVec3(inverse_world, arwing3, NULL);

  for (int i = 0; i < object_mesh->num_indices; i += 3) {
    float* vertex1 = &object_mesh->vertices[ object_mesh->indices[i] * STRIDE ];
    float* vertex2 = &object_mesh->vertices[ object_mesh->indices[i + 1] * STRIDE ];
    float* vertex3 = &object_mesh->vertices[ object_mesh->indices[i + 2] * STRIDE ];
    if (tri_tri_intersect(arwing1, arwing2, arwing3, vertex1, vertex2, vertex3)) {
      //normal[0] = object_mesh->vertices[ object_mesh->indices[i] * STRIDE ];
      //normal[1] = object_mesh->vertices[ object_mesh->indices[i] * STRIDE + 1 ];
      //normal[2] = object_mesh->vertices[ object_mesh->indices[i] * STRIDE + 2 ];
      printf("colliding\n");
      return true;
    }
  }
  return false;
  // arwing engine triangle
  // = {0.001113, -0.372016, 0.122996};
  // = {-0.215379, -0.254635, -0.018131};
  // = {0.219197, -0.253044, -0.018172};
}
