#ifndef SCENE_SPEC_HEADER
#define SCENE_SPEC_HEADER 1

#include <math.h>
#define MAX_SPEC_STR_LEN 16
#define NO_SCALAR -INFINITY

//////////////////// Structs and Typedefs ////////////////////
typedef struct SpecField* SpecFieldRef;
typedef struct Spec* SpecRef;
typedef SpecRef* Scene;
//////////////////////////////////////////////////////////////


SpecFieldRef new_spec_field(void);
void spec_field_set_name(SpecFieldRef, char*);
void spec_field_solidify_to_type_decl(SpecFieldRef, char*);
void spec_field_solidify_to_scalar(SpecFieldRef, double);
void spec_field_solidify_to_vector(SpecFieldRef, double*);
void print_spec_field(SpecFieldRef);
void destroy_spec_field(SpecFieldRef);

SpecRef new_spec(void);
void add_spec_field_to_spec(SpecFieldRef, SpecRef);
double* next_vector_field_value_with_name(SpecRef, char*);
double next_scalar_field_value_with_name(SpecRef, char*);
void print_spec(SpecRef);
void destroy_spec(SpecRef);


Scene new_scene(void);
void add_spec_to_scene(Scene, SpecRef);
SpecRef next_spec_declaring_kind(Scene, char*);
void print_scene(Scene);
void destroy_scene(Scene);
SpecRef next_spec_declaring_kind(Scene, char*);
#endif
