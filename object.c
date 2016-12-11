#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "object.h"
#include "spec.h"
#include "vecmath.h"
#include "util.h"

#define MAX_OBJECTS 128

#define A 0
#define B 1
#define C 2
#define D 3
#define E 4
#define F 5
#define G 6
#define H 7
#define I 8
#define J 9

//////////////////// Forward Declarations ////////////////////
static ObjectRef get_next_plane_from_scene(Scene);
static void validate_plane(ObjectRef);
static ObjectRef get_next_sphere_from_scene(Scene);
static void validate_sphere(ObjectRef);
static ObjectRef get_next_quadric_from_scene(Scene);
static void validate_quadric(ObjectRef);
static ObjectRef new_object_from_spec(SpecRef);
static double* get_diffuse_color_from_spec(SpecRef);
static double* get_specular_color_from_spec(SpecRef);
static double get_ns(SpecRef);
static void validate_object(ObjectRef);
static double plane_intersection(RayRef, ObjectRef);
static double sphere_intersection(RayRef, ObjectRef);
static double quadric_intersection(RayRef, ObjectRef);
static void get_quadric_surface_normal(ObjectRef, double *, double *);
static void get_sphere_surface_normal(ObjectRef, double *, double *);
static void get_plane_surface_normal(ObjectRef, double *);


//////////////////// Public Functions ////////////////////
ObjectRef* get_objects_from_scene(Scene scene) {
  ObjectRef* objects = checked_malloc((MAX_OBJECTS + 1) * sizeof(*objects));
  ObjectRef last_got_o;
  int i = 0;
  last_got_o = get_next_plane_from_scene(scene);
  while(NULL != last_got_o && i < MAX_OBJECTS) {
    objects[i] = last_got_o;
    last_got_o = get_next_plane_from_scene(scene);
    i++;
  }
  last_got_o = get_next_sphere_from_scene(scene);
  while(NULL != last_got_o && i < MAX_OBJECTS) {
    objects[i] = last_got_o;
    last_got_o = get_next_sphere_from_scene(scene);
    i++;
  }
  last_got_o = get_next_quadric_from_scene(scene);
  while(NULL != last_got_o && i < MAX_OBJECTS) {
    objects[i] = last_got_o;
    last_got_o = get_next_quadric_from_scene(scene);
    i++;
  }
  objects[i] = NULL;

  return objects;
}


const double* get_diffuse_color(ObjectRef o) {
  return o->diffuse_color; 
}


double has_intersection(RayRef ray, ObjectRef o) {
  switch(o->kind) {
  case Plane:
    return plane_intersection(ray, o);
  case Sphere:
    return sphere_intersection(ray, o);
  case Quadric:
    return quadric_intersection(ray, o);
  case NoObjKind:
    fprintf(stderr, "ERROR: Tried to check for intersection with unknown object type\n");
    exit(EXIT_FAILURE);
  }
  fprintf(stderr, "INSANITY: The impossible has occurred!\n");
  exit(EXIT_FAILURE);
}

void get_surface_normal(ObjectRef o, double *point, double *out) {
  switch(o->kind) {
  case Plane:
    get_plane_surface_normal(o, out);
    return;
  case Sphere:
    get_sphere_surface_normal(o, point, out);
    return;
  case Quadric:
    get_quadric_surface_normal(o, point, out);
    return;
  case NoObjKind:
    fprintf(stderr, "ERROR: Tried to get surface normal of an unknown object type\n");
    exit(EXIT_FAILURE);
  }
  fprintf(stderr, "INSANITY: The impossible has occurred!\n");
  exit(EXIT_FAILURE);
}


void print_objects(ObjectRef* objects) {
  while(NULL != *objects) {
    printf("\n");
    print_object(*objects);
    objects++;
  }
}


void print_object(ObjectRef o) {
    switch(o->kind) {
    case Plane:
      printf("Plane:\n\tPosition: [%f, %f, %f]\n\tNormal: [%f, %f, %f]\n\n",
	     o->plane.position[0], o->plane.position[1], o->plane.position[2],
	     o->plane.normal[0], o->plane.normal[1], o->plane.normal[2]);
      break;
    case Sphere:
      printf("Sphere:\n\tPosition: [%f, %f, %f]\n\tRadius: %f\n\n",
	     o->sphere.position[0], o->sphere.position[1], o->sphere.position[2],
	     o->sphere.radius);
      break;
    case Quadric:
      printf("Quadric:\n\tA: %f\n\tB: %f\n\tC: %f\n\tD: %f\n\tE: %f\n\tF: %f\n\tG: %f\n\tH: %f\n\tI: %f\n\tJ: %f\n\n",
	     o->quadric.parts[0], o->quadric.parts[1], o->quadric.parts[2],
	     o->quadric.parts[3], o->quadric.parts[4], o->quadric.parts[5],
	     o->quadric.parts[6], o->quadric.parts[7], o->quadric.parts[8],
	     o->quadric.parts[9]); 
      break;
    case NoObjKind:
      printf("Uh oh!\n");
      break;
    }
    printf("\tDiffuseColor: [%f, %f, %f]\n\tSpecularColor: [%f, %f, %f]\n\tNS: %f\n",
	   o->diffuse_color[0], o->diffuse_color[1], o->diffuse_color[2], 
	   o->specular_color[0], o->specular_color[1], o->specular_color[2], 
	   o->ns);
}
//////////////////////////////////////////////////////////

static ObjectRef get_next_plane_from_scene(Scene scene) {
  SpecRef spec = next_spec_declaring_kind(scene, "plane");
  if(NULL == spec) {
    return NULL;
  }

  ObjectRef p = new_object_from_spec(spec);
  p->kind = Plane;
  p->plane.position = next_vector_field_value_with_name(spec, "position");
  p->plane.normal = next_vector_field_value_with_name(spec, "normal");

  validate_plane(p);
  destroy_spec(spec);

  return p;
}

static void validate_plane(ObjectRef p) {
  if(Plane != p->kind) {
    fprintf(stderr, "Error: Plane was somehow... not... a plane...\n");
    exit(EXIT_FAILURE);
  }
  if(NULL == p->plane.position) {
    fprintf(stderr, "Error: Plane has no position\n");
    exit(EXIT_FAILURE);
  }
  if(NULL == p->plane.normal) {
    fprintf(stderr, "Error: Plane has no normal\n");
    exit(EXIT_FAILURE);
  }
}


static ObjectRef get_next_sphere_from_scene(Scene scene) {
  SpecRef spec = next_spec_declaring_kind(scene, "sphere");
  if(NULL == spec) return NULL;

  ObjectRef s = new_object_from_spec(spec);
  s->kind = Sphere;
  s->sphere.position = next_vector_field_value_with_name(spec, "position");
  s->sphere.radius = next_scalar_field_value_with_name(spec, "radius");

  validate_sphere(s);
  destroy_spec(spec);

  return s;
}

static void validate_sphere(ObjectRef s) {
  if(Sphere != s->kind) {
    fprintf(stderr, "Error: Sphere was somehow... not... a sphere...\n");
    exit(EXIT_FAILURE);
  }
  if(NULL == s->sphere.position) {
    fprintf(stderr, "Error: Sphere has no position\n");
    exit(EXIT_FAILURE);
  }
  if(NO_SCALAR == s->sphere.radius) {
    fprintf(stderr, "Error: Sphere has no radius\n");
    exit(EXIT_FAILURE);
  }
}

static ObjectRef get_next_quadric_from_scene(Scene scene) {
  SpecRef spec = next_spec_declaring_kind(scene, "quadric");
  if(NULL == spec) return NULL;

  ObjectRef q = new_object_from_spec(spec);
  q->kind = Quadric;
  q->quadric.parts = checked_malloc(10 * sizeof(*(q->quadric.parts)));
  q->quadric.parts[0] = next_scalar_field_value_with_name(spec, "A");
  q->quadric.parts[1] = next_scalar_field_value_with_name(spec, "B");
  q->quadric.parts[2] = next_scalar_field_value_with_name(spec, "C");
  q->quadric.parts[3] = next_scalar_field_value_with_name(spec, "D");
  q->quadric.parts[4] = next_scalar_field_value_with_name(spec, "E");
  q->quadric.parts[5] = next_scalar_field_value_with_name(spec, "F");
  q->quadric.parts[6] = next_scalar_field_value_with_name(spec, "G");
  q->quadric.parts[7] = next_scalar_field_value_with_name(spec, "H");
  q->quadric.parts[8] = next_scalar_field_value_with_name(spec, "I");
  q->quadric.parts[9] = next_scalar_field_value_with_name(spec, "J");

  validate_quadric(q);
  destroy_spec(spec);

  return q;
}

static void validate_quadric(ObjectRef q) {
  if(Quadric != q->kind) {
    fprintf(stderr, "Error: Quadric was somehow... not... a quadric...\n");
    exit(EXIT_FAILURE);
  }

  for(int i = 0; i < 10; i++) {
    if(NO_SCALAR == q->quadric.parts[i]) {
      #ifdef DEBUG_NOTICE
      fprintf(stderr, "NOTICE: No '%c' was assigned for quadric\n", (char) 65 + i);
      fprintf(stderr, "NOTICE: Assigning 0 as a default\n");
      #endif
      q->quadric.parts[i] = 0;
    }
  }
}


static ObjectRef new_object_from_spec(SpecRef osr) {
  Object zero_object = {0};
  ObjectRef o = checked_malloc(sizeof(*o));
  *o = zero_object;
  o->diffuse_color = get_diffuse_color_from_spec(osr);
  o->specular_color = get_specular_color_from_spec(osr);
  o->ns = get_ns(osr);

  validate_object(o);
 
  return o;
}

static double* get_diffuse_color_from_spec(SpecRef osr) {
  double *dc = next_vector_field_value_with_name(osr, "diffuse_color");

  if(NULL == dc) {
    dc = next_vector_field_value_with_name(osr, "color");
    if(NULL == dc) {
      fprintf(stderr, "Error: Neither 'color' nor 'diffuse_color' was specified for object\n");
      exit(EXIT_FAILURE);
    }
  }

  return dc;
}

static double* get_specular_color_from_spec(SpecRef osr) {
  double *sc = next_vector_field_value_with_name(osr, "specular_color");
  if(NULL == sc) {
    #ifdef DEBUG_NOTICE
    fprintf(stderr, "NOTICE: No 'specular_color' was specified for object\n");
    fprintf(stderr, "NOTICE: Assigning default of (0, 0, 0)\n");
    #endif
    sc = checked_malloc(3 * sizeof(*sc));
    sc[0] = 0;
    sc[1] = 0;
    sc[2] = 0;
  }
  return sc;
}

static double get_ns(SpecRef osr) {
  double ns = next_scalar_field_value_with_name(osr, "ns");
  if(NO_SCALAR == ns) {
    #ifdef DEBUG_NOTICE
    fprintf(stderr, "NOTICE: No 'ns' was specified for object\n");
    fprintf(stderr, "NOTICE: Assigning default of 20\n");
    #endif
    return 20;
  }

  return ns;
}

static void validate_object(ObjectRef o) {
  if(NULL == o->specular_color) {
    fprintf(stderr, "Error: No specular color for object\n");
    exit(EXIT_FAILURE);
  }
  if(NULL == o->diffuse_color) {
    fprintf(stderr, "Error: No diffuse color for object\n");
    exit(EXIT_FAILURE);
  }
  if(NO_SCALAR == o->ns) {
    fprintf(stderr, "Error: No ns for object\n");
    exit(EXIT_FAILURE);
  }
}


static double plane_intersection(RayRef ray, ObjectRef p) {
  double p_norm_dot_r_dir = vec_dot(p->plane.normal, ray->dir);
  if(0 == p_norm_dot_r_dir) return MISS;
  double r_origin_sub_p_pos[3];
  vec_subtract(ray->origin, p->plane.position, r_origin_sub_p_pos);
  double p_norm_dot_r_origin_sub_p_pos = vec_dot(p->plane.normal, r_origin_sub_p_pos);
  double t_for_intersection = -1 * (p_norm_dot_r_origin_sub_p_pos / p_norm_dot_r_dir);
  return t_for_intersection > 0 ? t_for_intersection : MISS;
}


static double sphere_intersection(RayRef r, ObjectRef s) {
  // First, see if there is any intersection at all
  double s_center_sub_r_origin[3];
  vec_subtract(s->sphere.position, r->origin, s_center_sub_r_origin);
  double closest_t_to_s_center = vec_dot(r->dir, s_center_sub_r_origin);
  if(closest_t_to_s_center <= 0) return MISS;
  
  double closest_point_to_s_center_on_r[3];
  point_on_ray_at_t(r, closest_t_to_s_center, closest_point_to_s_center_on_r);
  double dist_closest_point_to_center = point_distance(closest_point_to_s_center_on_r, s->sphere.position);
  if(dist_closest_point_to_center > s->sphere.radius) return MISS;
  if(dist_closest_point_to_center == s->sphere.radius) return dist_closest_point_to_center;

  return closest_t_to_s_center - sqrt(pow(s->sphere.radius, 2) - pow(dist_closest_point_to_center, 2));
}


double quadric_intersection(RayRef ray, ObjectRef or) {
  double* q = or->quadric.parts;
  double* o = ray->origin;
  double* d = ray->dir;
  double Aq = q[A] * pow(d[X], 2) +
    q[B] * pow(d[Y], 2) +
    q[C] * pow(d[Z], 2) +
    q[D] * d[X] * d[Y] +
    q[E] * d[X] * d[Z] +
    q[F] * d[Y] * d[Z];
  double Bq = 2 * q[A] * o[X] * d[X] +
    2 * q[B] * o[Y] * d[Y] +
    2 * q[C] * o[Z] * d[Z] +
    q[D] * (o[X] * d[Y] + o[Y] * d[X]) +
    q[E] * (o[X] * d[Z] + o[Z] * d[X]) +
    q[F] * (o[Y] * d[Z] + o[Y] * d[Z]) +
    q[G] * d[X] +
    q[H] * d[Y] +
    q[I] * d[Z];
  double Cq = q[A] * pow(o[X], 2) +
    q[B] * pow(o[Y], 2) +
    q[C] * pow(o[Z], 2) +
    q[D] * o[X] * o[Y] +
    q[E] * o[X] * o[Z] +
    q[F] * o[Y] * o[Z] +
    q[G] * o[X] +
    q[H] * o[Y] +
    q[I] * o[Z] +
    q[J];

  if(0 == Aq) {
    if(Bq == 0.0) { return MISS; }
    return -Cq / Bq;
  }

  double discriminant = pow(Bq, 2) - 4 * Aq * Cq;
  if(discriminant < 0.0) {
    return MISS;
  }

  double sqrt_discriminant = sqrt(discriminant);
  double t0 = (-Bq - sqrt_discriminant) / (2 * Aq);
  if(t0 > 0.0) { return t0; }
  double t1 = (-Bq + sqrt_discriminant) / (2 * Aq);
  if(t1 > 0.0) { return t1; }

  return MISS;
}


static void get_plane_surface_normal(ObjectRef p, double *out) {
  vec_copy(p->plane.normal, out);
}


static void get_sphere_surface_normal(ObjectRef s, double *point, double *out) {
  double intermediate[] = {0.0, 0.0, 0.0};
  vec_subtract(point, s->sphere.position, intermediate);
  vec_normalize(intermediate, out);
}


static void get_quadric_surface_normal(ObjectRef quadric, double *point, double *out) {
  double *q = quadric->quadric.parts;
  out[X] = (2 * q[A] * point[X]) + (q[D] * point[Y]) + (q[E] * point[Z]) + q[G];
  out[Y] = (2 * q[B] * point[Y]) + (q[D] * point[X]) + (q[F] * point[Z]) + q[H];
  out[Z] = (2 * q[C] * point[Z]) + (q[E] * point[X]) + (q[F] * point[Y]) + q[I];
  vec_normalize(out, out);
}
