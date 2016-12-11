#ifndef LIGHT_HEADER
#define LIGHT_HEADER 1

#include <stdbool.h>
#include "vecmath.h"
#include "spec.h"

struct Light {
  double *position;
  double *color;
  double radial_a0;
  double radial_a1;
  double radial_a2;
  // Spotlights only
  double *direction;
  double theta;
  double angular_a0;
};

typedef struct Light Light;
typedef struct Light * LightRef;

LightRef* get_lights_from_scene(Scene);
void illumination_for_light(LightRef, double, double*, double*, double*, double*, double*);
bool light_is_contributing(LightRef, double*);
void get_diffuse_contrib(LightRef, double*, double*, double*);
void get_specular_contrib(LightRef, double*, double*, double*, double, double*);
void attenuate_radially(LightRef, double, double*, double*);
void print_lights(LightRef*);

#endif
