#ifndef CAMERA_HEADER
#define CAMERA_HEADER 1


#include "spec.h"
#include "vecmath.h"

struct Camera {
  double width;
  double height;
  double focal_length;
  double *position;
  double *facing;
  double *up;
};

typedef struct Camera Camera;
typedef struct Camera* CameraRef;

CameraRef get_camera_from_scene(Scene);
double get_camera_width(CameraRef);
double get_camera_height(CameraRef);
void get_camera_position(CameraRef, Point);
void get_camera_facing(CameraRef, Point);
void get_viewplane_center(CameraRef, Vec);
void get_viewplane_unit_vectors(CameraRef, Vec, Vec, Vec);
void print_camera(CameraRef);
#endif
