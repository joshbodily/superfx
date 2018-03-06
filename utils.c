#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

char* get_file_data(const char* path, int* length) {
	FILE* f = fopen(path, "rb");
  if (f) {
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buffer = malloc(*length);
    if (buffer) {
      fread(buffer, 1, *length, f);
    }
    buffer[*length] = '\0';
    fclose(f);
    return buffer;
  }
  return NULL;
}
