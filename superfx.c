// gcc -w superfx.c -framework OpenGL -lSDL2 -lgl-matrix

#include "stdio.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <SDL2/SDL.h>
#include <gl-matrix.h>
#include <assert.h>

#include "model.h"

GLuint program;

void render() {
  glLoadIdentity();
  glRotatef(40.0f, 1.0f, 0.0f, 1.0f);
  glBegin( GL_QUADS );
    glColor3f(1.0f, 0.0, 0.0);
    glVertex2f( -0.5f, -0.5f );
    glVertex2f( 0.5f, -0.5f );
    glVertex2f( 0.5f, 0.5f );
    glVertex2f( -0.5f, 0.5f );
  glEnd();
  glLoadIdentity();
}

GLuint quadVBO;
GLuint modelVBO;
GLuint modelIBO;
/*GLuint indices[] = {
  1, 3, 0,
  7, 5, 4,
  4, 1, 0,
  5, 2, 1,
  2, 7, 3,
  0, 7, 4,
  1, 2, 3,
  7, 6, 5,
  4, 5, 1,
  5, 6, 2,
  2, 6, 7,
  0, 3, 7
};
// x, y, z u, v, nx, ny, nz
float data[] = {//{1.0, 0.0, 1.0, 0.0, 0.0, -1.0, -1.0, 0.0, 1.0};
  1.000000, -1.000000, -1.000000, 0.0, 1.0,
  1.000000, -1.000000, 1.000000, 0.0, 0.0,
  -1.000000, -1.000000, 1.000000, 1.0, 1.0,
  -1.000000, -1.000000, -1.000000, 0.0, 1.0,
  0.336596, 1.000000, -0.336595, 0.0, 0.0,
  0.336595, 1.000000, 0.336596, 1.0, 1.0,
  -0.336596, 1.000000, 0.336596, 1.0, 0.0,
  -0.336596, 1.000000, -0.336596, 0.0, 1.0
};*/


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

int compileShaders(const char* vert, const char* frag) {
	GLchar errors[512];

	//Read our shaders into the appropriate buffers
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
		return 2;
	}

	program = glCreateProgram();

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);

	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
	if(isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &errors);
		printf("%s", errors);
		glDeleteProgram(program);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return 3;
	}

	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);

  free(vertexSource);
  free(fragmentSource);
	return 0;
}

int main() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    return 1;
  }

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

  SDL_Window* window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (window == NULL) {
    return 1;
  }

  SDL_GLContext* context = SDL_GL_CreateContext(window);
	if (context == NULL) {
    return 1;
  }

  // quad
  float quad[] = { 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0 };
  glGenBuffers(1, &quadVBO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad) * sizeof(float), quad, GL_STATIC_DRAW);

  //loadModel("/Users/josh0/Desktop/untitled.obj");
  glGenBuffers(1, &modelVBO);
  glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data) * sizeof(float), data, GL_STATIC_DRAW);

  glGenBuffers(1, &modelIBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) * sizeof(GLuint), indices, GL_STATIC_DRAW);

	compileShaders("/Users/josh0/superfx/shader.vert", "/Users/josh0/superfx/shader.frag");
  
  // Create FBO
	// create a texture object
	GLuint textureId;
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
	// attach the renderbuffer to depth attachment point
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER,      // 1. fbo target: GL_FRAMEBUFFER
														//GL_DEPTH_ATTACHMENT, // 2. attachment point
														//GL_RENDERBUFFER,     // 3. rbo target: GL_RENDERBUFFER
														//rboId);              // 4. rbo ID
	// check FBO status & render
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
		return 2;
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glClearColor(0.0, 1.0, 0.7, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0,0,256,256);
	render();

	glBindTexture(GL_TEXTURE_2D, textureId);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
		
	// revert back to main buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0,0,640,480);
  
  //glBindFramebuffer(GL_FRAMEBUFFER, fboId);
  //glViewport(0, 0, 256, 256);

  mat4_t identity;
  mat4_t view;
  mat4_t model;
  mat4_t perspective;
  identity = mat4_create(NULL);
  perspective = mat4_create(NULL);
  model = mat4_create(NULL);
  view = mat4_create(NULL);
  // perspective
  mat4_identity(perspective);
  mat4_perspective(60.0, 640.0 / 480.0, 1.0, 1000.0, perspective);
  // view
  mat4_identity(view);
  float translate[3] = {0.0f, 0.0f, -10.0f};
  mat4_translate(view, translate, view);
  // model
  mat4_identity(model);

  // Main Loop
  short run = 1;
  SDL_Event event;
  float rot = 0.0;

  while (run) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        run = 0;
      }
    }

    // rotate model
    rot += 0.01f;
    mat4_identity(model);
    float axis[3] = {0.707f, 0.707f, 0.0f};
    //float axis[3] = {1.0, 0.0, 0.0};
    mat4_rotate(model, rot, axis, NULL);

		glEnable(GL_CULL_FACE);
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glDisable(GL_LIGHTING);

		glViewport(0,0,640,480);
		glClearColor(0, 255, 0, 255);
		glClear(GL_COLOR_BUFFER_BIT);

    // Pass data to shaders
		glUseProgram(program);
    GLuint projectionID = glGetUniformLocation(program, "perspective");
    GLuint modelID = glGetUniformLocation(program, "model");
    GLuint viewID = glGetUniformLocation(program, "view");
    assert(projectionID != -1);
    assert(modelID != -1);
    assert(viewID != -1);
    glUniformMatrix4fv(projectionID, 1, GL_FALSE, perspective);
    glUniformMatrix4fv(modelID, 1, GL_FALSE, model);
    glUniformMatrix4fv(viewID, 1, GL_FALSE, view);

    // bind texture
    /*GLuint texLoc = glGetUniformLocation(program, "texture");
    assert(texLoc != -1);
    glUniform1i(texLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/

    // First pass, just fixed pipeline
		/*glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0.0, 1.0); glVertex3f(-1.f, 1.f, 0.f); glNormal3f(1.0f, 0.0f, 0.0f);
      glTexCoord2f(1.0, 1.0); glVertex3f(1.f, 1.f, 0.f); glNormal3f(0.0f, 1.0f, 0.0f);
      glTexCoord2f(0.0, 0.0); glVertex3f(-1.f, -1.f, 0.f); glNormal3f(0.0f, 0.0f, 1.0f);
      glTexCoord2f(1.0, 0.0); glVertex3f(1.f, -1.f, 0.f); glNormal3f(0.0f, 1.0f, 1.0f);
		glEnd();*/

    // Second pass, array of vertices
    /*glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
    glVertexPointer(3, GL_FLOAT, 0, NULL);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, sizeof(data) / sizeof(float) / 3);*/

    // Third pass, index array
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
    glVertexPointer(3, GL_FLOAT, 8 * sizeof(float), 0);
    glTexCoordPointer(2, GL_FLOAT, 8 * sizeof(float), 3 * sizeof(float));
    glNormalPointer(GL_FLOAT, 8 * sizeof(float), 5 * sizeof(float));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelIBO);
    glDrawElements(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, NULL);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    //glUseProgram(0);
    /*glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, NULL);
    glTexCoordPointer(2, GL_FLOAT, 0, 2);
    glDrawArrays(GL_TRIANGLES, 0, sizeof(quad) / sizeof(float) / 3);*/

    SDL_GL_SwapWindow(window);
  }
}
