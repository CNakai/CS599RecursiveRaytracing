#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <math.h>
#include "light.h"
#include "spec.h"
#include "vecmath.h"
#include "util.h"


#define DEG_TO_RAD_CONV_FACTOR (3.14159265358979323846 / 180.0)
#define MAX_LIGHTS 128

static LightRef get_next_light_from_scene(Scene);
static void validate_light(LightRef);
static bool is_spotlight(LightRef);
void get_common_contrib(LightRef, double*, double*);

LightRef* get_lights_from_scene(Scene scene) {
  LightRef* lights = checked_malloc(sizeof(*lights) * (MAX_LIGHTS + 1));
  int i = 0;
  LightRef last_got_light = get_next_light_from_scene(scene);
  while(NULL != last_got_light && i < MAX_LIGHTS) {
    lights[i] = last_got_light;
    last_got_light = get_next_light_from_scene(scene);
    i++;
  }
  lights[i] = NULL;

  return lights;
}


bool light_is_contributing(LightRef light, double *intersectward_n) {
  return true;
  if(!is_spotlight(light)) {
    return true;
  } else {
    return !(acos(vec_dot(intersectward_n, light->direction)) > light->theta / 2.0);
  }
}


void get_diffuse_contrib(LightRef light, double *intersectward_n, double *surface_n, double *out) {
  get_common_contrib(light, intersectward_n, out);
  vec_scale(out, -vec_dot(intersectward_n, surface_n), out);
}


void get_specular_contrib(LightRef light, double *intersectward_n, double *surface_n, double *view_n,
			  double ns, double *out) {
  get_common_contrib(light, intersectward_n, out);
  double reflection_n[3] = {0.0};
  vec_reflect(intersectward_n, surface_n, reflection_n);
  double intermediate_dot = vec_dot(reflection_n, view_n);
  intermediate_dot = intermediate_dot < 0 ? 0 : intermediate_dot;
  vec_scale(out, pow(intermediate_dot, ns), out);
}


void get_common_contrib(LightRef light, double *intersectward_n, double *out) {
  vec_copy(light->color, out);
  if(is_spotlight(light)) {
    vec_scale(out, pow(vec_dot(intersectward_n, light->direction), light->angular_a0), out);
  }
}


void attenuate_radially(LightRef light, double dist, double *diffuse_contrib, double *specular_contrib) {
  double radial_s = 1.0 / (pow(dist, 2) * light->radial_a2 + dist * light->radial_a1 + light->radial_a0);
  vec_scale(diffuse_contrib, radial_s, diffuse_contrib);
  vec_scale(specular_contrib, radial_s, specular_contrib);
}


void print_lights(LightRef* lights) {
  LightRef l;
  bool l_is_spotlight = false;
  while(NULL != (l = *lights)) {
    l_is_spotlight = is_spotlight(l);
    printf("\n");
    printf("Light:\n");
    printf("\tPosition: [%f, %f, %f]\n", l->position[0], l->position[1], l->position[2]);
    if(l_is_spotlight) {
      printf("\tDirection: [%f, %f, %f]\n", l->direction[0], l->direction[1], l->direction[2]);
    }
    printf("\tColor: [%f, %f, %f]\n", l->color[0], l->color[1], l->color[2]);
    printf("\tRadial-a0: %f\n", l->radial_a0);
    printf("\tRadial-a1: %f\n", l->radial_a1);
    printf("\tRadial-a2: %f\n", l->radial_a2);
    if(l_is_spotlight) {
      printf("\tTheta: %f\n", l->theta);
      printf("\tAngular-a0: %f\n", l->angular_a0);
    }
    lights++;
  }
}


static LightRef get_next_light_from_scene(Scene scene) {
  SpecRef light_spec = next_spec_declaring_kind(scene, "light");
  if(NULL == light_spec) return NULL;

  Light zero_light = {0};
  LightRef l = checked_malloc(sizeof(*l));
  *l = zero_light;
  l->position = next_vector_field_value_with_name(light_spec, "position");
  l->color = next_vector_field_value_with_name(light_spec, "color");
  l->radial_a0 = next_scalar_field_value_with_name(light_spec, "radial-a0");
  l->radial_a1 = next_scalar_field_value_with_name(light_spec, "radial-a1");
  l->radial_a2 = next_scalar_field_value_with_name(light_spec, "radial-a2");
  l->direction = next_vector_field_value_with_name(light_spec, "direction");
  l->theta = next_scalar_field_value_with_name(light_spec, "theta");
  l->angular_a0 = next_scalar_field_value_with_name(light_spec, "angular_a0");

  validate_light(l);
  destroy_spec(light_spec);

  return l;
}


static bool is_spotlight(LightRef l) {
  return (NO_SCALAR != l->theta) && (0 != l->theta);
}


static void validate_light(LightRef l) {
  if(NULL == l->position) {
    fprintf(stderr, "Error: No position specified for light\n");
    exit(EXIT_FAILURE);
  }
  if(NULL == l->color) {
    fprintf(stderr, "Error: No color specified for light\n");
    exit(EXIT_FAILURE);
  }
  if(NO_SCALAR == l->radial_a0) {
    fprintf(stderr, "Error: No radial-a0 specified for light\n");
    exit(EXIT_FAILURE);
  }
  if(NO_SCALAR == l->radial_a1) {
    fprintf(stderr, "Error: No radial-a1 specified for light\n");
    exit(EXIT_FAILURE);
  }
  if(NO_SCALAR == l->radial_a2) {
    fprintf(stderr, "Error: No radial-a2 specified for light\n");
    exit(EXIT_FAILURE);
  }
  if((l->theta != NO_SCALAR && l->theta != 0) || l->direction != NULL || l->angular_a0 != NO_SCALAR) {
    if(NO_SCALAR == l->theta) {
      fprintf(stderr, "Error: No theta specified for spot-light\n");
      exit(EXIT_FAILURE);
    } else {
      l->theta = DEG_TO_RAD_CONV_FACTOR * l->theta;
    }

    if(NULL == l->direction) {
      fprintf(stderr, "Error: No direction vector specified for spot-light\n");
      exit(EXIT_FAILURE);
    }

    if(NO_SCALAR == l->angular_a0) {
      fprintf(stderr, "Error: No angular-a0 specified for spot-light\n");
      exit(EXIT_FAILURE);
    }
  }
}
