#include "chunk_map.h"

void chunk_map_alloc(chunk_hashmap *map, uint32 size)
{
    map->arr = (chunk **)calloc(size, sizeof(chunk *));
    verify(map->arr, "failed to allocate chunk hashmap", __LINE__);
    map->count = size;
}
void chunk_map_free(chunk_hashmap *map)
{
    free(map->arr);
    map->count = 0;
}
int chunk_map_function(int x, int y, int z)
{
    // ! idea: make this 'not' perfectly unique, by subtracting the player position or something, keeping the hash function always
    // centered around where the player is. That will make the hash map stay as small as the render_area (render distance) at all times
    // but don't bother rushing to implement this, just get it done before 1.0 release
    int out_x = abs(x) * 2;
    int out_y = abs(y) * 2;
    int out_z = abs(z) * 2;
    if (x < 0) out_x += 1;
    if (y < 0) out_y += 1;
    if (z < 0) out_z += 1;

    int index = out_z * world_local_edge_size * world_local_edge_size + out_y * world_local_edge_size + out_x;

    return index;
}
void chunk_map_rehash(chunk_hashmap *map)
{
    int new_size = map->count * 2;
    chunk **new_map_arr = (chunk **)calloc(new_size, sizeof(chunk *));
    verify(new_map_arr, "failed to allocate memory for chunk map rehash", new_size);
    memcpy(new_map_arr, map->arr, map->count * sizeof(chunk *));
    free(map->arr);
    map->arr = new_map_arr;
    map->count *= 2;
}
void chunk_map_insert(chunk_hashmap *map, int x, int y, int z, chunk *p_chunk)
{
    unsigned int index = chunk_map_function(x, y, z);

    while (index >= map->count)
    {
        chunk_map_rehash(map);
    }
    map->arr[index] = p_chunk;
}
void chunk_map_remove(chunk_hashmap *map, int x, int y, int z)
{
    unsigned int index = chunk_map_function(x, y, z);
    if (index >= map->count || !map->arr[index])
        return;

    map->arr[index] = NULL;
}
chunk *chunk_map_lookup(chunk_hashmap *map, int x, int y, int z)
{
    unsigned int index = chunk_map_function(x, y, z);
    if (index >= map->count || !map->arr[index])
    {
        return NULL;
    }
        
    return map->arr[index];
}
void chunk_map_print(chunk_hashmap *map)
{
    printf("--------------------hashmap data--------------------\n");
    for (int i = 0; i < map->count; ++i)
    {
        if (map->arr[i] == NULL)
            continue;

        printf("%i / %i chunk at pointer %p\n", i, map->count, (void *)map->arr[i]);
        printf("pos: %i %i %i\n", map->arr[i]->x, map->arr[i]->y, map->arr[i]->z);
    }
    printf("---end hashmap data. hashmap total size: %lu bytes---\n", map->count * sizeof(chunk *));
}

void chunk_map_cleanup(chunk_hashmap *map, vec3 player_pos, double render_distance)
{
    for (int i = 0; i < map->count; ++i)
    {
        if (!map->arr[i])
            continue;

        vec3 chunk_pos = {map->arr[i]->x * CHUNK_EDGE_LENGTH, map->arr[i]->y * CHUNK_EDGE_LENGTH, map->arr[i]->z * CHUNK_EDGE_LENGTH};
        double distance = glm_vec3_distance2(player_pos, chunk_pos);
        if (distance > render_distance)
        {
            chunk_map_remove(map, map->arr[i]->x, map->arr[i]->y, map->arr[i]->z);
        }
    }
}
