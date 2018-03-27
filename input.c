#include "input.h"

#define JOYSTICK_DEAD_ZONE 8000

void process_joystick_input(SDL_Event* event) {
  if (event->type == SDL_JOYAXISMOTION) {
    if (event->jaxis.which == 0) { // Which joystick
      if (event->jaxis.axis == 0) {
        if (abs(event->jaxis.value) > JOYSTICK_DEAD_ZONE) {
          g_input.x_axis = event->jaxis.value / 32767.0f;
        } else {
          g_input.x_axis = 0.0f;
        }
      } else if (event->jaxis.axis == 1) {
        if (abs(event->jaxis.value) > JOYSTICK_DEAD_ZONE) {
          g_input.y_axis = event->jaxis.value / 32767.0f;
        } else {
          g_input.y_axis = 0.0f;
        }
      }
    }
  } else if (event->type == SDL_JOYBUTTONDOWN) {
    if (event->jbutton.button == 4) {
      g_input.roll += -1.0f;
    } else if (event->jbutton.button == 5) {
      g_input.roll += 1.0f;
    }
  } else if (event->type == SDL_JOYBUTTONUP) {
    if (event->jbutton.button == 4) {
      g_input.roll -= -1.0f;
    } else if (event->jbutton.button == 5) {
      g_input.roll -= 1.0f;
    }
  }
}

void process_keyboard_input(SDL_Event* event) {
  short val = event->type == SDL_KEYDOWN ? 1 : 0;

  if (event->key.keysym.sym == SDLK_LEFT) { g_input.x_axis = -val; } 
  if (event->key.keysym.sym == SDLK_RIGHT) { g_input.x_axis = val; } 
  if (event->key.keysym.sym == SDLK_UP) { g_input.y_axis = -val; } 
  if (event->key.keysym.sym == SDLK_DOWN) { g_input.y_axis = val; } 
  if (event->key.keysym.sym == SDLK_e) { g_input.roll = val; } 
  if (event->key.keysym.sym == SDLK_q) { g_input.roll = -val; }
  if (event->key.keysym.sym == SDLK_SPACE) { g_input.fire = val; }
}
