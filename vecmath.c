#include <stdlib.h>
#include "vecmath.h"
#include "util.h"

void vec_copy(Vec from, Vec into) {
  into[X] = from[X];
  into[Y] = from[Y];
  into[Z] = from[Z];
}


void vec_add(Vec a, Vec b, Vec out) {
  out[X] = a[X] + b[X];
  out[Y] = a[Y] + b[Y];
  out[Z] = a[Z] + b[Z];
}


// TODO: Rename to vec_sub
void vec_subtract(Vec a, Vec b, Vec out) {
  out[X] = a[X] - b[X];
  out[Y] = a[Y] - b[Y];
  out[Z] = a[Z] - b[Z];
}


void vec_mult(Vec a, Vec b, Vec out) {
  out[X] = a[X] * b[X];
  out[Y] = a[Y] * b[Y];
  out[Z] = a[Z] * b[Z];
}


void vec_scale(Vec a, double s, Vec out) {
  out[X] = s * a[X];
  out[Y] = s * a[Y];
  out[Z] = s * a[Z];
}


double vec_magnitude(Vec a) {
  return sqrt(pow(a[X], 2) + pow(a[Y], 2) + pow(a[Z], 2));
}


double vec_dot(Vec a, Vec b) {
  return (a[X]*b[X]) + (a[Y]*b[Y]) + (a[Z]*b[Z]);
}


// a x b =
// a0b0(i x i) + a0b1(i x j) + a0b2(i x k) +
// a1b0(j x i) + a1b1(j x j) + a1b2(j x k) +
// a2b0(k x i) + a2b1(k x j) + a2b2(k x k)
//
//           0 +    a0b1k    +    a0b2-j   +
//      a1b0-k +      0      +    a1b2i    +
//       a2b0j +    a2b1-i   +      0      +
// a1b2i - a2b1i + a2b0j - a0b2j + a0b1k - a1b0k
void vec_cross(Vec a, Vec b, Vec out) {
  Vec intermediate = {0.0};
  intermediate[X] = a[Y]*b[Z] - a[Z]*b[Y];
  intermediate[Y] = a[Z]*b[X] - a[X]*b[Z];
  intermediate[Z] = a[X]*b[Y] - a[Y]*b[X];
  vec_copy(intermediate, out);
}


void vec_reflect(Vec a, Vec b, Vec out) {
  double scaled_b[3];
  vec_scale(b, (2 * vec_dot(b, a)), scaled_b);
  vec_subtract(a, scaled_b, out);
}


double interior_angle_of_normals(Vec a, Vec b) {
  return acos(vec_dot(a, b));
}


void point_add(Point a, Point b, Point out) {
  vec_add(a, b, out);
}


void point_subtract(Point a, Point b, Point out) {
  vec_subtract(a, b, out);
}


double point_distance(Point a, Point b) {
  return sqrt(pow(a[X] - b[X], 2) + pow(a[Y] - b[Y], 2) + pow(a[Z] - b[Z], 2));
}


RayRef new_ray() {
  Ray zero_ray = {{0.0}, {0.0}};
  RayRef r = checked_malloc(sizeof(*r));
  *r = zero_ray;
  return r;
}


void destroy_ray(RayRef r) {
  free(r);
}


void vec_normalize(Vec in, Vec out) {
  vec_scale(in, 1.0 / vec_magnitude(in), out);
}


void point_on_ray_at_t(RayRef r, double t, Point out) {
  Vec r_dir_scaled_by_t = {0};
  vec_scale(r->dir, t, r_dir_scaled_by_t);
  vec_add(r->origin, r_dir_scaled_by_t, out);
}

void get_inter_point_normal_vector(double *from, double *to, double *out) {
  double intermediate[] = {to[X] - from[X], to[Y] - from[Y], to[Z] - from[Z]};
  vec_normalize(intermediate, out);
}

void get_next_point_along_ray(RayRef ray, double *out) {
  point_on_ray_at_t(ray, 0.0000000001, out);
}

void scooch_ray_origin(RayRef ray) {
  double new_origin[3] = {0};
  get_next_point_along_ray(ray, new_origin);
  vec_copy(new_origin, ray->origin);
}
