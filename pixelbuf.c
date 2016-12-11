#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "pixelbuf.h"
#include "vecmath.h"
#include "util.h"

struct PixelBuf {
  int width;
  int height;
  uint8_t *buf;
};

typedef struct PixelBuf PixelBuf;

static uint8_t scale_double(double);

PixelBufRef new_pixel_buf(int width, int height) {
  PixelBufRef pbr = checked_malloc(sizeof(*pbr));
  pbr->width = width;
  pbr->height = height;
  pbr->buf = checked_malloc((sizeof(*(pbr->buf)) * 3) * width * height);
  return pbr;
}

void color_pixel(PixelBufRef pbr, double *color, int row, int col) {
  if(row < 0 || col < 0 || row > pbr->height || col > pbr->width) {
    fprintf(stderr, "Error: Illegal coordinates passed to color_pixel:\n");
    fprintf(stderr, "Error: \t\trow: %d  col: %d\t\t max_row: %d  max_col: %d\n", row, col, pbr->height, pbr->width);
    exit(EXIT_FAILURE);
  }
  int row_offset = (pbr->height - 1 - row) * pbr->width * 3;
  int col_offset = col * 3;
  int total_offset = row_offset + col_offset;
  pbr->buf[total_offset] = scale_double(color[0]);
  pbr->buf[total_offset + 1] = scale_double(color[1]);
  pbr->buf[total_offset + 2] = scale_double(color[2]);
}

uint8_t* get_byte_array(PixelBufRef pbr) {
  size_t buf_len = pbr->width * pbr->height * 3;
  uint8_t *out_arr = checked_malloc(sizeof(*(pbr->buf)) * buf_len);
  memcpy(out_arr, pbr->buf, buf_len);
  return out_arr;
}

static uint8_t scale_double(double d) {
  d = d <= 1.0 ? d : 1.0;
  double intermediate = floor(d * 255.0);
  return (uint8_t) (intermediate > 255 ? 255 : intermediate);
}
