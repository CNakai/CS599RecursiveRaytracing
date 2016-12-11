#include <stdio.h>
#include <stdlib.h>
#include "vecmath.h"

void test_in_place_add() {
  double vec1[3] = {1, 2, 3};
  double vec2[3] = {4, 5, 6};
  double result[3] = {0.0};

  vec_add(vec1, vec2, result);
  printf("vec1 + vec2 -> result == {%f, %f, %f}\n", result[X], result[Y], result[Z]);
  vec_add(vec1, vec2, vec1);
  printf("vec1 + vec2 -> vec1 == {%f, %f, %f}\n", vec1[X], vec1[Y], vec1[Z]);
}


void test_in_place_normalize() {
  double vec1[3] = {1, 1, 1};
  double result[3] = {0.0};

  vec_normalize(vec1, result);
  printf("normalize(vec1) -> result == {%f, %f, %f}\n", result[X], result[Y], result[Z]);
  vec_normalize(vec1, vec1);
  printf("normalize(vec1) -> vec1 == {%f, %f, %f}\n", vec1[X], vec1[Y], vec1[Z]);
}


int main(int argc, char* argv[]) {
  test_in_place_add();
  test_in_place_normalize();
  exit(EXIT_SUCCESS);
}
