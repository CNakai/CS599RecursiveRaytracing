#include <stddef.h>

typedef struct PixelBuf* PixelBufRef;

PixelBufRef new_pixel_buf(int, int);
void color_pixel(PixelBufRef, double*, int, int);
uint8_t* get_byte_array(PixelBufRef);

