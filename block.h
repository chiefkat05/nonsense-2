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
    0, 0, 0, 0, 0,

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
    0, 0, 1, 0, 1};

typedef enum
{
    BLOCK_NULL,
    BLOCK_GRASS,
    BLOCK_ROCK
} block_id;
typedef struct
{
    // each side has it's own texture id???? makes sense... but excessive??
    // block_id id[6];
    block_id id;
    // other data things can be maybe break time and stuff, or you can put everything important in the id? and make structs or enums to handle stats based on that?
} voxel;

#endif