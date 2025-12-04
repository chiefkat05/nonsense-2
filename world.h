#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"
#include "chunk_map.h"

extern double render_distance;

typedef struct
{
    unsigned int texture_id, texture_width, texture_height;
    int lookat_block, placement_block;
    int lookat_block_normal;
    chunk *lookat_chunk, *placement_chunk;
    int lookat_chunk_normal;
    chunk_hashmap chunk_map;
} world;

void world_break_block(world *w);
void world_place_block(world *w);
void world_chunk_update(world *w, camera *cam, shader_list *shaders, double *lookat_block_distance);
void world_exit(world *w);


#endif