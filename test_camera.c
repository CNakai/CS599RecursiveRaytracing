#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "spec.h"
#include "camera.h"

int main(int argc, char* argv[]) {
  Scene scene = parse_scene_from_file("test_data/inputs/sphere_and_plane.json");
  CameraRef camera = get_camera_from_scene(scene);
  print_camera(camera);
  exit(EXIT_SUCCESS);
}
