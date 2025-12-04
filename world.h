#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"
#include "chunk_map.h"

extern double render_distance;

typedef struct
{
    chunk *chunk_list;
    int chunk_count;
    int world_x_count, world_y_count, world_z_count;
    unsigned int texture_id, texture_width, texture_height;
    int lookat_block, placement_block;
    int lookat_block_normal;
    int lookat_chunk;
    chunk *placement_chunk;
    chunk_hashmap chunk_map;
} world;

void world_break_block(world *w);
void world_place_block(world *w);
void world_chunk_generation(world *w, camera *cam);
void world_chunk_update(world *w, camera *cam, shader_list *shaders);
void world_exit(world *w);


#endif