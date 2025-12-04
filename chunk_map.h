#ifndef CHUNK_MAP_H
#define CHUNK_MAP_H

#include "chunk.h"

extern unsigned int world_local_edge_size;

typedef struct
{
    unsigned int count;
    chunk **arr;
} chunk_hashmap;
void chunk_map_alloc(chunk_hashmap *map, uint32 size);
void chunk_map_free(chunk_hashmap *map);
int chunk_map_function(int x, int y, int z);
void chunk_map_rehash(chunk_hashmap *map);
void chunk_map_insert(chunk_hashmap *map, int x, int y, int z, chunk *p_chunk);
void chunk_map_remove(chunk_hashmap *map, int x, int y, int z);
chunk *chunk_map_lookup(chunk_hashmap *map, int x, int y, int z);
void chunk_map_print(chunk_hashmap *map);

void chunk_map_cleanup(chunk_hashmap *map, vec3 player_pos, double render_distance);


#endif