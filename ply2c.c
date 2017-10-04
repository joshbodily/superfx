#include "stdio.h"
#include "stdlib.h"
#include <assert.h>

void loadModel(const char* path) {
  FILE * fp;
  FILE * wp;
  char * line = NULL;
  size_t len = 0;

  int vertices = 0;
  int faces = 0;

  fp = fopen(path, "r");
  wp = fopen("model.h", "w+");

  // Parse header
  int parse = 0;
  while (getline(&line, &len, fp) != -1) {
    int temp = 0;
    int count = sscanf(line, "element vertex %d", &temp);
    if (count == 1) { vertices = temp; }
    count = sscanf(line, "element face %d", &temp);
    if (count == 1) { faces = temp; }
    if (strcmp(line, "end_header\n") == 0) { 
      break; 
    }
  }

  // Parse vertices
  fprintf(wp, "GLfloat data[] = {\n");
  float x, y, z, nx, ny, nz, u, v, r, g, b;
  for (int i = 0; i < vertices; ++i) {
    getline(&line, &len, fp);
    size_t count = sscanf(line, "%f %f %f %f %f %f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz, &u, &v, &r, &g, &b);
    assert(count == 11);
    fprintf(wp, "  %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f,\n", x, y, z, nx, ny, nz, u, v, r/255.0f, g/255.0f, b/255.0f);
  }
  fprintf(wp, "};\n");

  // Parse faces
  fprintf(wp, "const GLuint indices[] = {\n");
  int f1, f2, f3;
  for (int i = 0; i < faces; ++i) {
    getline(&line, &len, fp);
    size_t count = sscanf(line, "3 %d %d %d", &f1, &f2, &f3);
    assert(count == 3);
    fprintf(wp, "  %d, %d, %d,\n", f1, f2, f3);
  }
  fprintf(wp, "};\n");

  fclose(fp);
}

int main(int argc, const char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: ply2c [filename]\n");
    return 2;
  }
  loadModel(argv[1]);
  return 0;
}
