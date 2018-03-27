#include <dirent.h>
#include <png.h>
#include "resources.h"
#include "utils.h"

GLuint shadow_program = 0;
GLuint mesh_program = 0;
GLuint sprite_program = 0;
GLuint quad_program = 0;
int g_mesh_count = 0;
int g_texture_count = 0;

void load_mesh(const char* path, Mesh* mesh, float* minx, float* miny, float* minz, float* maxx, float* maxy, float* maxz);
GLuint png_texture_load(const char * file_name, int * width, int * height);
int compile_shaders(const char* vert_file, const char* frag_file, GLuint* program);

void load_shaders() {
	compile_shaders("shaders/shadow.vert", "shaders/shadow.frag", &shadow_program);
	compile_shaders("shaders/model.vert", "shaders/model.frag", &mesh_program);
	compile_shaders("shaders/sprite.vert", "shaders/sprite.frag", &sprite_program);
	compile_shaders("shaders/quad.vert", "shaders/quad.frag", &quad_program);
}

int compile_shaders(const char* vert_file, const char* frag_file, GLuint* program) {
	GLchar errors[512];

  int len;
  char* file_source = get_file_data(vert_file, &len);
	const char* const vertex_source = file_source;
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_source, 0);
	glCompileShader(vertex_shader);
  free(file_source);

	GLint is_compiled = 0;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &is_compiled);
	if (is_compiled == GL_FALSE) {
		GLint max_length = 0;
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &max_length);
		glGetShaderInfoLog(vertex_shader, max_length, &max_length, errors);
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Could not load shader %s - %s", vert_file, errors);
		glDeleteShader(vertex_shader);
		return 1;
	}

  file_source = get_file_data(frag_file, &len);
  const char *const fragment_source = file_source;
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_source, 0);
	glCompileShader(fragment_shader);
  free(file_source);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &is_compiled);
	if (is_compiled == GL_FALSE) {
		GLint max_length = 0;
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &max_length);
		glGetShaderInfoLog(fragment_shader, max_length, &max_length, errors);
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Could not load shader %s - %s", vert_file, errors);
		glDeleteShader(fragment_shader);
		glDeleteShader(vertex_shader);
		return 1;
	}

	*program = glCreateProgram();

	glAttachShader(*program, vertex_shader);
	glAttachShader(*program, fragment_shader);

	glLinkProgram(*program);

	GLint is_linked = 0;
	glGetProgramiv(*program, GL_LINK_STATUS, (int *)&is_linked);
	if (is_linked == GL_FALSE) {
		GLint max_length = 0;
		glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &max_length);
		glGetProgramInfoLog(*program, max_length, &max_length, errors);
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Could not link shader - %s", errors);
		glDeleteProgram(*program);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		return 1;
	}

	glDetachShader(*program, vertex_shader);
	glDetachShader(*program, fragment_shader);

	return 0;
}

void load_meshes() {
  DIR *d;
  struct dirent *dir;
  d = opendir("models");
  int count = 0;
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".ply") != NULL) {
        count++;
      }
    }
    rewinddir(d);
    g_meshes = (Mesh*)malloc(sizeof(Mesh) * count);
    int i = 0;
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".ply") != NULL) {
        Mesh* model = &(g_meshes[i++]);
        strcpy(model->name, dir->d_name);
        char path[128];
        sprintf(path, "models/%s", model->name);
        load_mesh(path, model, &model->minx, &model->miny, &model->minz, &model->maxx, &model->maxy, &model->maxz);
        printf("%s >> %f %f %f => %f %f %f\n", path, model->minx, model->miny, model->minz, model->maxx, model->maxy, model->maxz);
        for (int i = 0; i < model->num_vertices; ++i) {
          
        }
      }
    }
    closedir(d);
  }

  g_mesh_count = count;
}

Mesh* get_entity_mesh(const Entity* entity) {
  assert(entity->value.model.mesh_index < g_mesh_count);

  return g_meshes + entity->value.model.mesh_index;
}

void load_mesh(const char* path, Mesh* model, float* minx, float* miny, float* minz, float* maxx, float* maxy, float* maxz) {
  FILE* fp;
  char* line = NULL;
  size_t len = 0;

  int vertices = 0;
  int faces = 0;

  fp = fopen(path, "r");

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
  *maxx = *maxy = *maxz = 0.0f;
  *minx = *miny = *minz = 99.9f;
  model->vertices = (GLfloat*)malloc(sizeof(GLfloat) * vertices * 11);
  model->num_vertices = vertices * 11;
  float x, y, z, nx, ny, nz, u, v, r, g, b;
  for (int i = 0; i < vertices; ++i) {
    getline(&line, &len, fp);
    size_t count = sscanf(line, "%f %f %f %f %f %f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz, &u, &v, &r, &g, &b);
    model->vertices[i * 11] = x;
    model->vertices[i * 11 + 1] = y;
    model->vertices[i * 11 + 2] = z;
    model->vertices[i * 11 + 3] = nx;
    model->vertices[i * 11 + 4] = ny;
    model->vertices[i * 11 + 5] = nz;
    model->vertices[i * 11 + 6] = u;
    model->vertices[i * 11 + 7] = v;
    model->vertices[i * 11 + 8] = r / 255.0;
    model->vertices[i * 11 + 9] = g / 255.0;
    model->vertices[i * 11 + 10] = b / 255.0;
    assert(count == 11);
    if (x > *maxx) { *maxx = x; }
    if (y > *maxy) { *maxy = y; }
    if (z > *maxz) { *maxz = z; }
    if (x < *minx) { *minx = x; }
    if (y < *miny) { *miny = y; }
    if (z < *minz) { *minz = z; }
  }

  // Parse faces
  model->indices = (GLuint*)malloc(sizeof(GLuint) * faces * 3);
  model->num_indices = faces * 3;
  int f1, f2, f3;
  for (int i = 0; i < faces; ++i) {
    getline(&line, &len, fp);
    size_t count = sscanf(line, "3 %d %d %d", &(model->indices[3 * i]), &(model->indices[3 * i + 1]), &(model->indices[3 * i + 2]));
    assert(count == 3);
  }

  glGenBuffers(1, &(model->verticesID));
  glBindBuffer(GL_ARRAY_BUFFER, model->verticesID);
  glBufferData(GL_ARRAY_BUFFER, model->num_vertices * sizeof(GLfloat), model->vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &(model->indicesID));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->indicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->num_indices * sizeof(GLuint), model->indices, GL_STATIC_DRAW);
}

void load_textures() {
  DIR *d;
  struct dirent *dir;
  d = opendir("textures");
  int count = 0;
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".png") != NULL) {
        count++;
      }
    }
    rewinddir(d);
    g_textures = (Texture*)malloc(sizeof(Texture) * count);
    int i = 0;
    while ((dir = readdir(d)) != NULL) {
      if (strstr(dir->d_name, ".png") != NULL) {
        Texture* texture = &(g_textures[i++]);
        strcpy(texture->name, dir->d_name);
        char path[128];
        sprintf(path, "textures/%s", texture->name);
        texture->id = png_texture_load(path, &texture->width, &texture->height); 
      }
    }
    closedir(d);
  }

  g_texture_count = count;
}

Texture* get_texture(const char* texture_name) {
  int texture_index = -1; 
  for (int i = 0; i < g_texture_count; ++i) {
    if (strcmp(g_textures[i].name, texture_name) == 0) {
      texture_index = i;
    }
  }
  assert(texture_index != -1);
  return &g_textures[texture_index];
}

GLuint png_texture_load(const char * file_name, int * width, int * height) {
  png_byte header[8];

  FILE *fp = fopen(file_name, "rb");
  if (fp == 0) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Could not load %s", file_name);
    return 0;
  }

  fread(header, 1, 8, fp);

  if (png_sig_cmp(header, 0, 8)) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "%s is not a PNG", file_name);
    fclose(fp);
    return 0;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "png_create_read_struct returned 0");
    fclose(fp);
    return 0;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "png_create_info_struct returned 0");
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    fclose(fp);
    return 0;
  }

  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "png_create_info_struct returned 0");
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
    fclose(fp);
    return 0;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Error from libpng\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    return 0;
  }

  png_init_io(png_ptr, fp);

  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  int bit_depth, color_type;
  png_uint_32 temp_width, temp_height;

  png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
      NULL, NULL, NULL);

  if (width) { *width = temp_width; }
  if (height) { *height = temp_height; }

  png_read_update_info(png_ptr, info_ptr);

  int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  rowbytes += 3 - ((rowbytes-1) % 4);

  png_byte * image_data;
  image_data = malloc(rowbytes * temp_height * sizeof(png_byte)+15);
  if (image_data == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Could not allocate memory for PNG image data");
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    return 0;
  }

  // row_pointers is for pointing to image_data for reading the png with libpng
  png_bytep * row_pointers = malloc(temp_height * sizeof(png_bytep));
  if (row_pointers == NULL) {
    SDL_Log("Could not allocate memory for PNG row pointers");
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(image_data);
    fclose(fp);
    return 0;
  }

  // set the individual row_pointers to point at the correct offsets of image_data
  int i;
  for (i = 0; i < temp_height; i++) {
    row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
  }

  // read the png into image_data through row_pointers
  png_read_image(png_ptr, row_pointers);

  // Generate the GL texture object
  GLuint texture = 0;

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, temp_width, temp_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // clean up
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  free(image_data);
  free(row_pointers);
  fclose(fp);

  return texture;
}

void create_text(const char* font_name, int size, const char* text, SDL_Color color, GLuint texture, int* width, int* height) {
  TTF_Font* ttf = TTF_OpenFont(font_name, size);

  if (ttf == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "TTF_OpenFont error\n");
  }
  SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(ttf, text, color, 512);
  if (surface == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "TTF_RenderText_Blended  error\n");
  }

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  *width = surface->w;
  *height = surface->h;
  SDL_Surface *glsurface = SDL_CreateRGBSurface(0, *width, *height, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
  SDL_BlitSurface(surface, NULL, glsurface, NULL);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, glsurface->pixels);
}
