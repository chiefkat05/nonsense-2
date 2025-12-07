#ifndef BLOCK_H
#define BLOCK_H

#include "definitions.h"

static const int block_vertices[] = {
    // pos z
    0, 0, 1, 0, 0,
    1, 0, 1, 1, 0,
    1, 1, 1, 1, 1,

    0, 0, 1, 0, 0,
    1, 1, 1, 1, 1,
    0, 1, 1, 0, 1,

    // neg z
    0, 0, 0, 1, 0,
    1, 1, 0, 0, 1,
    1, 0, 0, 0, 0,

    0, 0, 0, 1, 0,
    0, 1, 0, 1, 1,
    1, 1, 0, 0, 1,

    // pos y
    1, 1, 0, 0, 0,
    0, 1, 1, 1, 1,
    1, 1, 1, 0, 1,

    1, 1, 0, 0, 0,
    0, 1, 0, 1, 0,
    0, 1, 1, 1, 1,

    // neg y
    0, 0, 0, 0, 0,
    1, 0, 0, 1, 0,
    1, 0, 1, 1, 1,

    0, 0, 0, 0, 0,
    1, 0, 1, 1, 1,
    0, 0, 1, 0, 1,

    // pos x
    1, 0, 1, 0, 0,
    1, 0, 0, 1, 0,
    1, 1, 1, 0, 1,

    1, 0, 0, 1, 0,
    1, 1, 0, 1, 1,
    1, 1, 1, 0, 1,

    // neg x
    0, 0, 1, 1, 0,
    0, 1, 1, 1, 1,
    0, 1, 0, 0, 1,

    0, 0, 1, 1, 0,
    0, 1, 0, 0, 1,
    0, 0, 0, 0, 0};

static const vec2 texture_locs[] =
{
    {-1.0, -1.0},
    {0.0, 1.0},
    {0.0, 1.0},
    {2.0, 1.0},
    {1.0, 1.0}
};

typedef enum
{
    BLOCK_NULL,
    BLOCK_GRASS,
    BLOCK_FLOWER,
    BLOCK_ROCK,
    BLOCK_CHERRY
} block_id;
typedef struct
{
    block_id id;
    int mesh_location, size;
    bool dirty;
} voxel;

#endif