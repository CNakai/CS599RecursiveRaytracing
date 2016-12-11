#include <stdio.h>
#include <stdlib.h>
#include "spec.h"
#include "parser.h"
#include "object.h"

int main(int argc, char* argv[]) {
  Scene scene = parse_scene_from_file("test_data/inputs/sphere_and_plane.json");
  ObjectRef* objects = get_objects_from_scene(scene);
  print_objects(objects);
  exit(EXIT_SUCCESS);
}
