#ifndef RAYCAST_HEADER
#define RAYCAST_HEADER 1

#include "pixelbuf.h"
#include "camera.h"
#include "object.h"
#include "light.h"

PixelBufRef raycast(CameraRef, ObjectRef*, LightRef*, int, int);

#endif
