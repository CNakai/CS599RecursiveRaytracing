#include <stdio.h>
#include <stdlib.h>
#include "spec.h"
#include "parser.h"
#include "light.h"

int main(int argc, char* argv[]) {
  Scene scene = parse_scene_from_file("test_data/inputs/sphere_and_plane_and_light.json");
  LightRef *lights = get_lights_from_scene(scene);
  print_lights(lights);
  exit(EXIT_SUCCESS);
}
