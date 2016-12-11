#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "parser.h"
#include "spec.h"
#include "camera.h"
#include "object.h"
#include "light.h"
#include "pixelbuf.h"
#include "raycast.h"
#include "ppmwrite.h"

static void validate_argc(int);
static void initializes_static_vars(char**);

static int width;
static int height;
static char* input_file_name;
static char* output_file_name;

int main(int argc, char* argv[]) {
  validate_argc(argc);
  initializes_static_vars(argv);
  Scene scene = parse_scene_from_file(input_file_name);
  CameraRef camera = get_camera_from_scene(scene);
  ObjectRef *objects = get_objects_from_scene(scene);
  LightRef *lights = get_lights_from_scene(scene);
  PixelBufRef pixel_buf = raycast(camera, objects, lights, width, height); //REFACTOR: Too many params!
  uint8_t *byte_buf = get_byte_array(pixel_buf);
  ppm_write(output_file_name, '3', byte_buf, width, height);

  exit(EXIT_SUCCESS);
}


static void validate_argc(int argc) {
  if(argc != 5) {
    fprintf(stderr, "ERROR: You supplied an incorrect number of arguments.\n");
    fprintf(stderr, "ERROR: Correct usage is:\n");
    fprintf(stderr, "ERROR: \traycast width height input_file.json output_file.ppm\n");
    exit(EXIT_FAILURE);
  }
}

static void initializes_static_vars(char *argv[]) {
  width = (int) strtol(argv[1], NULL, 10);
  height = (int) strtol(argv[2], NULL, 10);
  if(width <= 0 || height <= 0) {
    fprintf(stderr, "ERROR: The supplied dimensions must be positive integers\n");
    exit(EXIT_FAILURE);
  }

  input_file_name = argv[3];
  output_file_name = argv[4];
}
