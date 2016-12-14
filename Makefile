CC = gcc

ifdef DEBUG
CFLAGS = -g -std=c11 -Wall -Wextra -pedantic
else
CFLAGS = -O3 -std=c11 -Wall -Wextra -pedantic
endif

LDLIBS = -lm

raycast: main.o parser.o spec.o camera.o object.o light.o pixelbuf.o ppmwrite.o vecmath.o util.o
samples: raycast
	./raycast 500 500 test_data/cone.json sample_outputs/cone.ppm
	./raycast 500 500 test_data/cylinder.json sample_outputs/cylinder.ppm
	./raycast 500 500 test_data/sphere_and_plane.json sample_outputs/sphere.ppm
	./raycast 500 500 test_data/reflect.json sample_outputs/reflect.ppm
	./raycast 500 500 test_data/refract.json sample_outputs/refract.ppm
	./raycast 500 500 test_data/mix_rr.json sample_outputs/mix_rr.ppm
	./raycast 500 500 test_data/reflect_cone.json sample_outputs/reflect_cone.ppm

main.o: spec.h camera.h object.h light.h pixelbuf.h raycast.h ppmwrite.h
raycast.o: raycast.h camera.h object.h light.h pixelbuf.h vecmath.h
ppmwrite.o: ppmwrite.h
pixelbuf.o: pixelbuf.h util.h
vecmath.o: vecmath.h util.h
light.o: light.h spec.h util.h
object.o: object.h spec.h vecmath.h util.h
camera.o: camera.h spec.h vecmath.h util.h
parser.o: parser.h spec.h util.h
spec.o: spec.h util.h
util.o: util.h

.PHONY: clean rebuild
clean:
	-rm -f *.o raycast test_parser test_objects test_lights test_camera test_vecmath example_outputs/*.ppm
rebuild: clean raycast

test_lights: spec.o parser.o light.o util.o
test_objects: object.o parser.o spec.o vecmath.o util.o
test_camera: camera.o parser.o spec.o vecmath.o util.o
test_parser: parser.o spec.o util.o
test_vecmath: vecmath.o util.o

test_vecmath.o: vecmath.h util.h
test_lights.o: spec.h parser.h light.h
test_parser.o: parser.h spec.h
test_camera.o: parser.h spec.h camera.h
test_objects.o: object.h parser.h spec.h
