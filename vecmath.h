#ifndef VECMATH_HEADER
#define VECMATH_HEADER 1

#include <math.h>

#define X 0
#define Y 1
#define Z 2

typedef double Vec[3];
typedef double Point[3];

struct Ray {
  Point origin;
  Vec dir;
};

typedef struct Ray Ray;
typedef struct Ray * RayRef;


RayRef new_ray(void);
void destroy_ray(RayRef);
void vec_normalize(Vec, Vec);
void point_on_ray_at_t(RayRef, double, Point);
void vec_copy(Vec, Vec);
void vec_add(Vec, Vec, Vec);
void vec_subtract(Vec, Vec, Vec);
void vec_mult(Vec, Vec, Vec);
void vec_scale(Vec, double, Vec);
double vec_magnitude(Vec);
double vec_dot(Vec, Vec);
void vec_cross(Vec, Vec, Vec);
void vec_reflect(Vec, Vec, Vec);
void point_add(Point, Point, Point);
void point_subtract(Point, Point, Point);
double point_distance(Point, Point);
void get_inter_point_normal_vector(double *, double *, double *);
void get_next_point_along_ray(RayRef, double *);
void scooch_ray_origin(RayRef);

#endif
