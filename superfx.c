#include <stdio.h>
#include "superfx.h"
#include "resources.h"
#include "types.h"
#include "input.h"
#include "bind.h"
#include "utils.h"

#define PI_2 3.141592 * 0.5
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

extern void luaopen_SuperFX(lua_State*);

int main() {
  Entity background_entity;
  background_entity.type = QUAD;
  background_entity.value.quad.x = 0;
  background_entity.value.quad.y = 0;
  background_entity.value.quad.width = 512;
  background_entity.value.quad.height = 512;

  // render_quad(&fbo_entity, 0, 0, 512, 512, false);
  Entity fbo_entity;
  fbo_entity.type = QUAD;
  fbo_entity.value.quad.x = 0;
  fbo_entity.value.quad.y = 0;
  fbo_entity.value.quad.width = 512;
  fbo_entity.value.quad.height = 512;

  // Setup Gamepad
  SDL_Joystick* joystick = NULL;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Could not initialize video or joystick%s\n", SDL_GetError());
    return 1;
  }

  // Check for joysticks
  if(SDL_NumJoysticks() < 1) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Warning: No joysticks connected!\n");
    //return 1;
  } else {
    // Load joystick
    joystick = SDL_JoystickOpen(0);
    if (joystick == NULL) {
      SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
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

  load_shaders();
  load_textures();  
  // TODO: Move to bind
  Texture* texture = get_texture("sky.png");
  background_entity.value.quad.texture_id = texture->id;
  load_meshes();
  TTF_Init();

  // setup lua
  lua_State* L;
	L = lua_open();   /* opens Lua */
	luaL_openlibs(L);
  luaopen_SuperFX(L);

  int len;
  //char* script = get_file_data("scripts/main.lua", &len);
  char* script = get_file_data("scripts/main.lua", &len);
  int error = luaL_loadbuffer(L, script, len, "[main]") || lua_pcall(L, 0, 0, 0);
  if (error) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "%s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    return 1;
  }
  
  // Create FBO
	// create a texture object
	glGenTextures(1, &fbo_entity.value.quad.texture_id);
	glBindTexture(GL_TEXTURE_2D, fbo_entity.value.quad.texture_id);
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
												 fbo_entity.value.quad.texture_id,             // 4. tex ID
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
		
  // GL Global Setup
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);

  bool run = true;
  while (run) {
    // -1. Input
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        run = 0;
      }
      process_joystick_input(&event);
      process_keyboard_input(&event);
    }

    // 0. Game logic
    lua_getfield(L, LUA_GLOBALSINDEX, "update");
    error = lua_pcall(L, 0, 0, 0);
    if (error) {
      SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Error %s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
		glViewport(0, 0, 256, 256);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Background
    render_quad(&background_entity);
		glClear(GL_DEPTH_BUFFER_BIT);

    // 2. Render all entities
    lua_getfield(L, LUA_GLOBALSINDEX, "render");
    error = lua_pcall(L, 0, 0, 0);
    if (error) {
      SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Error %s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
    }

    // 3. UI (Text and or character HUD or icons)
		glClear(GL_DEPTH_BUFFER_BIT);
    //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    //render_quad(g_textures[2].id, 8, 8, 64 * 2, 64 * 2, false);

    //render_console(ttfTextureId);

    // Render scaled full-screen quad
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 512, 512);
    glClearColor(0, 0, 0, 255);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_quad(&fbo_entity);

    SDL_GL_SwapWindow(window);
    SDL_Delay(17);
  }

  SDL_JoystickClose(joystick);
  joystick = NULL;

  //Destroy window
  SDL_DestroyWindow(window);
  window = NULL;

  SDL_Quit();
}

