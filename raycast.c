#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "raycast.h"
#include "camera.h"
#include "object.h"
#include "light.h"
#include "pixelbuf.h"
#include "vecmath.h"

#define RECURSIVE_DEPTH 7

static PixelBufRef s_raycast(void);
static ObjectRef shoot(RayRef, double*);
static void shade(double*, ObjectRef, double*, int, double*);
static void get_lightward_ray(double*, LightRef, RayRef);
static bool ray_intersects_objects(RayRef, double);
static void get_cameraward_normal(double*, double*);
static void get_reflective_contrib(double*, ObjectRef, double*, double*, int, double*);
static void get_refractive_contrib(double*, ObjectRef, double*, double*, int, double*);
static void get_refractive_ray(RayRef, double*, double*, double*, ObjectRef);
static CameraRef camera;
static ObjectRef *objects;
static LightRef *lights;
static int width;
static int height;
static double bg_color[3] = {0.5, 0.5, 0.5};

PixelBufRef raycast(CameraRef c, ObjectRef *os, LightRef *ls, int w, int h) {
  camera = c;
  objects = os;
  lights = ls;
  width = w;
  height = h;
  return s_raycast();
}

static PixelBufRef s_raycast() {
  PixelBufRef pb = new_pixel_buf(width, height);
  double c_width = get_camera_width(camera);
  double c_height = get_camera_height(camera);
  double pix_width = c_width / (double) width;
  double pix_height = c_height / (double) height;
  Point c_pos = {0.0};
  get_camera_position(camera, c_pos);
  Vec vpc = {0.0};
  get_viewplane_center(camera, vpc);
  Vec vpx_u = {0.0};
  Vec vpy_u = {0.0};
  Vec vpz_u = {0.0};
  get_viewplane_unit_vectors(camera, vpx_u, vpy_u, vpz_u);
  Vec vp_x_to_pixel = {0.0};
  Vec vp_y_to_pixel = {0.0};
  Point vp_xy_to_pixel = {0.0};
  Vec camera_to_pixel_center = {0.0};
  RayRef r = new_ray();
  vec_copy(c_pos, r->origin);
  Point intersection_point = {0.0};
  for(int row = 0; row < height; row++) {
    double row_scale = (-c_height / 2.0) + (pix_height * (row + 0.5));
    vec_scale(vpy_u, row_scale, vp_y_to_pixel);

    for(int col = 0; col < width; col++) {
      double col_scale = (-c_width / 2.0) + (pix_width * (col + 0.5));
      vec_scale(vpx_u, col_scale, vp_x_to_pixel);

      Vec intermediate = {0.0};
      vec_add(vp_x_to_pixel, vp_y_to_pixel, intermediate);
      vec_add(vpc, intermediate, vp_xy_to_pixel);
      vec_subtract(vp_xy_to_pixel, c_pos, camera_to_pixel_center);
      vec_normalize(camera_to_pixel_center, r->dir);
      ObjectRef intersected_obj = shoot(r, intersection_point);
      if(NULL != intersected_obj) {
	double view_n[3] = {0.0};
	get_cameraward_normal(intersection_point, view_n);
	vec_scale(view_n, -1.0, view_n);
	double color_at_point[3] = {0.0};
	shade(intersection_point, intersected_obj, view_n, RECURSIVE_DEPTH, color_at_point);
	color_pixel(pb, color_at_point, row, col);
      } else {
	color_pixel(pb, bg_color, row, col);
      }
    }
  }
  return pb;
}


static ObjectRef shoot(RayRef r, Point intersection) {
  ObjectRef best_t_obj = NULL;
  double best_t = INFINITY; 
  double current_t = INFINITY;
  for(int obj_offset = 0; NULL != objects[obj_offset] ;obj_offset++) {
    current_t = has_intersection(r, objects[obj_offset]);
    if(current_t < best_t) {
      best_t = current_t;
      best_t_obj = objects[obj_offset];
    }
  }
  point_on_ray_at_t(r, best_t, intersection);
  return best_t_obj;
}


 static void shade(double *intersect, ObjectRef intersected_obj, double *view_n, int r_level,
		   double *color_out) {
  double total_diffuse[3] = {0.0};
  double total_specular[3] = {0.0};
  double surface_n[3] = {0.0};
  get_surface_normal(intersected_obj, intersect, surface_n);

  for(LightRef *lights_iter = lights; NULL != *lights_iter; lights_iter++) {
    LightRef light = *lights_iter;
    Ray lightward_r = {{0.0}, {0.0}};
    get_lightward_ray(intersect, light, &lightward_r);
    double intersectward_n[3] = {0.0};
    vec_scale(lightward_r.dir, -1.0, intersectward_n);

    double dist_to_light = point_distance(intersect, light->position);
    if(ray_intersects_objects(&lightward_r, dist_to_light) || !light_is_contributing(light, intersectward_n)) {
      continue;
    }

    if(vec_dot(surface_n, intersectward_n) > 0) {
      vec_scale(surface_n, -1, surface_n);
    }

    double diffuse_contrib[3] = {0.0};
    get_diffuse_contrib(light, intersectward_n, surface_n, diffuse_contrib);

    double inv_view_n[3] = {0.0};
    vec_scale(view_n, -1.0, inv_view_n);
    double specular_contrib[3] = {0.0};
    get_specular_contrib(light, intersectward_n, surface_n, inv_view_n, intersected_obj->ns, specular_contrib);

    attenuate_radially(light, dist_to_light, diffuse_contrib, specular_contrib);

    vec_mult(intersected_obj->diffuse_color, diffuse_contrib, diffuse_contrib);
    vec_mult(intersected_obj->specular_color, specular_contrib, specular_contrib);

    vec_add(diffuse_contrib, total_diffuse, total_diffuse);
    vec_add(specular_contrib, total_specular, total_specular);
  }

  vec_add(total_diffuse, total_specular, color_out);
  vec_scale(color_out, (1.0 - (intersected_obj->reflectivity + intersected_obj->refractivity)), color_out);

  if(r_level <= 0)
    return;
  double reflective_contrib[3] = {0};
  get_reflective_contrib(intersect, intersected_obj, view_n, surface_n, r_level, reflective_contrib);
  double refractive_contrib[3] = {0};
  get_refractive_contrib(intersect, intersected_obj, view_n, surface_n, r_level, reflective_contrib);

  vec_add(reflective_contrib, color_out, color_out);
  vec_add(refractive_contrib, color_out, color_out);
 }


static void get_reflective_contrib(double *intersect, ObjectRef intersected_obj, double *view_n,
				   double *surface_n, int r_level, double *reflective_contrib) {
  Ray refl_ray = {{intersect[X], intersect[Y], intersect[Z]}, {0.0}};
  vec_reflect(view_n, surface_n, refl_ray.dir);
  scooch_ray_origin(&refl_ray);

  double refl_intersect[3] = {0.0};
  ObjectRef refl_obj = shoot(&refl_ray, refl_intersect);
  if(NULL == refl_obj) {
    reflective_contrib[X] = 0.0;
    reflective_contrib[Y] = 0.0;
    reflective_contrib[Z] = 0.0;
    return;
  }

  shade(refl_intersect, refl_obj, refl_ray.dir, r_level - 1, reflective_contrib);
  vec_scale(reflective_contrib, intersected_obj->reflectivity, reflective_contrib);
}


static void get_refractive_contrib(double *intersect, ObjectRef intersected_obj, double *surface_n,
				   double *view_n, int r_level, double *refractive_contrib) {
  Ray refr_ray = {{0.0}, {0.0}};
  get_refractive_ray(&refr_ray, view_n, surface_n, intersect, intersected_obj);
  
  double refr_intersect[3] = {0.0};
  ObjectRef refr_obj = shoot(&refr_ray, refr_intersect);
  if(NULL == refr_obj) {
    refractive_contrib[X] = 0.0;
    refractive_contrib[Y] = 0.0;
    refractive_contrib[Z] = 0.0;
    return;
  }

  shade(refr_intersect, refr_obj, refr_ray.dir, r_level - 1, refractive_contrib);
  vec_scale(refractive_contrib, intersected_obj->refractivity, refractive_contrib);
}


static void get_refractive_ray(RayRef refr_ray, double *view_n, double *surface_n, double *intersect,
			       ObjectRef obj) {
  Ray internal_ray = {{intersect[X], intersect[Y], intersect[Z]}, {0.0}};
  double a[3] = {0.0};
  vec_cross(surface_n, view_n, a);
  vec_normalize(a, a);
  double b[3] = {0.0};
  vec_cross(a, surface_n, b);
  double sin_phi = (1.0/obj->ior) * vec_dot(view_n, b);
  double cos_phi = sqrt(1 - pow(sin_phi, 2));
  double scaled_view_n[3] = {0.0};
  vec_scale(view_n, -cos_phi, scaled_view_n);
  double scaled_b_n[3] = {0.0};
  vec_scale(b, sin_phi, scaled_b_n);
  vec_add(scaled_view_n, scaled_b_n, internal_ray.dir);
  scooch_ray_origin(&internal_ray);

  double internal_intersect[3] = {0.0};
  ObjectRef surrounding_obj = shoot(&internal_ray, internal_intersect);
  if(obj != surrounding_obj || NULL == surrounding_obj) {
    vec_copy(internal_ray.origin, refr_ray->origin);
    vec_copy(internal_ray.dir, refr_ray->dir);
    return;
  } else {
    vec_copy(internal_intersect, refr_ray->origin);
    vec_copy(view_n, refr_ray->dir);
    double out_surface_n[3] = {0.0};
    get_surface_normal(obj, internal_intersect, out_surface_n);
    if(-vec_dot(view_n, surface_n) != vec_dot(refr_ray->dir, out_surface_n)) {
      printf("ERROR: ASSUMPTION VIOLATED!!!!\n");
    }
    return;
  }
  printf("PANIC: Sump'n real bad hap'nin!\n");
  exit(EXIT_FAILURE);
}


static void get_lightward_ray(double *point, LightRef light, RayRef out) {
  out->origin[X] = point[X];
  out->origin[Y] = point[Y];
  out->origin[Z] = point[Z];
  
  get_inter_point_normal_vector(point, light->position, out->dir);

  Point scooched_point = {0.0};
  get_next_point_along_ray(out, scooched_point);
  out->origin[X] = scooched_point[X];
  out->origin[Y] = scooched_point[Y];
  out->origin[Z] = scooched_point[Z];
}


static bool ray_intersects_objects(RayRef lightward_r, double distance_to_light) {
  Point point_intersected = {0.0};
  ObjectRef object_intersected = shoot(lightward_r, point_intersected);
  if(NULL == object_intersected) {
    return false;
  } else {
    double distance_to_intersection = point_distance(lightward_r->origin, point_intersected);
    return distance_to_light >= distance_to_intersection;
  }
}


static void get_cameraward_normal(double *from_point, double *out) {
  vec_subtract(camera->position, from_point, out);
  vec_normalize(out, out);
}
