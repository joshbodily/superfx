// clang -Wno-deprecated-declarations `pkg-config --cflags glib-2.0` -I /usr/local/Cellar/sdl2/2.0.9_1/include -I /usr/local/Cellar/sdl2/2.0.9_1/include/SDL2/ -I /usr/local/Cellar/sdl2_ttf/2.0.14/include/SDL2/ -Igl-matrix gl-matrix/*.c bind.c hex.c resources.c utils.c -L /usr/local/Cellar/sdl2/2.0.9_1/lib -lSDL2 -framework OpenGL -lpng -lSDL2_ttf -lglib-2.0

#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <assert.h>
#include <stdbool.h>
#include <gl-matrix.h>
#include "resources.h"
#include "types.h"
//#include "input.h"
#include "bind.h"
#include "utils.h"

#include <glib.h>
#include <SDL.h>
#include <SDL_ttf.h>

#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#elif __linux__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#endif

#define DEG_30 30.0 / 180.0 * M_PI
#define DEG_60 60.0 / 180.0 * M_PI
#define BIG_R 1.15470053838
#define EPSILON 0.001

// Camera
GList* g_entities;
//Entity camera;
Mesh g_board_mesh;
//Entity g_board;
//Entity g_text;
float g_mat_perspective[16];

void free_text_entity(Entity* text_entity) {
  GLuint id = text_entity->value.quad.texture_id;
  g_entities = g_list_remove(g_entities, (void *)text_entity);
  glDeleteTextures(1, &id);
  free(text_entity);
}

typedef enum UnitType {
  INFANTRY
} UnitType;

typedef struct Unit {
  UnitType type;
} Unit;

typedef enum TileType {
  GRASS,
  FOREST,
  TOWN
} TileType;

typedef enum Zone {
  LEFT, 
  LEFT_AND_CENTER,
  CENTER,
  CENTER_AND_RIGHT,
  RIGHT
} Zone;

struct Hex;

typedef struct Hex {
  struct Hex* northwest;
  struct Hex* northeast;
  struct Hex* east;
  struct Hex* southeast;
  struct Hex* southwest;
  struct Hex* west;
  double x;
  double y;
  int row;
  int col;
  Zone zone;
  int flags;
  TileType type;
  Unit* unit;
  int visited;
  int min_visit_cost;
  int best_moves_left; // maximize
  int lowest_range;
} Hex;

Zone get_zone(int row, int col) {
  // odd
  if (row % 2) {
    if (col < 3) return LEFT;
    else if (col == 3) return LEFT_AND_CENTER;
    else if (col < 8) return CENTER;
    else if (col == 8) return CENTER_AND_RIGHT;
    else return RIGHT;
  } else {
  // even
    if (col < 4) return LEFT;
    else if (col < 9) return CENTER;
    else return RIGHT;
  }
}

int blocks_line_of_sight(const Hex* cell) {
  switch (cell->type) {
    case FOREST:
    case TOWN:
      return 1;
    default:
      return 0;
  }
}

void rotate_vector2(double x, double y, double* out_x, double* out_y, double angle) {
  *out_x = x * cos(angle) - y * sin(angle);
  *out_y = x * sin(angle) + y * cos(angle);
}

int line_of_sight(const Hex* current, const Hex* dest, const Hex* cells_in_range[], size_t num_cells) {
  double ref_angle = atan2(dest->y - current->y, dest->x - current->x);
  int left_blocked = 0;
  int right_blocked = 0;
  for (size_t i = 0; i < num_cells; ++i) {
    const Hex* cell = cells_in_range[i];
    if (!blocks_line_of_sight(cell) || cell == dest || cell == current) continue;

    int greater = 0;
    int less = 0;
    int along = 0;
    for (int i = 0; i < 6; ++i) {
      double x = cell->x + cos(DEG_30 + DEG_60 * i) * BIG_R;
      double y = cell->y + sin(DEG_30 + DEG_60 * i) * BIG_R;
      double vecx = x - current->x; 
      double vecy = y - current->y; 
      rotate_vector2(vecx, vecy, &vecx, &vecy, -ref_angle);
      double vertex_angle = atan2(vecy, vecx);

      if (fabs(vertex_angle) > 1.5708) break;
      else if (fabs(vertex_angle) < 0.0001) along = 1;
      else if (vertex_angle > 0.0) greater = 1;
      else if (vertex_angle < 0.0) less = 1;
    }
    if (along && less) left_blocked = 1;
    if (along && greater) right_blocked = 1;
    if (greater && less) return 0;
  }

  return !(left_blocked && right_blocked);
}

int should_range(Hex* hex, int range) {
  if (hex == NULL) return 0;
  if (range < hex->lowest_range) {
    return 1;
  } else {
    return 0;
  }
}

int should_visit(Hex* hex, int moves_left) {
  if (hex == NULL) return 0;
  if (hex->visited) {
    // Already visited, compare moves left
    if (hex->best_moves_left < moves_left) {
      //printf("Found better way prev: %d now: %d\n", hex->best_moves_left, moves_left);
      return 1;
    } else {
      return 0;
    }
  }
  return 1;
}

int stop_when_move_onto(Hex* hex) {
  switch (hex->type) {
    case FOREST:
    case TOWN:
      return 1;
    default:
      return 0;
  }
}

void range(Hex* current, int dist, int max_dist) {
  current->visited = 1;
  current->lowest_range = dist;

  if (dist < max_dist) {
    if (should_range(current->west, dist + 1)) range(current->west, dist + 1, max_dist);
    if (should_range(current->east, dist + 1)) range(current->east, dist + 1, max_dist);
    if (should_range(current->northwest, dist + 1)) range(current->northwest, dist + 1, max_dist);
    if (should_range(current->northeast, dist + 1)) range(current->northeast, dist + 1, max_dist);
    if (should_range(current->southwest, dist + 1)) range(current->southwest, dist + 1, max_dist);
    if (should_range(current->southeast, dist + 1)) range(current->southeast, dist + 1, max_dist);
  }
}

void _visit(Hex* current, int moves_left) {
  if (!current->visited) printf("Visiting %d %d\n", current->row, current->col);
  current->visited = 1;
  current->best_moves_left = moves_left;
  if (stop_when_move_onto(current)) moves_left = 0;

  if (moves_left > 0) {
    if (should_visit(current->west, moves_left)) _visit(current->west, moves_left - 1);
    if (should_visit(current->east, moves_left)) _visit(current->east, moves_left - 1);
    if (should_visit(current->northwest, moves_left)) _visit(current->northwest, moves_left - 1);
    if (should_visit(current->northeast, moves_left)) _visit(current->northeast, moves_left - 1);
    if (should_visit(current->southwest, moves_left)) _visit(current->southwest, moves_left - 1);
    if (should_visit(current->southeast, moves_left)) _visit(current->southeast, moves_left - 1);
  }
}

void visit(Hex* current, int moves_left) {
  current->visited = 1;
  if (should_visit(current->west, moves_left)) _visit(current->west, moves_left - 1);
  if (should_visit(current->east, moves_left)) _visit(current->east, moves_left - 1);
  if (should_visit(current->northwest, moves_left)) _visit(current->northwest, moves_left - 1);
  if (should_visit(current->northeast, moves_left)) _visit(current->northeast, moves_left - 1);
  if (should_visit(current->southwest, moves_left)) _visit(current->southwest, moves_left - 1);
  if (should_visit(current->southeast, moves_left)) _visit(current->southeast, moves_left - 1);
}

Hex* get_by_row_col(Hex board[], int row, int col) {
  for (int i = 0; i < 113; ++i) {
    if (board[i].row == row && board[i].col == col) return &(board[i]);
  }
  return NULL;
}


int main() {
  Hex board[113];

  // Initialize all the cell row and column positions
  for (int i = 0; i < 113; ++i) {
    int group = i / 25;
    int col = i % 25;
    int row = col > 12 ? group * 2 + 1 : group * 2;
    if (col > 12) col -= 13;
    //printf("%d (%d) %d   ", row, group, col); 
    double x_offset = 0.0;
    if (row % 2 == 1) x_offset = 1.0;

    board[i].x = col * 2.0 + x_offset;
    board[i].y = BIG_R + row * 1.5 * BIG_R; 
    board[i].row = row;
    board[i].col = col;
    board[i].zone = get_zone(row, col);
    board[i].flags = 0x0;
    board[i].type = GRASS;
    board[i].unit = NULL;
    board[i].visited = 0;
    board[i].best_moves_left = 0;
    board[i].lowest_range = INT_MAX;
    //board[i].traversal_cost = 1;

    board[i].northwest = NULL;
    board[i].northeast = NULL;
    board[i].east = NULL;
    board[i].southeast = NULL;
    board[i].southwest = NULL;
    board[i].west = NULL;
  }

  // Link up to their neighbors
  for (int i = 0; i < 113; ++i) {
    int col = board[i].col;
    int row = board[i].row;
    int col_offset = 0;
    if (row % 2 == 1) {
      col_offset = 1; 
    }
    Hex* hex = NULL;
    if ((hex = get_by_row_col(board, row, col - 1)) != NULL) board[i].west = hex;
    if ((hex = get_by_row_col(board, row, col + 1)) != NULL) board[i].east = hex;
    if ((hex = get_by_row_col(board, row - 1, col + col_offset)) != NULL) board[i].northeast = hex;
    if ((hex = get_by_row_col(board, row - 1, col - 1 + col_offset)) != NULL) board[i].northwest = hex;
    if ((hex = get_by_row_col(board, row + 1, col + col_offset)) != NULL) board[i].southeast = hex;
    if ((hex = get_by_row_col(board, row + 1, col - 1 + col_offset)) != NULL) board[i].southwest = hex;
  }

  for (int i = 0; i < 113; ++i) {
    if (board[i].northwest) printf("%d %d NW-> %d %d\n", board[i].row, board[i].col, board[i].northwest->row, board[i].northwest->col);
    if (board[i].northeast) printf("%d %d NE-> %d %d\n", board[i].row, board[i].col, board[i].northeast->row, board[i].northeast->col);
    if (board[i].southwest) printf("%d %d SW-> %d %d\n", board[i].row, board[i].col, board[i].southwest->row, board[i].southwest->col);
    if (board[i].southeast) printf("%d %d SE-> %d %d\n", board[i].row, board[i].col, board[i].southeast->row, board[i].southeast->col);
    if (board[i].west) printf("%d %d W-> %d %d\n", board[i].row, board[i].col, board[i].west->row, board[i].west->col);
    if (board[i].east) printf("%d %d E-> %d %d\n", board[i].row, board[i].col, board[i].east->row, board[i].east->col);
  }

  board[0].best_moves_left = INT_MAX;
  //board[44].type = FOREST;
  board[31].type = FOREST;
  board[32].type = FOREST;
  //board[33].type = FOREST;
  board[70].type = FOREST;
  visit(&(board[45]), 2);

  range(&(board[45]), 0, 2);

  int count = 0;
  const Hex* in_range[64];
  for (int i = 0; i < 113; ++i) {
    if (board[i].lowest_range < INT_MAX && board[i].lowest_range > 0) {
      in_range[count++] = &board[i];
      in_range[count] = NULL;
    }
  }

  for (int i = 0; i < 360; ++i) {
    double x = cos(i / 180.0 * M_PI);
    double y = sin(i / 180.0 * M_PI);
    printf("%d deg atan2(%f, %f) => %f\n", i, y, x, atan2(y, x));
  }

  // test 180 degress blocking 45 -> Tree 44 -> 43
  // test half blocked 45 -> 33 & 32 -> 20

  //int line_of_sight(const Hex* current, const Hex* dest, const Hex* cells_in_range[], size_t num_cells) {
  int has_sight = line_of_sight(&board[45], &board[20], in_range, count);
  printf("%s\n", has_sight ? "Has line of sight" : "Does *not* have line of sight");

  // SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Could not initialize video or joystick%s\n", SDL_GetError());
    return 1;
  }

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
  SDL_Window* window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 780, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (window == NULL) {
    return 2;
  }

  SDL_GLContext* context = SDL_GL_CreateContext(window);
	if (context == NULL) {
    return 3;
  }

  // SDL_ttf
  TTF_Init();

  load_meshes();

  Entity* town_entity = (Entity*)malloc(sizeof(Entity));
  new_mesh_entity(town_entity, "hex.ply");
  g_entities = g_list_append(g_entities, (void*)town_entity);
  Entity town;
  new_mesh_entity(&town, "town.ply");
  g_entities = g_list_append(g_entities, (void*)&town);
  Entity* board_entity = (Entity*)malloc(sizeof(Entity));
  g_entities = g_list_append(g_entities, (void*)board_entity);
  Entity* text_entity = (Entity*)malloc(sizeof(Entity));
  g_entities = g_list_append(g_entities, (void*)text_entity);
  Entity* camera_entity = (Entity*)malloc(sizeof(Entity));
  g_entities = g_list_append(g_entities, (void*)camera_entity);

  Entity* camera_dolly_entity = (Entity*)malloc(sizeof(Entity));
  g_entities = g_list_append(g_entities, (void*)camera_dolly_entity);
  mat4_identity(camera_dolly_entity->transform);
  //mat4_translate(xform, forward, NULL);
  float pos[3] = {5.0f, 5.0f, 5.0f};
  mat4_translate(camera_dolly_entity->transform, pos, NULL);
  //add_child(camera_dolly_entity, camera_entity);

  // Setup
    Size size = new_text_quad_entity(text_entity, "Tree", 36, "fonts/Stencil WW II.ttf");
    printf("%d x %d\n", size.width, size.height);
    text_entity->transform[12] = 20.0f;
    text_entity->transform[13] = 20.0f;
    text_entity->transform[0] = size.width;
    text_entity->transform[5] = size.height;

    load_textures();

    compile_shaders("shaders_memoir44/model.vert", "shaders_memoir44/model_textured.frag", &texture_program);
    compile_shaders("shaders_memoir44/model.vert", "shaders_memoir44/model_colored.frag", &color_program);
    compile_shaders("shaders/quad.vert", "shaders/quad.frag", &quad_program);
    // Camera Setup
    init_camera(camera_entity);
        float zero[3] = {0.0f, 0.0f, 0.0f};

    // Board Mesh Setup
    Vertex vertices[7 * 113]; // 7 indices 2x13
    Face faces[6 * 113];
    int total = 0;
    for (int row = 0; row < 9; ++row) {
      float x_offset = row % 2 ? 0.86602540378f : 0.0f;
      int columns = row % 2 ? 12 : 13; 
      for (int i = 0; i < columns; ++i) {
        //int tile = i % 4;
        //int tile_row = tile / 2;
        //int tile_col = tile % 2;
        float u = 0.5f; //0.25f + tile_row * 0.5f;
        float v = 0.5f; //0.25f + tile_col * 0.5f;
        int index = (total + i) * 7;

        vertices[index].x = 2.0f * 0.86602540378f * i + x_offset;
        vertices[index].y = 0.0f;
        vertices[index].z = row * 1.5f;
        vertices[index].u = u;
        vertices[index].v = v;
        for (int j = 1; j < 7; ++j) {
          vertices[index + j].x = cos(j * M_PI / 180.0f * 60.0f + 0.523599f) + (2.0f * 0.86602540378f * i) + x_offset;
          vertices[index + j].y = 0.0f;
          vertices[index + j].z = sin(j * M_PI / 180.0f * 60.0f + 0.523599f) + row * 1.5f;
          vertices[index + j].u = cos(j * M_PI / 180.0f * 60.0f + 0.523599f) * 0.5f + 0.5f; //0.25f + u;
          vertices[index + j].v = sin(j * M_PI / 180.0f * 60.0f + 0.523599f) * 0.5f + 0.5f; //0.25f + v;
        }
      }
      total += columns;
    }
    for (int i = 0; i < 113; ++i) {
      int j = 0;
      for (j = 0; j < 6; ++j) {
        faces[i * 6 + j].first = i * 7;
        faces[i * 6 + j].second = i * 7 + j + 1;
        faces[i * 6 + j].third = i * 7 + j + 2;
      }
      // Fix up the last one
      faces[i * 6 + j - 1].third = i * 7 + 1;
    }
    create_mesh("models/board", vertices, 7 * 113, faces, 6 * 113);
    // Board Setup
    mat4_identity(board_entity->transform);
    board_entity->type = MODEL;
    board_entity->value.model.mesh_index = get_mesh_index("models/board");
    board_entity->value.model.texture_id = 5;
    // Look at board
    //move_to(camera_entity, 5.0f, 10.0f, 5.0f);
    float initial_camera_pos[3] = {-5.0f, -5.0f, -5.0f};
    mat4_identity(camera_entity->transform);
    mat4_translate(camera_entity->transform, initial_camera_pos, NULL);
    look_at_pos(camera_entity, zero);
  // End Setup

  // GL Global Setup
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);

  // Testing only
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  //init_input();
  bool run = true;
  float velocity = 0.0f;
  float strafe_velocity = 0.0f;

  while (run) {
    // -2. Reset input
    //g_input.start_pressed = false;

    // -1. Input
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        run = 0;
      }
      if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_f && text_entity) {
          free_text_entity(text_entity); 
          text_entity = NULL;
        }
      }

      if (event.type == SDL_MOUSEBUTTONDOWN) {
        printf(">>> %d %d\n", event.button.x, event.button.y);
        float x = (event.button.x - 320.0f) / 320.0f;
        float y = (event.button.y - 240.0f) / 240.0f;

        //* vec - vec3, screen-space vector to project
        //* view - mat4, View matrix
        //* proj - mat4, Projection matrix
        //* viewport - vec4, Viewport as given to gl.viewport [x, y, width, height]
        //* dest - Optional, vec3_t receiving unprojected result. If NULL, result is written to vec
        // See https://github.com/toji/gl-matrix/commit/95d06c32e52bbe43aba811a49ed4e32cf2d1fea7#diff-e083d9f38382848a77149f396563edabR256
        float l[3] = {event.button.x, 780.0f - event.button.y, 1.0f};
        float viewport[4] = {0.0f, 0.0f, 1024.0f, 780.0f};
        vec3_unproject(l, camera_entity->transform, g_mat_perspective, viewport, NULL);
        //float eye[3] = {camera.transform[12], camera.transform[13], camera.transform[14]};
        float eye[3] = {event.button.x, 780.0f - event.button.y, 0.0f};
        vec3_unproject(eye, camera_entity->transform, g_mat_perspective, viewport, NULL);

        // p0 = point in plane
        // n = normal
        // l = vector in direction of line
        // l0 = point on line
        // d = (p0 - l0) . n / ( l . n )
        // point on plane = dl + l0
        vec3_normalize(l, NULL);

        float plane_point[3] = {0.0f, 0.0f, 0.0f};
        float plane_normal[3] = {0.0f, 1.0f, 0.0f};

        float diff[3];
        vec3_subtract(plane_point, eye, diff);
        //vec3_normalize(diff, NULL);
        float d = vec3_dot(diff, plane_normal) / vec3_dot(l, plane_normal);
        vec3_scale(l, d, NULL);
        vec3_add(l, eye, NULL);
        printf("d: %f intersection: (%f, %f, %f)\n", d, l[0], l[1], l[2]);

        //move_to(board_entity, l[0], l[1], l[2]);
        //look_at_pos(camera_entity, l);
        //move_to(camera_entity, 5.0f, 10.0f, 5.0f);
        look_at_pos(camera_entity, l);
      }
      //process_joystick_input(&event);
      //process_keyboard_input(&event);
    }

    // Camera movement
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    //if (state[SDL_SCANCODE_RIGHT] && state[SDL_SCANCODE_UP]) {
    if (state[SDL_SCANCODE_W]) {
      velocity += 0.05f;
    }
    if (state[SDL_SCANCODE_S]) {
      velocity -= 0.05f;
    }
    if (state[SDL_SCANCODE_A]) {
      strafe_velocity += 0.05f;
    }
    if (state[SDL_SCANCODE_D]) {
      strafe_velocity -= 0.05f;
    }

    forward(camera_entity, velocity);
    strafe(camera_entity, strafe_velocity);
    velocity *= 0.9f;
    strafe_velocity *= 0.9f;

		glClearColor(11 / 255.0f, 102 / 255.0f, 35 / 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Background
		//glClear(GL_DEPTH_BUFFER_BIT);

    // 2. Render all entities
    // void render_model(const Entity* entity, vec3_t out, GLuint mode, GLuint texture) {
    for (int i = 0; i < g_list_length(g_entities); ++i) {
      Entity* entity = (Entity*)g_list_nth_data(g_entities, i);
      switch (entity->type) {
        case QUAD:
          render_quad(entity, true);
          break;
        case MODEL:
          render_model(entity, NULL, GL_TRIANGLES);
          break;
        case NODE:
        case SPRITE:
        case CAMERA:
          break;
        //default:
          //fprintf(stderr, "[WARNING] Not rendering\n");
      }
    }
    //render_model(&g_board, NULL, GL_TRIANGLES, 0);
    //render_quad(&g_text, true);

    // Render scaled full-screen quad

    SDL_GL_SwapWindow(window);
    SDL_Delay(17);
  }

  //SDL_JoystickClose(joystick);
  //joystick = NULL;

  //Destroy window
  SDL_DestroyWindow(window);
  window = NULL;

  SDL_Quit();
}
