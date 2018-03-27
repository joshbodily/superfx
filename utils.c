#include "superfx.h"
#include "utils.h"

char* get_file_data(const char* path, int* length) {
	FILE* file = fopen(path, "rb");
  if (file) {
    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(*length);
    if (buffer) {
      fread(buffer, 1, *length, file);
    } else {
      SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Could not allocate data for file %s", path);
    }
    buffer[*length] = '\0';
    fclose(file);
    return buffer;
  }
  SDL_LogError(SDL_LOG_CATEGORY_ASSERT, "Could not open file %s", path);
  return NULL;
}
