#include "bind.h"
#include "resources.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define m(r,c) (entity->transform[c * 4 + r])

// Camera
Entity* g_camera;
float g_mat_perspective[16];
float g_mat_shadow[16];

void add_child(Entity* parent, Entity* child) {
  parent->child = child;
  child->parent = parent;
}

void set_background(const char* texture_name) {
  assert(texture_name);
  Texture* texture = get_texture(texture_name);
  assert(texture);
  background_entity.value.quad.texture_id = texture->id;
}

Size new_quad_entity(Entity* entity, const char* texture_name) {
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
  Size s;
  s.width = g_textures[texture_index].width;
  s.height = g_textures[texture_index].height;

  entity->value.quad.index = 0;
  entity->value.quad.rows = 1;
  entity->value.quad.columns = 1;

  entity->type = QUAD;
  entity->transform[0] = 1.0;
  entity->transform[5] = 1.0;
  GLuint texture_id = g_textures[texture_index].id;
  entity->value.quad.texture_id = texture_id;

  return s;
}

void set_quad_index(Entity* entity, int index) {
  assert(entity);
  assert(entity->type == QUAD || entity->type == SPRITE);
  entity->value.quad.index = index; 
}

Size new_text_quad_entity(Entity* entity, const char* text, int size, const char* font) {
  assert(entity);
  assert(text);

  mat4_identity(entity->transform);
  mat4_identity(entity->inverse_world);

  GLuint texture = 0;
  glGenTextures(1, &texture);
  SDL_Color color = {255, 255, 255};
  int width, height;
  create_text(font, size, text, color, texture, &width, &height);

  entity->value.quad.index = 0;
  entity->value.quad.rows = 1;
  entity->value.quad.columns = 1;

  entity->type = QUAD;
  entity->transform[0] = 1.0;
  entity->transform[5] = 1.0;
  entity->value.quad.texture_id = texture;

  Size s;
  s.width = width;
  s.height = height;
  return s;
}

void new_sprite_entity(Entity* entity, const char* texture_name, int rows, int columns) {
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
  entity->value.sprite.rows = rows;
  entity->value.sprite.columns = columns;

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

  int mesh_index = get_mesh_index(model_name);
  assert(mesh_index != -1 && "Could not find model");

  entity->type = MODEL;
  entity->value.model.texture_id = -1;
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
  assert(mesh);

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

  // TODO: Stop hardcoding screen sizes !!!
  float x_normalized = (x - 512.0f) / 512.0f;
  float y_normalized = (y - 365.0f) / 365.0f;
  float width_normalized = w / 365.0f;
  float height_normalized = h / 365.0f;

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

void render_model(const Entity* entity, vec3_t out, GLuint mode) {
  //assert(mesh_program);
  GLuint program;
  GLuint texture = entity->value.model.texture_id;
  if (texture == -1)
    program = color_program;
  else
    program = texture_program;
  glUseProgram(program);

  GLuint projectionID = glGetUniformLocation(program, "perspective");
  GLuint modelID = glGetUniformLocation(program, "model");
  GLuint viewID = glGetUniformLocation(program, "view");

  assert(projectionID != -1);
  assert(modelID != -1);
  assert(viewID != -1);
  assert(g_mat_perspective);
  assert(entity->transform);
  assert(g_camera->transform);

  //mat4_t camera_xform = mat4_multiply(g_camera->parent->transform, g_camera->transform, NULL);

  // bind texture
  if (texture != -1) {
    GLuint texLoc = glGetUniformLocation(program, "texture");
    //assert(texLoc != -1);
    glUniform1i(texLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, entity->value.model.texture_id);
    //glBindTexture(GL_TEXTURE_2D, 6);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

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
  assert(mesh);

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
  render_model(entity, NULL, GL_TRIANGLES);
}

void render_points(const Entity* entity) {
  assert(entity);
  assert(entity->type == MODEL);
  render_model(entity, NULL, GL_POINTS);
}

Vec3 get_location(Entity* entity) {
  assert(entity);
  Vec3 ret;
  ret.x = entity->transform[12];
  ret.y = entity->transform[13];
  ret.z = entity->transform[14];
  return ret;
}

//void print_orientation(Entity* entity) {
void print_rotation(Entity* entity) {
  /*assert(entity); 
  entity->transform;
  float quat[4];
  float w = sqrt(1.0 + m(0,0) + m(1,1) + m(2,2)) / 2.0;
  double w4 = (4.0 * w);
  float x = (m(2,1) - m(1,2)) / w4;
  float y = (m(0,2) - m(2,0)) / w4;
  float z = (m(1,0) - m(0,1)) / w4;
  printf("%f %f %f %f\n", x, y, z, w);*/
  assert(entity);
  float t = m(0,0) + m(1,1) + m(2,2);
  float r = sqrt(1.0 + t);
  float w = 0.5f * r;
  float x = copysign(0.5f * sqrt(1.0f + m(0,0) - m(1,1) - m(2,2)), m(2,1) - m(1,2));
  float y = copysign(0.5f * sqrt(1.0f - m(0,0) + m(1,1) - m(2,2)), m(0,2) - m(2,0));
  float z = copysign(0.5f * sqrt(1.0f - m(0,0) - m(1,1) + m(2,2)), m(1,0) - m(0,1));
  printf("{%f %f %f %f %f %f %f},\n", x, y, z, w, entity->transform[12], entity->transform[13], entity->transform[14]);
}

Quat slerp(float x0, float y0, float z0, float w0, float x1, float y1, float z1, float w1, float t) {
  float out[4]; 
  float in1[4] = {x0, y0, z0, w0};
  float in2[4] = {x1, y1, z1, w1};
  quat_slerp(in1, in2, t, out);
  Quat ret;
  ret.x = out[0];
  ret.y = out[1];
  ret.z = out[2];
  ret.w = out[3];
  return ret;
}

void rotateQuat(Entity* entity, float x, float y, float z, float w) {
  //quat_t quat_toMat4(quat_t quat, mat4_t dest) {
  float quat[4] = {x, y, z, w};
  quat_toMat4(quat, entity->transform);
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
  float forward[3] = {xform[2], xform[6], xform[10]};
  printf("%f %f %f\n", forward[0], forward[1], forward[2]);
  vec3_normalize(forward, NULL);
  vec3_scale(forward, z, NULL);
  mat4_translate(xform, forward, NULL);
}

void forward(Entity* entity, float magnitude) {
  assert(entity);
  mat4_t xform = entity->transform;
  float forward[3] = {xform[2], xform[6], xform[10]};
  vec3_normalize(forward, NULL);
  vec3_scale(forward, magnitude, NULL);
  mat4_translate(xform, forward, NULL);
}

//Forward = glm::normalize(Vec3(ViewMatrix[0][2], ViewMatrix[1][2], ViewMatrix[2][2]));
//Right = glm::normalize(Vec3(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]));
//Up = glm::normalize(Vec3(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]));
void strafe(Entity* entity, float magnitude) {
  assert(entity);
  mat4_t xform = entity->transform;
  float right[3] = {xform[0], xform[4], xform[8]};
  vec3_normalize(right, NULL);
  vec3_scale(right, magnitude, NULL);
  mat4_translate(xform, right, NULL);
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

void set_scale(Entity* entity, float x, float y, float z) {
  assert(entity);
  mat4_t xform = entity->transform;
  assert(xform);
  xform[0] = x;
  xform[5] = y;
  xform[10] = z;
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

void rotateXYZ(Entity* entity, float x, float y, float z) {
  assert(entity);
  mat4_t xform = entity->transform;
  assert(xform);
  mat4_rotateX(xform, x, NULL);
  mat4_rotateY(xform, y, NULL);
  mat4_rotateZ(xform, z, NULL);
}

void look_at_pos(Entity* camera, vec3_t position) { 
  assert(camera->type == CAMERA);

  mat4_t camera_xform = camera->transform;

  float eye[3] = {0.0f, 0.0f, 0.0f};
  mat4_t inverse = mat4_inverse(camera_xform, NULL);
  mat4_multiplyVec3(inverse, eye, NULL);

  printf("%f %f %f\n", eye[0], eye[1], eye[2]);
  float up[3] = {0.0f, 1.0f, 0.0f};

  mat4_lookAt(eye, position, up, camera->transform);
}

/*void look_at_entity(Entity* camera, Entity* target) {
  assert(camera->type == CAMERA);
  assert(camera);
  assert(target);

  mat4_t camera_xform = camera->transform;
  mat4_t target_xform = target->transform;

  float eye[3] = {0.0f, 0.0f, 0.0f};
  float center[3] = {target_xform[12], target_xform[13], target_xform[14]};
  float up[3] = {0.0f, 1.0f, 0.0f};

  mat4_lookAt(eye, center, up, camera->transform);
}*/

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
  float arwing1[3] = {-0.762492, -0.985007, -0.313925};
  float arwing2[3] = {0.001113,  0.985007, -0.050108};
  float arwing3[3] = {0.762492, -0.985007, -0.313925};
  mat4_multiplyVec3(inverse_world, arwing1, NULL);
  mat4_multiplyVec3(inverse_world, arwing2, NULL);
  mat4_multiplyVec3(inverse_world, arwing3, NULL);

  for (int i = 0; i < object_mesh->num_indices; i += 3) {
    float* vertex1 = &object_mesh->vertices[ object_mesh->indices[i] * STRIDE ];
    float* vertex2 = &object_mesh->vertices[ object_mesh->indices[i + 1] * STRIDE ];
    float* vertex3 = &object_mesh->vertices[ object_mesh->indices[i + 2] * STRIDE ];
    //if (tri_tri_intersect(arwing1, arwing2, arwing3, vertex1, vertex2, vertex3)) {
      //normal[0] = object_mesh->vertices[ object_mesh->indices[i] * STRIDE ];
      //normal[1] = object_mesh->vertices[ object_mesh->indices[i] * STRIDE + 1 ];
      //normal[2] = object_mesh->vertices[ object_mesh->indices[i] * STRIDE + 2 ];
      printf("colliding\n");
      return true;
    //}
  }
  return false;
  // arwing engine triangle
  // = {0.001113, -0.372016, 0.122996};
  // = {-0.215379, -0.254635, -0.018131};
  // = {0.219197, -0.253044, -0.018172};
}
