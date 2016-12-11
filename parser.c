#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "spec.h"
#include "parser.h"
#include "util.h"


static FILE* open_scene_file(const char*);
static void ensure_non_empty_object_list(FILE*);
static Scene parse_scene(FILE*);
static void close_scene_file(FILE*);
static SpecRef next_spec(FILE*);
static SpecFieldRef next_spec_field(FILE*); 
static void next_key(FILE*, SpecFieldRef);
static void next_value(FILE*, SpecFieldRef);
static char* next_string(FILE*);
static void error_on_excessive_string_length(int);
static void error_on_invalid_char(char);
static double* next_vector(FILE*);
static double next_double(FILE*);
static void skip_ws(FILE*); 
static void consume_next_c_on_match_or_err(FILE*, char, const char*);
static void consume_next_c_on_match(FILE*, char);
static bool next_c_matches(FILE*, char);
static char next_c(FILE*);
static void report_error(const char*);

static int line_num = 0;

#define DEBUG 1

/* Parse scene from file */
Scene parse_scene_from_file(char* filename) {
  DEBUG_LOG("Entering parse_scene_from_file");
  FILE* scene_file = open_scene_file(filename);
  ensure_non_empty_object_list(scene_file);
  Scene scene = parse_scene(scene_file);
  close_scene_file(scene_file);
  return scene;
}


/* Wraps fopen to provide error checking */
static FILE* open_scene_file(const char* filename) {
  DEBUG_LOG("Entering open_scene_file");
  FILE* scene_file = fopen(filename, "r");

  if (scene_file == NULL) {
    fprintf(stderr, "Error: Could not open file \"%s\"\n", filename);
    exit(EXIT_FAILURE);
  }

  return scene_file;
}


static void ensure_non_empty_object_list(FILE* scene_file) {
  DEBUG_LOG("Entering ensure_non_empty_object_list");
  skip_ws(scene_file);
  consume_next_c_on_match_or_err(scene_file, '[', "Expected opening of object list (missing '[')");
  skip_ws(scene_file);
  if(next_c_matches(scene_file, ']')) report_error("This scene file is empty; expected object definitions");
}


/* Parses ObjSpecs from the scene file until a ']' is encountered, i.e., the close of the object
 * list. As ObjSpecs are parsed, they are wrapped in a SceneNode and those nodes are linked
 * sequentially. Returns a pointer to the head of the SceneNode list. */
static Scene parse_scene(FILE* scene_file) {
  DEBUG_LOG("Entering parse_scene");
  Scene scene = new_scene();
  SpecRef current_spec = next_spec(scene_file);
  if(NULL == current_spec) {
    report_error("Expected an object definition");
    exit(EXIT_FAILURE);
  }
  add_spec_to_scene(scene, current_spec);

  // As long as another ObjSpec can be parsed from the file, keep doing so.
  while(NULL != (current_spec = next_spec(scene_file))) {
    add_spec_to_scene(scene, current_spec);
  }
  skip_ws(scene_file);
  consume_next_c_on_match_or_err(scene_file, ']', "Expected end of object list (missing ']')");

  return scene;
}


/* Simply wraps fclose to make sure that nothing goes horribly wrong. */
static void close_scene_file(FILE* scene_file) {
  DEBUG_LOG("Entering close_scene_file");
  if(EOF == fclose(scene_file)) {
    fprintf(stderr, "Error: Something went horribly wrong.\n");
    fprintf(stderr, "Error: The scene file could not be closed.\n");
  }
}


/* Parses the next object definition in the scene file. Works by parsing FieldSpecs from the scene
 * file until a '}' is encountered, i.e., the closing of the object definition. Returns NULL when
 * the next non-whitespace char is nota '{', i.e., the opening of a new object definition. */
static SpecRef next_spec(FILE* scene_file) {
  DEBUG_LOG("Entering next_spec");
  skip_ws(scene_file);

  
  if(!next_c_matches(scene_file, '{')) {
    return NULL;
  } else {
    next_c(scene_file);
  }
  
  SpecRef out_spec = new_spec();
  while(!next_c_matches(scene_file, '}')) {
    add_spec_field_to_spec(next_spec_field(scene_file), out_spec);
  }
  next_c(scene_file); // Consume the '}'

  skip_ws(scene_file);
  consume_next_c_on_match(scene_file, ',');


  return out_spec;
}

/* Parses the next FieldSpec in the scene file. This either returns a FieldSpec or encounters a
 * failure such that exit() is called. Works by parsing the next string (field name), a colon
 * (separator), and the next string (for a "type" field), vector (for "position", "normal", etc.
 * fields), or a double (for "radius", "height", quadric component, etc. fields). */
static SpecFieldRef next_spec_field(FILE* scene_file) {
  DEBUG_LOG("Entering next_spec_field");

  SpecFieldRef f_spec = new_spec_field();
  next_key(scene_file, f_spec);
  skip_ws(scene_file);
  consume_next_c_on_match_or_err(scene_file, ':', "Expecting key-value pair (missing ':')");
  next_value(scene_file, f_spec);
  skip_ws(scene_file);
  consume_next_c_on_match(scene_file, ',');

  return f_spec;
}


static void next_key(FILE* scene_file, SpecFieldRef out_spec) {
  DEBUG_LOG("Entering next_key");
  skip_ws(scene_file);
  spec_field_set_name(out_spec, next_string(scene_file));
}


/* See the comment for next_spec_field. */
static void next_value(FILE* scene_file, SpecFieldRef out_spec) {
  DEBUG_LOG("Entering next_value");
  skip_ws(scene_file);
  if(next_c_matches(scene_file, '"')) {
    spec_field_solidify_to_type_decl(out_spec, next_string(scene_file));
  } else if(next_c_matches(scene_file, '[')) {
    spec_field_solidify_to_vector(out_spec, next_vector(scene_file));
  } else {
    spec_field_solidify_to_scalar(out_spec, next_double(scene_file));
  }
}


/* Gets the next string from the file handle. Emits an error and exits if no string is found. */
static char* next_string(FILE* scene_file) {
  DEBUG_LOG("Entering next_string");

  //handle string opening
  skip_ws(scene_file);
  consume_next_c_on_match_or_err(scene_file, '"', "Expected opening of string (missing \")");

  char buffer[MAX_SPEC_STR_LEN + 1];
  int i = 0;
  for(char c = next_c(scene_file); c != '"'; c = next_c(scene_file)) {
    error_on_excessive_string_length(i);
    error_on_invalid_char(c);
    buffer[i] = c;
    i++;
  } // Note that the closing '"' is discarded when the loop ends
  
  buffer[i] = '\0';
  return strdup(buffer);
}

static void error_on_excessive_string_length(int string_length) {
    if (string_length >= MAX_SPEC_STR_LEN - 1) {
      fprintf(stderr, "Error: Strings longer than %d characters are not supported.\n", MAX_SPEC_STR_LEN);
      report_error("Parsing ended unexpectedly");
      exit(EXIT_FAILURE);      
    }
}


static void error_on_invalid_char(char c) {
    if (c == '\\') {
      report_error("Strings with escape codes are not supported");
      exit(EXIT_FAILURE);      
    }
    if (c < 32 || c > 126) {
      report_error("Strings may contain only alpha-numeric characters");
      exit(EXIT_FAILURE);
    }
}


/* Gets the next vector from the file handle. Emits an error and exits if no vector is found. */
static double* next_vector(FILE* scene_file) {
  DEBUG_LOG("Entering next_vector");
  skip_ws(scene_file);
  consume_next_c_on_match_or_err(scene_file, '[', "Expected the opening of a vector (missing '[')");

  double* v = checked_malloc(3 * sizeof(double));
  for(int i = 0; i < 3; i++) {
    skip_ws(scene_file);
    v[i] = next_double(scene_file);
    skip_ws(scene_file);
    if(i != 2) {
      consume_next_c_on_match_or_err(scene_file, ',', "Expected the continuation of a vector (missing ',')");
    }
  }
  consume_next_c_on_match_or_err(scene_file, ']', "Expected the closing of a vector (missing ']')");
  return v;
}


/* Gets the next double from the file handle. Emits an error and exits if no double is found. */
static double next_double(FILE* scene_file) {
  DEBUG_LOG("Entering next_double");
  skip_ws(scene_file);
  double value;
  if(EOF == fscanf(scene_file, "%lf", &value)) {
    report_error("Parsing ended unexpectedly when a double was expected");
    exit(EXIT_FAILURE);
  }
  return value;
}


/* Skips the next run of white space in the file. Line number maintenance is deferred to next_c(). */
static void skip_ws(FILE* scene_file) {
  DEBUG_LOG("Entering skip_ws");
  char c = next_c(scene_file);
  while(isspace(c)) {
    c = next_c(scene_file);
  }
  ungetc(c, scene_file);
  DEBUG_LOG("Leaving skip_ws");
}


static void consume_next_c_on_match(FILE *scene_file, char c) {
  if(next_c_matches(scene_file, c)) next_c(scene_file);
}


static void consume_next_c_on_match_or_err(FILE* scene_file, char c, const char* err) {
  DEBUG_LOG("Entering consume_next_c_on_match_or_err");
  if(c == next_c(scene_file)) return;
  else {
    report_error(err);
    exit(EXIT_FAILURE);
  }
}


/* Returns true if the next char in the file is 'c', and false otherwise. Does not advance the
 * file's read index. */
static bool next_c_matches(FILE* scene_file, char c) {
  DEBUG_LOG("Entering next_c_matches");
  char d = next_c(scene_file);
  ungetc(d, scene_file);
  if(c == d) return true;
  else return false;
}


/* Wraps fgetc() providing error checking and line number maintenance. */
static char next_c(FILE* scene_file) {
  DEBUG_LOG("Entering next_c");
  int c = fgetc(scene_file);
  if (c == '\n') {
    line_num += 1;
  }
  if (c == EOF) {
    report_error("Unexpected end of file");
    exit(EXIT_FAILURE);
  }
  return c;
}


/* Reports errors in a standard format with the line number. */
static void report_error(const char* error_msg) {
  fprintf(stderr, "Error: %s on line %d\n", error_msg, line_num);
}
