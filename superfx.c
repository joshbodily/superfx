#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <SDL2/SDL.h>
#include <gl-matrix.h>
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include "entity.h"
#include "parse.h"
#include "text.h"
#include "bind.h"
#include "utils.h"
#include "console.h"

#define JOYSTICK_DEAD_ZONE 8000
#define PI_2 3.141592 * 0.5
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

extern void luaopen_SuperFX(lua_State*);

int tri_tri_intersect(float V0[3],float V1[3],float V2[3], float U0[3],float U1[3],float U2[3]);
int collide(Model* arwing, Model* object, vec3_t normal);

int nextpoweroftwo(int x) {
	double logbase2 = log(x) / log(2);
	return round(pow(2,ceil(logbase2)));
}

unsigned int power_two_floor(unsigned int val) {
  unsigned int power = 2, nextVal = power*2;
  while((nextVal *= 2) <= val)
    power*=2;
  return power*2;
}

// Render to texture
GLuint textureId;
GLuint ttfTextureId;

GLuint shadowProgram;
GLuint modelProgram;
GLuint spriteProgram;
GLuint quadProgram;

// Arwing & ground
int entityCount = 0;
Entity* entities[200];
Input input;
Entity* arwing;
Entity* ground;
Entity* reticle;
Entity* reticle2;

void ground_update(Entity* this) {
  /*mat4_t xform = arwing->transform;
  float x = xform[12];
  float y = xform[13];
  y = fmod(y, 0.25 * 20.0f);
  x = fmod(x, 0.25 * 20.0f);
  this->value.model.node->transform[12] = xform[12] - x;
  this->value.model.node->transform[13] = xform[13] + 40.0f - y;*/
}

void reticle_update(Entity* this) {
  /*mat4_t xform = arwing->value.model.node->transform;
  float reticle_position[3] = {0.0f, 5.0f, 0.0f};
  mat4_multiplyVec3(xform, reticle_position, NULL);
  reticle->value.model.node->transform[12] = reticle_position[0];
  reticle->value.model.node->transform[13] = reticle_position[1];
  reticle->value.model.node->transform[14] = reticle_position[2];*/
}

void projectile_update(Entity* this) {
  /*mat4_t xform = this->value.model.node->transform;
  float velocity_vector[3] = {0.0f, 0.5f, 0.0f};
  mat4_translate(xform, velocity_vector, NULL);*/
}

void spawn_projectile(mat4_t position) {
  /*mat4_t pos = mat4_create(position);
  float velocity_vector[3] = {0.0f, 1.0f, 0.0f};
  mat4_translate(pos, velocity_vector, NULL);
  mat4_transpose(pos, NULL);
  create_mesh_entity(entities[entityCount], "beam.ply", pos);
  float scale[3] = {2.0f, 2.0f, 2.0f};
  mat4_scale(entities[entityCount]->value.model.node->transform, scale, entities[entityCount]->value.model.node->transform);
  entities[entityCount]->value.model.update = projectile_update;
  entityCount++;*/
}

float yaw = 0.0f;

void arwing_update(Entity* this) {
  static float x, y, z = 0.0f;
  static float roll, pitch, velocity = 0.0f;

  yaw -= input.x_axis;
  pitch += input.y_axis;
  roll += input.roll * PI_2 * 0.075;

  // limits
  if (roll > PI_2) roll = PI_2;
  if (roll < -PI_2) roll = -PI_2;

  // return to straight ahead
  if (fabs(input.x_axis) < 0.01f) yaw *= 0.98;
  if (fabs(input.y_axis) < 0.01f) pitch *= 0.97;
  if (fabs(input.roll) < 0.01f) roll *= 0.90;
  
  // Handle input
  mat4_t xform = this->transform;
  mat4_identity(xform);
  mat4_rotateZ(xform, yaw * PI_2 * 0.02, xform);
  mat4_rotateX(xform, pitch * PI_2 * 0.02, xform);
  mat4_rotateY(xform, roll, xform);
  mat4_rotateY(xform, -yaw * PI_2 * 0.001, xform);

  float velocity_vector[3] = {0.0f, 0.08f, 0.0f};
  mat4_multiplyVec3(xform, velocity_vector, NULL);
  x += velocity_vector[0]; y += velocity_vector[1]; z += velocity_vector[2];

  xform[12] = x;
  xform[13] = y;
  xform[14] = z;
  
  // Projectiles
  if (input.fire) {
    spawn_projectile(this->transform);
    input.fire = 0;
  }
}

int compileShaders(const char* vert, const char* frag, GLuint* program) {
	GLchar errors[512];

  int len;
  char* fileSource = get_file_data(vert, &len);
	const char* const vertexSource = fileSource;
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, 0);
	glCompileShader(vertexShader);
  free(fileSource);

	GLint isCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
		glGetShaderInfoLog(vertexShader, maxLength, &maxLength, errors);
		printf("%s > %s", vert, errors);
		glDeleteShader(vertexShader);
		return 1;
	}

	//char* fragmentSource = getFileData(frag);
  fileSource = get_file_data(frag, &len);
  const char *const fragmentSource = fileSource;
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, 0);
	glCompileShader(fragmentShader);
  free(fileSource);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);
		glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, errors);
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
		glGetProgramInfoLog(*program, maxLength, &maxLength, errors);
		printf("%s", errors);
		glDeleteProgram(*program);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return 98;
	}

	glDetachShader(*program, vertexShader);
	glDetachShader(*program, fragmentShader);

  //free(vertexSource);
  //free(fragmentSource);
	return 0;
}

void renderQuad(GLuint texture, int x, int y, int w, int h, bool invert) {
  glUseProgram(quadProgram);

  // bind texture
  GLuint texLoc = glGetUniformLocation(quadProgram, "texture");
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

  float top = 1.0f;
  float bottom = 0.0f;
  if (invert) {
    top = 0.0f;
    bottom = 1.0f;
  }

  glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0f, bottom); glVertex2f(x_normalized, y_normalized);
    glTexCoord2f(1.0f, bottom); glVertex2f(x_normalized + width_normalized, y_normalized);
    glTexCoord2f(0.0f, top); glVertex2f(x_normalized, y_normalized + height_normalized);
    glTexCoord2f(1.0f, top); glVertex2f(x_normalized + width_normalized,  y_normalized + height_normalized);
  glEnd();
}

void processJoystickInput(SDL_Event* event) {
  if (event->type == SDL_JOYAXISMOTION) {
    if (event->jaxis.which == 0) { // Which joystick
      if (event->jaxis.axis == 0) {
        if (abs(event->jaxis.value) > JOYSTICK_DEAD_ZONE) {
          input.x_axis = event->jaxis.value / 32767.0f;
        } else {
          input.x_axis = 0.0f;
        }
      } else if (event->jaxis.axis == 1) {
        if (abs(event->jaxis.value) > JOYSTICK_DEAD_ZONE) {
          input.y_axis = event->jaxis.value / 32767.0f;
        } else {
          input.y_axis = 0.0f;
        }
      }
    }
  } 
  else if (event->type == SDL_JOYBUTTONDOWN) {
    if (event->jbutton.button == 4) {
      input.roll += -1.0f;
    } else if (event->jbutton.button == 5) {
      input.roll += 1.0f;
    }
  } 
  else if (event->type == SDL_JOYBUTTONUP) {
    if (event->jbutton.button == 4) {
      input.roll -= -1.0f;
    } else if (event->jbutton.button == 5) {
      input.roll -= 1.0f;
    }
  }
}

void processKeyboardInput(SDL_Event* event) {
  bool handled = console_handle_key(event);
  if (handled) return;

  short val = event->type == SDL_KEYDOWN ? 1 : 0;
  if (event->key.keysym.sym == SDLK_LEFT) { input.x_axis = -val; } 
  if (event->key.keysym.sym == SDLK_RIGHT) { input.x_axis = val; } 
  if (event->key.keysym.sym == SDLK_UP) { input.y_axis = -val; } 
  if (event->key.keysym.sym == SDLK_DOWN) { input.y_axis = val; } 
  if (event->key.keysym.sym == SDLK_e) { input.roll = val; } 
  if (event->key.keysym.sym == SDLK_q) { input.roll = -val; }
  if (event->key.keysym.sym == SDLK_SPACE) { input.fire = val; }
}

void renderSprite(Entity* entity, GLuint texture) {
  // Pass data to shaders
  glUseProgram(spriteProgram);

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
  // TODO: Do this once instead of each renderModel
  //mat4_identity(view);
  //mat4_rotateX(view, -PI_2, view);
  //mat4_t arwing_xform = arwing->value.model.node->transform;
  //float translate[3] = {-arwing_xform[12], -arwing_xform[13] + 3.0f, -arwing_xform[14] + 1.0f};
  float eye[3] = {0, 3.0f, 3.0f};
  float center[3] = {0, 0.0f, 0.0f};
  float up[3] = {0, 0.0f, 1.0f};
  //mat4_translate(view, translate, view);
//mat4_t mat4_lookAt(vec3_t eye, vec3_t center, vec3_t up, mat4_t dest) {
  mat4_lookAt(eye, center, up, view);

  model = entity->transform;

  GLuint projectionID = glGetUniformLocation(spriteProgram, "perspective");
  GLuint modelID = glGetUniformLocation(spriteProgram, "model");
  GLuint viewID = glGetUniformLocation(spriteProgram, "view");
  assert(projectionID != -1);
  assert(modelID != -1);
  assert(viewID != -1);

  // bind texture
  GLuint texLoc = glGetUniformLocation(spriteProgram, "texture");
  assert(texLoc != -1);
  glUniform1i(texLoc, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glUniformMatrix4fv(projectionID, 1, GL_FALSE, perspective);
  glUniformMatrix4fv(modelID, 1, GL_FALSE, model);
  glUniformMatrix4fv(viewID, 1, GL_FALSE, view);

  glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.0f, 0.5f); // top right
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.0f, 0.5f); // top left
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.0f, -0.5f); // bottom right
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.0f, -0.5f); // bottom left
  glEnd();
}

void renderShadow(Entity* entity) {
  // Pass data to shaders
  glUseProgram(shadowProgram);

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
  mat4_rotateX(view, -PI_2, view);
  mat4_rotateY(view, PI_2 * 0.1f, view);
  mat4_t arwing_xform = arwing->transform;
  float translate[3] = {-arwing_xform[12], -arwing_xform[13] + 3.0f, -arwing_xform[14] + 1.0f};
  mat4_translate(view, translate, view);

  model = entity->transform;
  GLuint projectionID = glGetUniformLocation(shadowProgram, "perspective");
  GLuint modelID = glGetUniformLocation(shadowProgram, "model");
  GLuint viewID = glGetUniformLocation(shadowProgram, "view");
  GLuint heightID = glGetUniformLocation(shadowProgram, "height");
  assert(projectionID != -1);
  assert(modelID != -1);
  assert(viewID != -1);
  //assert(heightID != -1);

  glUniformMatrix4fv(projectionID, 1, GL_FALSE, perspective);
  glUniformMatrix4fv(modelID, 1, GL_FALSE, model);
  glUniformMatrix4fv(viewID, 1, GL_FALSE, view);

  glUniform1f(heightID, -arwing_xform[14] + 1.0f);

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
  glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, NULL);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
}

int main() {
  // Setup Lua
	char script[4096];
	int error;
	L = lua_open();   /* opens Lua */
	luaL_openlibs(L);
  luaopen_SuperFX(L);

  // Glist tests
  /*GList* list = g_list_alloc();
  int foo = 1;
  int bar = 2;
  g_list_append(list, GINT_TO_POINTER(1));
  g_list_append(list, GINT_TO_POINTER(2));
  for (GList* l = list; l != NULL; l = l->next) {
    printf("Item %d\n", GPOINTER_TO_INT(l->data));
  }*/

  // Setup Gamepad
  SDL_Joystick* gGameController = NULL;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    fprintf(stderr, "Could not initialize video or joystick%s\n", SDL_GetError());
    return 1;
  }

  // Check for joysticks
  if(SDL_NumJoysticks() < 1) {
    fprintf(stderr, "Warning: No joysticks connected!\n");
    //return 1;
  } else {
    // Load joystick
    gGameController = SDL_JoystickOpen(0);
    if (gGameController == NULL) {
      fprintf(stderr, "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
    }
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
  parse_textures();  
  // load all models
  parse_models();
  // load fonts
  init_fonts();

  int w;
  int h;
  SDL_Color white = {255, 255, 255, 255};
	glGenTextures(1, &ttfTextureId); // Do I need this?
  //ttfTextureId = create_text("MEGAMAN10.ttf", "This is a test", white, &w, &h);

  // setup lua
  FILE* file = fopen("scripts/level1.lua", "r");
  size_t read = fread(script, 1, 4096, file);
  error = luaL_loadbuffer(L, script, read, "line") || lua_pcall(L, 0, 0, 0);
  if (error) {
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    return 1;
  }

	compileShaders("shaders/shadow.vert", "shaders/shadow.frag", &shadowProgram);
	compileShaders("shaders/model.vert", "shaders/model.frag", &modelProgram);
	compileShaders("shaders/sprite.vert", "shaders/sprite.frag", &spriteProgram);
	compileShaders("shaders/quad.vert", "shaders/quad.frag", &quadProgram);
  
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
		
  // Main Loop
  short run = 1;
  SDL_Event event;
  float rot = 0.0;

  
  // Create Game Entities, leave 2 slots for ground & arwing
  //entityCount = parse_level(&(entities[0]), "models/level.txt");

  // Create the ground
  mat4_t identity = mat4_create(NULL);
  mat4_identity(identity);
  identity[0] *= 20.0f;
  identity[5] *= 20.0f;
  identity[10] *= 20.0f;
  entities[entityCount] = (struct Entity *)calloc(1, sizeof(struct Entity));
  create_mesh_entity(entities[entityCount], "ground.ply", identity);
  free(identity);
  ground = entities[entityCount]; 
  ground->value.model.update = ground_update;
  ground->value.model.mode = GL_POINTS;
  entityCount++;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while (run) {
    // Process input
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        run = 0;
      }
      processJoystickInput(&event);
      processKeyboardInput(&event);
    }

    // Process logic
    lua_getfield(L, LUA_GLOBALSINDEX, "update");
    error = lua_pcall(L, 0, 0, 0);
    if (error) {
      fprintf(stderr, "Error %s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
    }
    // lua_gc(L, LUA_GCCOLLECT, 0);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
		glViewport(0, 0, 256, 256);
		//glClearColor(207.0/255.0, 252.0/255.0, 255.0/255.0, 1);
		glClearColor(0, 0, 0, 1);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Background
    renderQuad(g_textures[3].id, 0, 0, 256 * 2, 256 * 2, false);
    // Text
    //renderQuad(ttfTextureId, 0, 0, w, h, TRUE);
		glClear(GL_DEPTH_BUFFER_BIT);

    lua_getfield(L, LUA_GLOBALSINDEX, "render");
    error = lua_pcall(L, 0, 0, 0);
    if (error) {
      fprintf(stderr, "Error %s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
    }

    // 3. UI
		glClear(GL_DEPTH_BUFFER_BIT);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    renderQuad(g_textures[2].id, 8, 8, 64 * 2, 64 * 2, false);

    render_console(ttfTextureId);

    // Render scaled full-screen quad
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 512, 512);
    glClearColor(0, 0, 0, 255);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad(textureId, 0, 0, 512, 512, false);

    // Collision Test-Bed
    // TODO: For each model in level
    //float normal[3];
      /*if (collide(&arwing->value.model, &entities[4].value.model, normal)) {
        arwing->value.model.node->transform[12] += normal[0] * 2.0f;
        arwing->value.model.node->transform[13] += normal[1] * 2.0f;
        arwing->value.model.node->transform[14] += normal[2] * 2.0f;
        //printf("Colliding\n");
      } else {
        //printf("Not Colliding\n");
      }*/
    // End

    SDL_GL_SwapWindow(window);
    SDL_Delay(17);
  }

  SDL_JoystickClose(gGameController);
  gGameController = NULL;

  //Destroy window
  SDL_DestroyWindow(window);
  window = NULL;

  SDL_Quit();
}

int collide(Model* arwing, Model* object, vec3_t normal) {
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
