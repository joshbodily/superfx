#include "stdio.h"
#include "stdlib.h"

typedef struct {
  // VBOI data
  int index;
  // just for lookup 
  int vertex_index; 
  int normal_index;
  int uv_index;
  // VBO data
  float x, y, z;
  float u, v;
  float nx, ny, nz;
} VertexDef;

VertexDef* vertex_defs;
float* vertices;
float* uvs;
float* normals;
int normal = 0, vertex = 0, uv = 0;
int vertexIndex = 0; 

void printVertexDef(const VertexDef* def) {
  printf("  %f, \t%f, \t%f, \t%f, \t%f, \t%f, \t%f, \t%f,\n", def->x, def->y, def->z, def->u, def->v, def->nx, def->ny, def->nz);
}

int findVertexDef(int vi, int ui, int ni) {
  for (int i = 0; i < uv; ++i) {
    //printf("HERE %d %d %d => %d %d %d?\n", vi, ui, ni, vertex_defs[i].vertex_index, vertex_defs[i].uv_index, vertex_defs[i].normal_index);
    if (vertex_defs[i].vertex_index == vi && vertex_defs[i].normal_index == ni && vertex_defs[i].uv_index == ui) {
      return i;
    }
  }
  return -1;
}

void findOrCreateVertex(int v1, int u1, int n1) {
  int tempIndex;
  if ((tempIndex = findVertexDef(v1, u1, n1)) != -1) {
    printf("%d, ", tempIndex);
  } else {
    printf("%d, ", vertexIndex);
    VertexDef* def = &(vertex_defs[vertexIndex]);
    def->x = vertices[3 * v1];
    def->y = vertices[3 * v1 + 1];
    def->z = vertices[3 * v1 + 2]; 
    def->u = uvs[2 * u1];
    def->v = uvs[2 * u1 + 1]; 
    def->nx = normals[3 * n1];
    def->ny = normals[3 * n1 + 1];
    def->nz = normals[3 * n1 + 2];
    def->vertex_index = v1;
    def->uv_index = u1;
    def->normal_index = n1;
    def->index = vertexIndex++;
  }
}

void loadModel(const char* path) {
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  float x,y,z;
  float nx,ny,nz;
  float u,v;
  int v1,u1,n1, v2,u2,n2, v3,u3,n3;

  fp = fopen(path, "r");

  while ((read = getline(&line, &len, fp)) != -1) {
    if (line[0] == 'v' && line[1] == 'n') { normal++; }
    else if (line[0] == 'v' && line[1] == 't') { uv++; }
    else if (line[0] == 'v') { vertex++; }
    //else if (line[0] == 'f') { 
  }
  rewind(fp);

  //printf("Found %d normals, %d textures, %d vertices\n", normal, uv, vertex); 
  vertices = (float *)malloc(vertex * sizeof(float) * 3);
  uvs = (float *)malloc(uv * sizeof(float) * 2);
  normals = (float *)malloc(normal * sizeof(float) * 3);

  vertex_defs = (VertexDef *)malloc(uv * sizeof(VertexDef));

  normal = 0; vertex = 0; uv = 0;

  // Preamble
  printf("const GLuint indices[] = {\n");
  while ((read = getline(&line, &len, fp)) != -1) {
    // normals
    if (line[0] == 'v' && line[1] == 'n') {
      int items = sscanf(line, "vn %g %g %g", &nx, &ny, &nz);
      normals[3 * normal] = nx;
      normals[3 * normal + 1] = ny;
      normals[3 * normal + 2] = nz;
      //printf("Normal %d: %f, %f, %f,\n", normal, vertices[i * normal]
      normal++;
    }
    // textures
    else if (line[0] == 'v' && line[1] == 't') {
      int items = sscanf(line, "vt %g %g", &u, &v);
      //printf("UV %d: %f, %f\n", uv++, u, v);
      uvs[2 * uv] = u;
      uvs[2 * uv + 1] = v;
      uv++;
    }
    // vertices
    else if (line[0] == 'v') {
      int items = sscanf(line, "v %g %g %g", &x, &y, &z);
      printf("Vertex %d: %f, %f, %f,\n", vertex, x, y, z);
      vertices[3 * vertex] = x;
      vertices[3 * vertex + 1] = y;
      vertices[3 * vertex + 2] = z;
      vertex++;
    }
    // faces
    else if (line[0] == 'f') {
      int items = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &u1, &n1, &v2, &u2, &n2, &v3, &u3, &n3);
      findOrCreateVertex(v1 - 1, u1 - 1, n1 - 1);
      findOrCreateVertex(v2 - 1, u2 - 1, n2 - 1);
      findOrCreateVertex(v3 - 1, u3 - 1, n3 - 1);
    }
  }
  fclose(fp);
  printf("\n};\n\n");

  printf("GLfloat data[] = {\n");
  // Print all the vertices now
  for (int i = 0; i < vertexIndex; ++i) {
    printVertexDef(&(vertex_defs[i]));    
  }
  printf("};\n\n");
}

int main() {
  loadModel("/Users/josh0/Desktop/untitled.obj");
}
