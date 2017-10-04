// gcc -w -g superfx.c writepng.c -framework OpenGL -lSDL2 -lgl-matrix -lpng

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <SDL2/SDL.h>
#include <gl-matrix.h>
#include <assert.h>
#include "stdio.h"
#include "model.h"
//#include "model2.h"

#include "parse.c"

#define PI_2 3.141592 * 0.5

GLuint png_texture_load(const char * file_name, int * width, int * height);
int save_png_to_file(GLubyte*, const char *path);

// Render to texture
GLuint textureId;
GLuint program;
GLuint modelVBO;
GLuint modelIBO;

// Render full-screen quad
GLuint program2;

//

// Arwing state
float roll = 0.0f;
float pitch = 0.0f;

char* getFileData(const char* path) {
	FILE* f = fopen(path, "rb");
  fseek(f, 0, SEEK_END);
  int length = ftell(f);
  fseek(f, 0, SEEK_SET);
  char* buffer = malloc(length);
  if (buffer) {
    fread(buffer, 1, length, f);
  }
  buffer[length] = '\0';
  fclose(f);
	return buffer;
}

int compileShaders(const char* vert, const char* frag, GLuint* program) {
	GLchar errors[512];

	const char* vertexSource = getFileData(vert);
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, 0);
	glCompileShader(vertexShader);

	GLint isCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
		glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &errors);
		printf("%s > %s", vert, errors);
		glDeleteShader(vertexShader);
		return 1;
	}

	const char* fragmentSource = getFileData(frag);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, 0);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);
		glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &errors);
		printf("%s > %s", frag, errors);
		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);
		return 99;
	}

	*program = glCreateProgram();

	glAttachShader(*program, vertexShader);
	glAttachShader(*program, fragmentShader);

	glLinkProgram(*program);

	GLint isLinked = 0;
	glGetProgramiv(*program, GL_LINK_STATUS, (int *)&isLinked);
	if(isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &maxLength);
		glGetProgramInfoLog(*program, maxLength, &maxLength, &errors);
		printf("%s", errors);
		glDeleteProgram(*program);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return 98;
	}

	glDetachShader(*program, vertexShader);
	glDetachShader(*program, fragmentShader);

  free(vertexSource);
  free(fragmentSource);
	return 0;
}

void renderQuad(GLuint texture, int x, int y, int w, int h) {
  glUseProgram(program2);
  // bind texture
  GLuint texLoc = glGetUniformLocation(program2, "texture");
  assert(texLoc != -1);
  glUniform1i(texLoc, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  float x_normalized = (x - 128.0) / 128.0;
  float y_normalized = (y - 128.0) / 128.0;
  float width_normalized = w / 256.0;
  float height_normalized = h / 256.0;

  glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x_normalized, y_normalized);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x_normalized + width_normalized, y_normalized);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x_normalized, y_normalized + height_normalized);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x_normalized + width_normalized,  y_normalized + height_normalized);
  glEnd();
}

void processKeyboardInput(SDL_Event* event) {
  static short left, right, up, down = 0;
  if (event) {
    if (event->type == SDL_KEYDOWN) {
      if (event->key.keysym.sym == SDLK_LEFT) { left = 1; } 
      if (event->key.keysym.sym == SDLK_RIGHT) { right = 1; } 
      if (event->key.keysym.sym == SDLK_UP) { up = 1; } 
      if (event->key.keysym.sym == SDLK_DOWN) { down = 1; } 
    }
    if (event->type == SDL_KEYUP) {
      if (event->key.keysym.sym == SDLK_LEFT) { left = 0; } 
      if (event->key.keysym.sym == SDLK_RIGHT) { right = 0; } 
      if (event->key.keysym.sym == SDLK_UP) { up = 0; } 
      if (event->key.keysym.sym == SDLK_DOWN) { down = 0; } 
    }
  } else {
    if (left) { roll -= PI_2 * 0.02; }
    if (right) { roll += PI_2 * 0.02; }
    if (up) { pitch -= PI_2 * 0.02; }
    if (down) { pitch += PI_2 * 0.02; }
  }
}

void renderModel() {
  // Pass data to shaders
  glUseProgram(program);

  mat4_t view;
  mat4_t model, temp;
  mat4_t perspective;
  perspective = mat4_create(NULL);
  model = mat4_create(NULL);
  temp = mat4_create(NULL);
  view = mat4_create(NULL);
  // perspective
  mat4_identity(perspective);
  mat4_perspective(60.0, 1.0, 1.0, 1000.0, perspective);
  // view
  mat4_identity(view);
  float translate[3] = {0.0f, 0.0f, -3.0f};
  mat4_translate(view, translate, view);

  // rotate model
  mat4_identity(temp);
  mat4_rotateX(temp, -PI_2, NULL);
  mat4_rotateY(temp, roll * 0.33, temp);
  mat4_rotateZ(temp, -roll , temp); // yaw, should be opposite
  mat4_rotateX(temp, pitch, model);

  GLuint projectionID = glGetUniformLocation(program, "perspective");
  GLuint modelID = glGetUniformLocation(program, "model");
  GLuint viewID = glGetUniformLocation(program, "view");
  assert(projectionID != -1);
  assert(modelID != -1);
  assert(viewID != -1);

  glUniformMatrix4fv(projectionID, 1, GL_FALSE, perspective);
  glUniformMatrix4fv(modelID, 1, GL_FALSE, model);
  glUniformMatrix4fv(viewID, 1, GL_FALSE, view);

  // Third pass, index array
  size_t stride = 11 * sizeof(float);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
  glVertexPointer(3, GL_FLOAT, stride, 0);
  glNormalPointer(GL_FLOAT, stride, 3 * sizeof(float));
  glTexCoordPointer(2, GL_FLOAT, stride, 6 * sizeof(float));
  glColorPointer(3, GL_FLOAT, stride, 8 * sizeof(float));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelIBO);
  glDrawElements(GL_TRIANGLES, sizeof(indices) / 3, GL_UNSIGNED_INT, NULL);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  //GLubyte data[256 * 256 * 3]; 
  //glReadPixels(0, 0, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, &data);
  //save_png_to_file (&data, "/Users/josh0/superfx/out.png");
}

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    return 1;
  }

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
  SDL_Window* window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (window == NULL) {
    return 2;
  }

  SDL_GLContext* context = SDL_GL_CreateContext(window);
	if (context == NULL) {
    return 3;
  }

  // load any textures
  
  int count = 0;
  TextureDef* textures = parse(&count);  
  assert(count > 0);
  for (int i = 0; i < count; ++i) {
    printf("%s %d %dx%d\n", textures[i].name, textures[i].id, textures[i].width, textures[i].height);
  }
  //imageTextureId = png_texture_load("slippy.png", NULL, NULL);
  //assert(imageTextureId);
  //imageTextureId = png_texture_load("out.png", NULL, NULL);

  // quad
  //float quad[] = { 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0 };
  //glGenBuffers(1, &quadVBO);
  //glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  //glBufferData(GL_ARRAY_BUFFER, sizeof(quad) * sizeof(float), quad, GL_STATIC_DRAW);

  glGenBuffers(1, &modelVBO);
  glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data) * sizeof(float), data, GL_STATIC_DRAW);

  glGenBuffers(1, &modelIBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) * sizeof(GLuint), indices, GL_STATIC_DRAW);

	compileShaders("/Users/josh0/superfx/shader.vert", "/Users/josh0/superfx/shader.frag", &program);
	compileShaders("/Users/josh0/superfx/shader2.vert", "/Users/josh0/superfx/shader2.frag", &program2);
  
  // Create FBO
	// create a texture object
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	// create a renderbuffer object to store depth info
	GLuint rboId;
	glGenRenderbuffers(1, &rboId);
	glBindRenderbuffer(GL_RENDERBUFFER, rboId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 256, 256);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// create a framebuffer object
	GLuint fboId;
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	// attach the texture to FBO color attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER,        // 1. fbo target: GL_FRAMEBUFFER 
												 GL_COLOR_ATTACHMENT0,  // 2. attachment point
												 GL_TEXTURE_2D,         // 3. tex target: GL_TEXTURE_2D
												 textureId,             // 4. tex ID
												 0);                    // 5. mipmap level: 0(base)
  // attach depthbuffer image to FBO
  glFramebufferRenderbuffer(GL_FRAMEBUFFER,       // 1. fbo target: GL_FRAMEBUFFER
                            GL_DEPTH_ATTACHMENT,  // 2. depth attachment point
                            GL_RENDERBUFFER,      // 3. rbo target: GL_RENDERBUFFER
                            rboId);          // 4. rbo ID
	// check FBO status & render
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		return 4;
  }
		
  mat4_t identity;
  // model
  identity = mat4_create(NULL);

  // Main Loop
  short run = 1;
  SDL_Event event;
  float rot = 0.0;

  while (run) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        run = 0;
      }
      processKeyboardInput(&event);
    }
    processKeyboardInput(NULL);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
		glViewport(0, 0, 256, 256);
		glClearColor(156.0/255.0, 219.0/255.0, 111.0/255.0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderQuad(textures[0].id, 0, 0, 256 * 2, 256 * 2);
		glClear(GL_DEPTH_BUFFER_BIT);
    renderModel();
    renderQuad(textures[1].id, 8, 8, 64 * 2, 64 * 2);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, 512, 512);
    glClearColor(0, 0, 0, 255);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad(textureId, 0, 0, 512, 512);


    SDL_GL_SwapWindow(window);
  }
}
