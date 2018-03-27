#pragma once

#include "superfx.h"
#include "types.h"

Input g_input;

void process_keyboard_input(SDL_Event* event);
void process_joystick_input(SDL_Event* event);
