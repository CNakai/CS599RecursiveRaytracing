#include <stdio.h>
#include <stdlib.h>
#include "spec.h"
#include "parser.h"

int main(int argc, char* argv[]) {
  Scene scene = parse_scene_from_file("test_data/inputs/sphere_and_plane.json");
  print_scene(scene);
  exit(EXIT_SUCCESS);
  return 0;
}
