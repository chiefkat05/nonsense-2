#ifndef PHYSICS_H
#define PHYSICS_H

#include "definitions.h"

bool ray_voxel_colliding(vec3 point, vec3 direction, vec3 box_min, vec3 box_max, int *contact_normal);

#endif