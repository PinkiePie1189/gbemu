#include "error.h"
#include <stdlib.h>

void file_check_error(FILE *fd, char *filename) {
  if (fd == NULL) {
    fprintf(stderr, "Can't open file %s\n", filename);
    exit(-1);
  }
}
