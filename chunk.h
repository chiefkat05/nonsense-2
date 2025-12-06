#ifndef CHUNK_H
#define CHUNK_H

#include "definitions.h"
#include "block.h"
#include "physics.h"
#include "shader.h"

extern float *camera_position;

#define CHUNK_EDGE 16
#define CHUNK_SLAB 256
#define CHUNK_TOTAL 4096
typedef struct
{
    int x, y, z;
    int *mesh;
    int mesh_size;
    voxel blocks[CHUNK_TOTAL];
    bool dirty, partial_dirty;
    unsigned int vao, vbo;
    unsigned int texture_id, texture_width, texture_height;
    bool generated;
} chunk;

void chunk_generation(chunk *c, int _x, int _y, int _z);

void chunk_allocate(chunk *c);
void chunk_free(chunk *c);

void chunk_build_mesh(chunk *c, chunk *left, chunk *right, chunk *forwards, chunk *backwards, chunk *up, chunk *down);
void update_chunk(chunk *c, chunk *left_chunk, chunk *right_chunk, chunk *forwards_chunk,
                  chunk *backwards_chunk, chunk *up_chunk, chunk *down_chunk, camera *cam,
                  int *lookat_block, int *lookat_block_normal, double *lookat_block_distance, int *lookat_chunk_normal);
void draw_chunk(chunk *c, shader_list *shaders);

int chunk_get_block_from_position(int x, int y, int z);
bool chunk_get_position_from_block(int i, vec3 output, int line);

int chunk_dda_test(chunk *c, vec3 pos, vec3 dir, int *b_norm, int *ch_norm);

#endif