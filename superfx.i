%module SuperFX
%{
/* Includes the header in the wrapper code */
#include "entity.h"
#include "parse.h"
#include "bind.h"
%}

/* Parse the header file to generate wrappers */
%include "entity.h"
%include "bind.h"

void new_mesh_entity(Entity* entity, const char* modelName);
void new_camera(Entity* entity);
