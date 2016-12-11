#ifndef OBJECT_HEADER
#define OBJECT_HEADER 1

#include <math.h>
#include "spec.h"
#include "vecmath.h"

#define MISS INFINITY

enum ObjectKind { NoObjKind, Plane, Sphere, Quadric };

struct Object {
  enum ObjectKind kind;
  double *diffuse_color;
  double *specular_color;
  double ns;
  union {
    struct {
      double *position;
      double *normal;
    } plane;
    struct {
      double *position;
      double radius;
    } sphere;
    struct {
      double *parts;
    } quadric;
  };
};

typedef struct Object Object;
typedef struct Object* ObjectRef;

ObjectRef* get_objects_from_scene(Scene);
double has_intersection(RayRef, ObjectRef);
void get_surface_normal(ObjectRef, double*, double*);
void print_objects(ObjectRef*);
void print_object(ObjectRef);

#endif
