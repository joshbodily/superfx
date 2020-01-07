#include <readline/readline.h>
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

static char *line_read = (char *)NULL;

/* Read a string, and return a pointer to it.  Returns NULL on EOF. */
char * rl_gets () {
  /* If the buffer has already been allocated, return the memory
   *      to the free pool. */
  if (line_read)
  {
    free (line_read);
    line_read = (char *)NULL;
  }

  /* Get a line from the user. */
  line_read = readline ("> ");

  /* If the line has any text in it, save it on the history. */
  if (line_read && *line_read)
    add_history (line_read);

  return (line_read);
}

int main() {
  char* myfifo = "/tmp/myfifo";
  mkfifo(myfifo, 0666);

  sleep(5);

  int fd;
  fd = open(myfifo, O_WRONLY);
  assert(fd != -1);

  while (true) {
    char * line = rl_gets();
    write(fd, line, strlen(line));
    write(fd, "\n", 1);
    fsync(fd);
  }

  close(fd);
}
