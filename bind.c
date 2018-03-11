#include "bind.h"
#include "parse.h"
#include "utils.h"
#include "console.h"
#include <assert.h>
#include <stdlib.h>
#include <lua5.1/lualib.h>
#include <lua5.1/lauxlib.h>

#define PI_2 3.141592 * 0.5
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

// Camera
Entity* g_camera;
extern GLuint modelProgram;

void renderModel(const Entity* entity, vec3_t out, GLuint mode, GLuint texture) {
  // Pass data to shaders
  glUseProgram(modelProgram);
  //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

  mat4_t view;
  mat4_t model, temp;
  mat4_t perspective;
  perspective = mat4_create(NULL);
  model = mat4_create(NULL);
  temp = mat4_create(NULL);
  view = g_camera->transform;

  // perspective
  mat4_identity(perspective);
  mat4_perspective(60.0, 1.0, 1.0, 1000.0, perspective);

  model = entity->transform;
  // rotate model
  //mat4_identity(model);
  //mat4_identity(temp);
  //mat4_rotateY(temp, draw->yaw, temp);
  //mat4_rotateX(temp, draw->pitch, temp);
  //if (out != NULL) {
    //mat4_multiplyVec3(model, out, NULL);
  //}
  //float translate_model[3] = {draw->x, draw->y, draw->z};
  //mat4_translate(temp, translate_model, model);*/

  GLuint projectionID = glGetUniformLocation(modelProgram, "perspective");
  GLuint modelID = glGetUniformLocation(modelProgram, "model");
  GLuint viewID = glGetUniformLocation(modelProgram, "view");
  assert(projectionID != -1);
  assert(modelID != -1);
  assert(viewID != -1);

  // bind texture
  /*GLuint texLoc = glGetUniformLocation(modelProgram, "texture");
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
  Mesh* mesh = get_model_mesh(entity);

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
  assert(entity->type == MODEL);
  renderModel(entity, NULL, GL_TRIANGLES, 0);
}

void render_points(const Entity* entity) {
  assert(entity->type == MODEL);
  renderModel(entity, NULL, GL_POINTS, 0);
}

void moveTo(Entity* entity, float x, float y, float z) {
  assert(entity);
  mat4_t xform = entity->transform;
  assert(xform);
  xform[12] = x; xform[13] = y; xform[14] = z;
}

void translate(Entity* entity, float x, float y, float z) {
  mat4_t xform = entity->transform;
  assert(xform);
  float velocity_vector[3] = {x, y, z};
  mat4_translate(xform, velocity_vector, NULL);
}

void scale(Entity* entity, float x, float y, float z) {
  float scale[3] = {x, y, z};
  mat4_scale(entity->transform, scale, NULL);
}

void rotate(Entity* entity, float x, float y, float z, float angle) {
  float axis[3] = {x, y, z};
  // mat4_t mat4_rotate(mat4_t mat, float angle, vec3_t axis, mat4_t dest) {
  mat4_rotate(entity->transform, angle, axis, NULL);
}

void lookAt(Entity* camera, Entity* target) {
  assert(camera->type == CAMERA);
  assert(target);

  mat4_t camera_xform = camera->transform;
  mat4_t target_xform = target->transform;

  //printf("%f %f %f\n", camera_xform[12], camera_xform[13], camera_xform[14]);
  //printf("%f %f %f\n", target_xform[12], target_xform[13], target_xform[14]);

  //float eye[3] = {3, 5, 7}; //camera_xform[12], camera_xform[13], camera_xform[14]};
  float eye[3] = {camera_xform[12], camera_xform[13], camera_xform[14]};
  float center[3] = {target_xform[12], target_xform[13], target_xform[14]};
  float up[3] = {0, 0.0f, 1.0f};

  mat4_lookAt(eye, center, up, camera->transform);
}

// Utils
void loadLevel(const char* c) {
  SDL_Log("Loading level %s", c);
  int length;
  char* buffer = get_file_data(c, &length);
  if (buffer == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load file `%s'", c);
    return;
  }
	int error;
  error = luaL_loadbuffer(L, buffer, length, "loadLevel"); 
  if (error) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", lua_tostring(L, -1));
    lua_pop(L, 1);
  } else {
    error = lua_pcall(L, 0, 0, 0);
    if (error) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", lua_tostring(L, -1));
      lua_pop(L, 1);
    }
  }
}
