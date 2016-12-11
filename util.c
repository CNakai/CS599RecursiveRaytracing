#include <stdlib.h>
#include <stdio.h>
#include "util.h"

void secret_DEBUG_LOG(char *message) {
  fprintf(stderr, "DEBUG: %s\n", message);
}

void* checked_malloc(size_t size) {
  void* ret = malloc(size);
  if(NULL == ret && size != 0) { report_error_and_exit("NULL result from malloc on non-zero input"); }
  return ret;
}

void report_error_and_exit(char *error_msg) {
  fprintf(stderr, "Error: %s\n", error_msg);
  exit(EXIT_FAILURE);
}
