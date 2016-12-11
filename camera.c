#include <stdlib.h>
#include <stdio.h>
#include "camera.h"
#include "spec.h"
#include "vecmath.h"
#include "util.h"

//////////////////// Forward Declarations ////////////////////
static CameraRef new_camera_from_spec(SpecRef);
static CameraRef new_camera(void);
static void validate_camera(CameraRef);
//////////////////////////////////////////////////////////////

/* Assumed to be at position (0, 0, 0), with facing normal (0, 0, 1), and that the view plane is
 * forward of the camera by one unit */
CameraRef get_camera_from_scene(Scene scene) {
  SpecRef camera_spec = next_spec_declaring_kind(scene, "camera");
  if(NULL == camera_spec) {
    fprintf(stderr, "Error: There was no camera in the scene file\n");
    exit(EXIT_FAILURE);
  }

  CameraRef c = new_camera_from_spec(camera_spec);

  return c;
}


double get_camera_width(CameraRef c) {
  return c->width;
}


double get_camera_height(CameraRef c) {
  return c->height;
}


void get_camera_position(CameraRef c, Point out) {
  out[0] = c->position[0];
  out[1] = c->position[1];
  out[2] = c->position[2];
}


void get_camera_facing(CameraRef c, Point out) {
  out[0] = c->position[0];
  out[1] = c->position[1];
  out[2] = c->position[2];
}


void get_viewplane_center(CameraRef c, Vec out) {
  double *vpc = checked_malloc(sizeof(*vpc));
  Vec focal_length_scaled_facing_vector = {0.0, 0.0, 0.0};
  vec_scale(c->facing, c->focal_length, focal_length_scaled_facing_vector);
  vec_add(c->position, focal_length_scaled_facing_vector, out);
}


void get_viewplane_unit_vectors(CameraRef c, Vec x, Vec y, Vec z) {
  Vec intermediate = {0.0};
  vec_cross(c->facing, c->up, intermediate);
  if(0 == intermediate[0] && 0 == intermediate[1] && 0 == intermediate[2]) {
    report_error_and_exit("Theta between the camera facing and up vectors must be 0 < theta < 90");
  }
  vec_normalize(intermediate, x);
  vec_scale(c->facing, -1.0, z);
  vec_cross(z, x, y);
}


void print_camera(CameraRef c) {
  printf("\nCamera:\n\tPosition: [%f, %f, %f]\n\tFacing: [%f, %f, %f]\n\tUp: [%f, %f, %f]\n",
	 c->position[0], c->position[1], c->position[2],
	 c->facing[0], c->facing[1], c->facing[2],
	 c->up[0], c->up[1], c->up[2]);
  printf("\tWidth: %f\n\tHeight: %f\n\tFocal Length: %f\n\n", c->width, c->height, c->focal_length);
}
//////////////////////////////////////////////////////////


//////////////////// Static Functions ////////////////////
static CameraRef new_camera_from_spec(SpecRef camera_spec) {
  CameraRef c = new_camera();
  c->width = next_scalar_field_value_with_name(camera_spec, "width");
  c->height = next_scalar_field_value_with_name(camera_spec, "height");
  c->position = next_vector_field_value_with_name(camera_spec, "position");
  c->facing = next_vector_field_value_with_name(camera_spec, "facing");
  c->up = next_vector_field_value_with_name(camera_spec, "up");
  c->focal_length = next_scalar_field_value_with_name(camera_spec, "focal_length");

  validate_camera(c);
  destroy_spec(camera_spec);

  return c;
}


static CameraRef new_camera() {
  Camera zero_camera = {0};
  CameraRef c = checked_malloc(sizeof(*c));
  *c = zero_camera;
  return c;
}


static void validate_camera(CameraRef c) {
  Vec intermediate = {0.0};

  if(0 >= c->width) {
    fprintf(stderr, "Error: Camera has width <= 0\n");
    exit(EXIT_FAILURE);
  }

  if(0 >= c->height) {
    fprintf(stderr, "Error: Camera has height <= 0\n");
    exit(EXIT_FAILURE);
  }

  if(NULL == c->position) {
    c->position = malloc(sizeof(double) * 3);
    c->position[0] = 0.0;
    c->position[1] = 0.0;
    c->position[2] = 0.0;
#ifdef DEBUG_NOTICE
    fprintf(stderr, "NOTICE: No value specified for camera position\n");
    fprintf(stderr, "NOTICE: Defaulting to (0, 0, 0)\n");
#endif
  }

  if(NULL == c->facing) {
    c->facing = malloc(sizeof(double) * 3);
    c->facing[0] = 0.0;
    c->facing[1] = 0.0;
    c->facing[2] = 1.0;
#ifdef DEBUG_NOTICE
    fprintf(stderr, "NOTICE: No value specified for camera facing vector\n");
    fprintf(stderr, "NOTICE: Defaulting to (0, 0, 1)\n");
#endif
  } else {
    vec_normalize(c->facing, intermediate);
    vec_copy(intermediate, c->facing);
  }

  if(NULL == c->up) {
    c->up = malloc(sizeof(double) * 3);
    c->up[0] = 0.0;
    c->up[1] = 1.0;
    c->up[2] = 0.0;
#ifdef DEBUG_NOTICE
    fprintf(stderr, "NOTICE: No value specified for camera up vector\n");
    fprintf(stderr, "NOTICE: Defaulting to (0, 1, 0)\n");
#endif
  } else {
    vec_normalize(c->up, intermediate);
    vec_copy(intermediate, c->up);
  }

  if(NO_SCALAR == c->focal_length) {
    c->focal_length = 1.0;
#ifdef DEBUG_NOTICE
    fprintf(stderr, "NOTICE: No value specified for camera focal length\n");
    fprintf(stderr, "NOTICE: Defaulting to 1.0\n");
#endif
  } else if(0 >= c->focal_length) {
    fprintf(stderr, "ERROR: The camera focal length must be a positive real value\n");
    exit(EXIT_FAILURE);
  }
}
