#ifndef BIND_H
#define BIND_H

#include "entity.h"

void render(const Entity* entity);
void render_points(const Entity* entity);

void moveTo(Entity* entity, float x, float y, float z);
void translate(Entity* entity, float x, float y, float z);
void scale(Entity* entity, float x, float y, float z);
void rotate(Entity* entity, float x, float y, float z, float angle);

void lookAt(Entity* camera, Entity* target);

// Utils
void loadLevel(const char* level);
void setBackground(const char* bg);
void setBackgroundClearColor(short r, short g, short b);

#endif
