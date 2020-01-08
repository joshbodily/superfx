#include <stdio.h>
#include <math.h>
#include <stdbool.h>

typedef struct Vec3 {
  float x, y, z;
} Vec3;

float dot(const Vec3* a, const Vec3* b) {
  return a->x * b->x + a->y * b->y + a->z * b->z; 
}

Vec3 subtract(const Vec3* a, const Vec3* b) {
  Vec3 result;
  result.x = a->x - b->x;
  result.y = a->y - b->y;
  result.z = a->z - b->z;
  return result;
}

Vec3 add(const Vec3* a, const Vec3* b) {
  Vec3 result;
  result.x = a->x + b->x;
  result.y = a->y + b->y;
  result.z = a->z + b->z;
  return result;
}

Vec3 scale(float scalar, const Vec3* a) {
  Vec3 result;
  result.x = a->x * scalar;
  result.y = a->y * scalar;
  result.z = a->z * scalar;
  return result;
}

Vec3 normalize(const Vec3* a) {
  Vec3 result;
  float len = sqrt(a->x * a->x + a->y * a->y + a->z * a->z);
  result.x = a->x / len;
  result.y = a->y / len;
  result.z = a->z / len;
  return result;
}

int main() {
  // p0 = point in plane
  // n = normal
  // l = vector in direction of line
  // l0 = point on line
  // d = (p0 - l0) . n / ( l . n )
  // point on plane = dl + l0
  
  // Plane
  Vec3 p0 = {0.0f, 0.0f, 0.0f};
  Vec3 n  = {0.0f, 0.0f, 1.0f};

  //float h = sqrt(2.0f);
  Vec3 l0 = {0.0f, 0.0f, 1.0f};
  int index = 0;
  while (true) {
    //Vec3 dir  = {0.31f, 0.05f, -1.0f};
    Vec3 dir;
    dir.x = cos(0.01f * index * M_PI) * 1.0f; 
    dir.y = sin(0.01f * index * M_PI) * 1.0f; 
    dir.z = -1.0f;
    Vec3 l = normalize(&dir); 

    Vec3 diff = subtract(&p0, &l0);
    float d = dot(&diff, &n) / dot(&l, &n);
    //printf("d: %f\n", d);

    Vec3 scaled = scale(d, &l);
    Vec3 intersect = add(&scaled, &l0);
    printf("d: %f intersection: (%f, %f, %f)\n", d, intersect.x, intersect.y, intersect.z);

    index++;
  }
}
