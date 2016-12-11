#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void ppm_write(char* outfile_name, char format_flag, uint8_t *buf, int width, int height) {
  if(NULL == buf) {
    fprintf(stderr, "ERROR: The buffer passed to ppm_write was NULL\n");
    exit(EXIT_FAILURE);
  }

  FILE *output_file = fopen(outfile_name, "w");
  if(NULL == output_file) {
    fprintf(stderr, "ERROR: Output file could not be opened for writing\n");
    exit(EXIT_FAILURE);
  }

  if(format_flag != '3' && format_flag != '6') {
    fprintf(stderr, "ERROR: The format flag passed to ppm_write must be '3' or '6'\n");
    exit(EXIT_FAILURE);
  }

  if(0 > fprintf(output_file, "P%c\n%d %d\n255\n", format_flag, width, height)) {
    fprintf(stderr, "ERROR: An error occurred while writing to the output file 1\n");
    exit(EXIT_FAILURE);
  }

  int buf_len = sizeof(uint8_t) * width * height * 3;
  if(format_flag == '3') {
    for(int i = 0; i < buf_len; i++) {
      if(i % width == 0 && 0 > fprintf(output_file, "\n")) {
	fprintf(stderr, "ERROR: An error occurred while writing to the output file 2\n");
	exit(EXIT_FAILURE);
      }
      if(0 > fprintf(output_file, "%u ", buf[i])) {
	fprintf(stderr, "ERROR: An error occurred while writing to the output file 3\n");
	exit(EXIT_FAILURE);
      }
    }
  }
  if(format_flag == '6' && !((size_t) buf_len == fwrite(buf, sizeof(uint8_t), buf_len, output_file))) {
    fprintf(stderr, "ERROR: An error occurred while writing to the output file 4\n");
    exit(EXIT_FAILURE);
  }

  fclose(output_file);
  return;
}
