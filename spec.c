#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "spec.h"
#include "util.h"


enum FieldKind { NoFieldKind, TypeDecl, Vector, Scalar };

struct SpecField {
  enum FieldKind kind;
  char* name;
  union {
    char* type_str;
    double* vector_value;
    double scalar_value;
  };
  SpecFieldRef next;
};

typedef struct SpecField SpecField;

struct Spec {
  SpecFieldRef first_field;
  SpecRef next;
};

typedef struct Spec Spec;
typedef struct Spec SceneNode;

//////////////////// Forward Declarations ////////////////////
// static void print_spec(SpecRef);
// static void print_spec_field(SpecFieldRef);
static bool spec_declares_kind(SpecRef, char*); 
//////////////////////////////////////////////////////////////


//////////////////// Field Spec Functions ////////////////////
SpecFieldRef new_spec_field() {
  SpecField zero_spec_field = {0};
  SpecFieldRef ret = checked_malloc(sizeof(*ret));
  *ret = zero_spec_field;
  return ret;
}


void spec_field_set_name(SpecFieldRef fsr, char* name) {
  fsr->name = name;
}


void spec_field_solidify_to_type_decl(SpecFieldRef fsr, char* type_str) {
  fsr->kind = TypeDecl;
  fsr->type_str = type_str;
}


void spec_field_solidify_to_scalar(SpecFieldRef fsr, double scalar) {
  fsr->kind = Scalar;
  fsr->scalar_value = scalar;
}


void spec_field_solidify_to_vector(SpecFieldRef fsr, double* vector) {
  fsr->kind = Vector;
  fsr->vector_value = vector;
}


void print_spec_field(SpecFieldRef fsr) {
  if(NULL == fsr) {
    printf("\t\t\"NULL\" : NULL\n");
    return;
  }
#ifdef DEBUG
  printf("DEBUG: Entering print_spec_field\n");
#endif
  printf("\t\t\"%s\" : ", fsr->name);
  switch(fsr->kind) {
  case TypeDecl:
    printf("\"%s\"", fsr->type_str);
    break;
  case Vector:
    printf("[%f, %f, %f]", fsr->vector_value[0], fsr->vector_value[1], fsr->vector_value[2]);
    break;
  case Scalar:
    printf("%f", fsr->scalar_value);
    break;
  case NoFieldKind:
    fprintf(stderr, "Error: Trying to print unsolidified field spec.\n");
    exit(EXIT_FAILURE);
    break;
  default:
    fprintf(stderr, "Error: Something has gone horribly wrong.\n");
    fprintf(stderr, "Error: When printing a field spec the default kind case was reached.\n");
    exit(EXIT_FAILURE);
    break;
  }
  printf(",\n");
}


void destroy_spec_field(SpecFieldRef f) {
  free(f->name);
  switch(f->kind) {
  case TypeDecl:
    free(f->type_str);
    break;
  case Vector:
    free(f->vector_value);
    break;
  case Scalar:
  case NoFieldKind:
    break;
  }
  free(f);
}
//////////////////////////////////////////////////////////////


//////////////////// Object Spec Fucntions ////////////////////
SpecRef new_spec() {
  Spec zero_spec = {0};
  SpecRef ret = checked_malloc(sizeof(*ret));
  *ret = zero_spec;
  return ret;
}


void add_spec_field_to_spec(SpecFieldRef f, SpecRef o) {
  if(NULL == o) {
    fprintf(stderr, "Error: Null ObjectSpecRef passed to add_spec_field_to_spec\n");
    exit(EXIT_FAILURE);
  }

  if(NULL == f) {
    fprintf(stderr, "Error: Null SpecFieldRef passed to add_spec_field_to_spec\n");
    exit(EXIT_FAILURE);
  }

  SpecFieldRef current = o->first_field;
  if(NULL == current) {
    if(0 == strcmp(f->name, "type") && TypeDecl == f->kind) {
      o->first_field = f;
      return;
    } else {
      fprintf(stderr, "f->name is %s\n", f->name);
      print_spec_field(f);
      report_error_and_exit("The first SpecField added to an ObjectSpec must be named 'type'.");
    }
  }
  
  while(NULL != current->next) {
    current = current->next;
  }
  current->next = f;
}


void print_spec(SpecRef o) {
#ifdef DEBUG
  printf("DEBUG: Entering print_spec\n");
#endif

  if(NULL == o) { report_error_and_exit("Null SpecRef passed to print_spec"); }

  printf("\t{\n");
  SpecFieldRef current = o->first_field;
  while(NULL != current) {
    print_spec_field(current);
    current = current->next;
  }
  printf("\t},\n");
}

static bool spec_declares_kind(SpecRef osr, char* kind) {
  if(NULL == osr) {
    report_error_and_exit("Null SpecRef passed to spec_declares_kind");
  }

  if(NULL != osr->first_field) {
    // The first SpecField in an Spec should always declare the type.
    // This is ensured by add_spec_field_to_spec().
    assert(0 == strcmp(osr->first_field->name, "type")); 
    return 0 == strcmp(osr->first_field->type_str, kind);
  }

  return false;
}


double* next_vector_field_value_with_name(SpecRef osr, char* name) {
  if(NULL == osr) {
    report_error_and_exit("Null SpecRef passed to next_vector_field_value_with_name");
  }

  double* ret = checked_malloc(3 * sizeof(*ret));
  SpecFieldRef current = osr->first_field;

  if(NULL != current && Vector == current->kind && 0 == strcmp(current->name, name)) {
    ret[0] = current->vector_value[0];
    ret[1] = current->vector_value[1];
    ret[2] = current->vector_value[2];
    osr->first_field = current->next;
    destroy_spec_field(current);
    return ret;
  } 

  SpecFieldRef prev = current;
  current = current->next;
  while(NULL != current) {
    if(Vector == current->kind && strcmp(current->name, name) == 0) {
      ret[0] = current->vector_value[0];
      ret[1] = current->vector_value[1];
      ret[2] = current->vector_value[2];
      prev->next = current->next;
      destroy_spec_field(current);
      return ret;
    }
    prev = current;
    current = current->next;
  }
  
  return NULL;
}


double next_scalar_field_value_with_name(SpecRef osr, char* name) {
  double ret;
  SpecFieldRef current = osr->first_field;
  if(NULL != current && Scalar == current->kind && 0 == strcmp(current->name, name)) {
    ret = current->scalar_value;
    osr->first_field = current->next;
    destroy_spec_field(current);
    return ret;
  } 

  SpecFieldRef prev = current;
  current = current->next;
  while(NULL != current) {
    if(0 == strcmp(current->name, name) && Scalar == current->kind) {
      ret = current->scalar_value;
      prev->next = current->next;
      destroy_spec_field(current);
      return ret;
    }
    prev = current;
    current = current->next;
  }

  return NO_SCALAR;
}


void destroy_spec(SpecRef osr) {
  SpecFieldRef temp = osr->first_field;
  SpecFieldRef next = NULL;
  while(NULL != temp) {
    next = temp->next;
    destroy_spec_field(temp);
    temp = next;
  }
  free(osr);
}
//////////////////////////////////////////////////////////////


//////////////////// Scene Functions ////////////////////
Scene new_scene() {
  SpecRef *ret = checked_malloc(sizeof(*ret));
  return (Scene) ret;
}


void add_spec_to_scene(Scene scene, SpecRef osr) {
  if(NULL == osr) { report_error_and_exit("Null SpecRef passed to add_spec_to_scene"); }

  if(NULL == *scene) {
    *scene = osr;
    return;
  }

  SpecRef scene_osr = *scene;
  while(NULL != scene_osr->next) {
    scene_osr = scene_osr->next;
  }
  scene_osr->next = osr;
}


SpecRef next_spec_declaring_kind(Scene scene, char *kind) {
  SpecRef current = *scene;
  if(NULL == current) {
    return NULL;
  }

  if(NULL != current && spec_declares_kind(current, kind)) {
    *scene = current->next;
    current->next = NULL;
    return current;
  }

  SpecRef prev = current;
  current = current->next;
  while(NULL != current) {
    if(spec_declares_kind(current, kind)) {
      prev->next = current->next;
      current->next = NULL;
      return current;
    } else {
      prev = current;
      current = current->next;
    }
  }

  return NULL;
}


void print_scene(Scene scene) {
  SpecRef current = *scene;
  printf("[\n");
  while(NULL != current) {
    print_spec(current);
    current = current->next;
  }
  printf("]\n");
}


void destroy_scene(Scene scene) {
  SpecRef temp = *scene;
  SpecRef next = temp;
  while(NULL != temp) {
    next = temp->next;
    destroy_spec(temp);
    temp = next;
  }
  free(scene);
}
//////////////////////////////////////////////////////////////
