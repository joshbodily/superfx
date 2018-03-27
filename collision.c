#include "collision.h"

//int tri_tri_intersect(float V0[3],float V1[3],float V2[3], float U0[3],float U1[3],float U2[3]);
//int collide(Model* arwing, Model* object, vec3_t normal);

//if (collide(&arwing->value.model, &entities[4].value.model, normal)) {
int collide(Entity* arwing, Entity* object, vec3_t normal) {
  /*static float inverse_object_arwing[16];
  const int STRIDE = 11;

  mat4_identity(inverse_object_arwing);
  mat4_t arwing_xform = arwing->node->transform;
  mat4_t object_inverse = object->node->inverse_world;
  mat4_multiply(object_inverse, arwing_xform, inverse_object_arwing);

  // TODO: Replace w/ model def
  //ModelDef* arwing_mesh = &models[arwing->modelIndex];
  //ModelDef* object_mesh = &models[object->modelIndex];

  //for (int i = 0; i < 
  float arwing1[3] = { -0.762492, -0.313925, 0.985007 };
  float arwing2[3] = { 0.001113, -0.050108, -0.985007 };
  float arwing3[3] = { 0.762492, -0.313925, 0.985007 };
  mat4_multiplyVec3(inverse_object_arwing, arwing1, NULL);
  mat4_multiplyVec3(inverse_object_arwing, arwing2, NULL);
  mat4_multiplyVec3(inverse_object_arwing, arwing3, NULL);
  for (int i = 0; i < object_mesh->num_indices; i += 3) {
    float* vertex1 = &object_mesh->vertices[ object_mesh->indices[i] * STRIDE ];
    float* vertex2 = &object_mesh->vertices[ object_mesh->indices[i + 1] * STRIDE ];
    float* vertex3 = &object_mesh->vertices[ object_mesh->indices[i + 2] * STRIDE ];
    if (tri_tri_intersect(arwing1, arwing2, arwing3, vertex1, vertex2, vertex3)) {
      normal[0] = object_mesh->vertices[ object_mesh->indices[i] * STRIDE ];
      normal[1] = object_mesh->vertices[ object_mesh->indices[i] * STRIDE + 1 ];
      normal[2] = object_mesh->vertices[ object_mesh->indices[i] * STRIDE + 2 ];
      return 1;
    }
  }*/
  return 0;
  // arwing engine triangle
// = {0.001113, -0.372016, 0.122996};
// = {-0.215379, -0.254635, -0.018131};
// = {0.219197, -0.253044, -0.018172};
}
