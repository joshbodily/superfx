#include "input.h"

#define JOYSTICK_DEAD_ZONE 5000

void init_input() {
  g_input.left_trigger = -1.0f;
  g_input.right_trigger = -1.0f;
}

void process_joystick_input(SDL_Event* event) {
  //g_input.start_pressed = false;
  /*g_input.left_x_axis = 0.0f;
  g_input.left_y_axis = 0.0f;
  g_input.right_x_axis = 0.0f;
  g_input.right_y_axis = 0.0f;*/

  if (event->type == SDL_JOYAXISMOTION) {
    //fprintf(stderr, "Axis %d\n", event->jaxis.axis);
    // Left Joystick
    if (event->jaxis.axis == 0) {
      if (abs(event->jaxis.value) > JOYSTICK_DEAD_ZONE) {
        float val = event->jaxis.value / 32767.0f;
        g_input.left_x_axis = val * val * val;
      } else {
        g_input.left_x_axis = 0.0f;
      }
    } else if (event->jaxis.axis == 1) {
      if (abs(event->jaxis.value) > JOYSTICK_DEAD_ZONE) {
        float val = event->jaxis.value / 32767.0f;
        g_input.left_y_axis = val * val * val;
      } else {
        g_input.left_y_axis = 0.0f;
      }
    }
    // Right Joystick
    if (event->jaxis.axis == 3) {
      if (abs(event->jaxis.value) > JOYSTICK_DEAD_ZONE) {
        g_input.right_x_axis = event->jaxis.value / 32767.0f;
      } else {
        g_input.right_x_axis = 0.0f;
      }
    } else if (event->jaxis.axis == 4) {
      if (abs(event->jaxis.value) > JOYSTICK_DEAD_ZONE) {
        g_input.right_y_axis = event->jaxis.value / 32767.0f;
      } else {
        g_input.right_y_axis = 0.0f;
      }
    }
    // Left Trigger
    //printf("Axis %d", event->jaxis.axis);
    if (event->jaxis.axis == 4) {
      g_input.left_trigger = event->jaxis.value / 32767.0f;
    }
    // Right Trigger
    if (event->jaxis.axis == 5) {
      g_input.right_trigger = event->jaxis.value / 32767.0f;
    }
  } else if (event->type == SDL_JOYBUTTONDOWN || event->type == SDL_JOYBUTTONUP) {
    bool down = event->type == SDL_JOYBUTTONDOWN;
    printf("Button %d %s\n", event->jbutton.button, down ? "down" : "up");
    switch (event->jbutton.button) {
      case 0: g_input.a = down; break;
      case 1: g_input.b = down; break;
      case 2: g_input.x = down; break;
      case 3: g_input.y = down; break;
      case 4: g_input.left_shoulder = down; break;
      case 5: g_input.right_shoulder = down; break;
      case 6: 
        g_input.start_pressed = (!g_input.start && down);
        if (g_input.start_pressed) { printf("Pressed\n"); }
        g_input.start = down; 
        break;
      //case 6: g_input.left_joystick = down; break;
      case 7: g_input.right_joystick = down; break;
      case 9: g_input.back = down; break;
      case 10: g_input.xbox = down; break;
      case 11: g_input.up = down; break;
      case 12: g_input.down = down; break;
      case 13: g_input.left = down; break;
      case 14: g_input.right = down; break;
    }
  }
}

void process_keyboard_input(SDL_Event* event) {
  /*short val = event->type == SDL_KEYDOWN ? 1 : 0;

  if (event->key.keysym.sym == SDLK_LEFT) { g_input.x_axis = -val; } 
  if (event->key.keysym.sym == SDLK_RIGHT) { g_input.x_axis = val; } 
  if (event->key.keysym.sym == SDLK_UP) { g_input.y_axis = -val; } 
  if (event->key.keysym.sym == SDLK_DOWN) { g_input.y_axis = val; } 
  if (event->key.keysym.sym == SDLK_e) { g_input.roll = val; } 
  if (event->key.keysym.sym == SDLK_q) { g_input.roll = -val; }
  if (event->key.keysym.sym == SDLK_SPACE) { g_input.fire = val; }*/
}
