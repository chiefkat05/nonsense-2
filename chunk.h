#ifndef CHUNK_H
#define CHUNK_H

#include "definitions.h"
#include "block.h"
#include "physics.h"
#include "shader.h"

#define CHUNK_EDGE_LENGTH 8
static const int block_limit = CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
typedef struct
{
    int x, y, z;
    int *mesh;
    int mesh_size;
    voxel blocks[CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH];
    bool dirty;
    unsigned int vao, vbo;
    unsigned int texture_id, texture_width, texture_height;
} chunk;

void chunk_generation(chunk *c, int _x, int _y, int _z);

void chunk_allocate(chunk *c);
void chunk_free(chunk *c);

void build_chunk_mesh(chunk *c, chunk *left, chunk *right, chunk *forwards, chunk *backwards, chunk *up, chunk *down);
void update_chunk(chunk *c, chunk *left_chunk, chunk *right_chunk, chunk *forwards_chunk,
                  chunk *backwards_chunk, chunk *up_chunk, chunk *down_chunk, camera *cam,
                  int *lookat_block, int *lookat_block_normal, double *lookat_block_distance, int *lookat_chunk_normal);
void draw_chunk(chunk *c, shader_list *shaders, int lookat_block);

int chunk_get_block_from_position(chunk *c, int x, int y, int z);
void chunk_get_position_from_block(chunk *c, int i, vec3 output, int line);

#endif