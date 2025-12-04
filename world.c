#include "world.h"

void world_break_block(world *w)
{
    if (w->lookat_block < 0)
        return;
    w->chunk_list[w->lookat_chunk].blocks[w->lookat_block].id = BLOCK_NULL;
    w->chunk_list[w->lookat_chunk].dirty = true;

    vec3 block_pos = {};
    chunk_get_position_from_block(&w->chunk_list[w->lookat_chunk], w->lookat_block, block_pos, __LINE__);

    int x = block_pos[0];
    int y = block_pos[1];
    int z = block_pos[2];

    int chunk_update_normal_x = 0, chunk_update_normal_y = 0, chunk_update_normal_z = 0;

    if (x == 0)
        chunk_update_normal_x = -1;
    if (x + 1 >= CHUNK_EDGE_LENGTH)
        chunk_update_normal_x = 1;
    if (y == 0)
        chunk_update_normal_y = -1;
    if (y + 1 >= CHUNK_EDGE_LENGTH)
        chunk_update_normal_y = 1;
    if (z == 0)
        chunk_update_normal_z = -1;
    if (z + 1 >= CHUNK_EDGE_LENGTH)
        chunk_update_normal_z = 1;

    int chx = w->chunk_list[w->lookat_chunk].x;
    int chy = w->chunk_list[w->lookat_chunk].y;
    int chz = w->chunk_list[w->lookat_chunk].z;

    if (chunk_update_normal_x != 0)
    {
        chunk *p_chunk = chunk_map_lookup(&w->chunk_map, chx + chunk_update_normal_x, chy, chz);
        if (p_chunk)
            p_chunk->dirty = true;
    }
    if (chunk_update_normal_y != 0)
    {
        chunk *p_chunk = chunk_map_lookup(&w->chunk_map, chx, chy + chunk_update_normal_y, chz);
        if (p_chunk)
            p_chunk->dirty = true;
    }
    if (chunk_update_normal_z != 0)
    {
        chunk *p_chunk = chunk_map_lookup(&w->chunk_map, chx, chy, chz + chunk_update_normal_z);
        if (p_chunk)
            p_chunk->dirty = true;
    }
}
void world_place_block(world *w)
{
    int chunk_size = CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
    int slab_size = CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
    int row_size = CHUNK_EDGE_LENGTH;

    int new_block_pos = -1;
    int x = w->placement_block % CHUNK_EDGE_LENGTH;
    int y = (w->placement_block / CHUNK_EDGE_LENGTH) % CHUNK_EDGE_LENGTH;
    int z = w->placement_block / (CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH);
    switch (w->lookat_block_normal)
    {
    case 5:
        // pos z
        new_block_pos = w->placement_block + slab_size;
        ++z;
        break;
    case 4:
        // neg z
        new_block_pos = w->placement_block - slab_size;
        --z;
        break;
    case 3:
        // pos y
        new_block_pos = w->placement_block + row_size;
        ++y;
        break;
    case 2:
        // neg y
        new_block_pos = w->placement_block - row_size;
        --y;
        break;
    case 1:
        // pos x
        new_block_pos = w->placement_block + 1;
        ++x;
        break;
    case 0:
        // neg x
        new_block_pos = w->placement_block - 1;
        --x;
        break;
    default:
        break;
    }

    if (new_block_pos > -1 && new_block_pos < chunk_size && w->placement_chunk)
    {
        w->placement_chunk->blocks[new_block_pos].id = BLOCK_ROCK;
        w->placement_chunk->dirty = true;
    }
}

void world_new_chunk(world *w, int x, int y, int z)
{
    chunk_hashmap *map = &w->chunk_map;
    if (chunk_map_lookup(map, x, y, z))
        return;

    chunk *new_chunklist = realloc(w->chunk_list, (w->chunk_count + 1) * sizeof(chunk));
    verify(new_chunklist, "failed to realloc chunk list", __LINE__);
    w->chunk_list = new_chunklist;
    chunk *current = &w->chunk_list[w->chunk_count];

    chunk_generation(current, x, y, z);
    chunk_allocate(current);
    current->texture_id = w->texture_id;
    current->texture_width = w->texture_width;
    current->texture_height = w->texture_height;

    chunk_map_insert(map, x, y, z, current);

    chunk *left = chunk_map_lookup(map, x - 1, y, z);
    chunk *right = chunk_map_lookup(map, x + 1, y, z);
    chunk *forwards = chunk_map_lookup(map, x, y, z + 1);
    chunk *backwards = chunk_map_lookup(map, x, y, z - 1);
    chunk *up = chunk_map_lookup(map, x, y + 1, z);
    chunk *down = chunk_map_lookup(map, x, y - 1, z);
    build_chunk_mesh(current, left, right, forwards, backwards, up, down);

    w->chunk_count++;
}
void world_chunk_generation(world *w, camera *cam)
{
    w->world_x_count = 10;
    w->world_y_count = 1;
    w->world_z_count = 10;
    int total_world_size = w->world_x_count * w->world_y_count * w->world_z_count;
    w->chunk_list = (chunk *)calloc(total_world_size, sizeof(chunk));
    verify(w->chunk_list, "failed to allocate world chunk area", __LINE__); // windows build -> x11 fps test (let's go)

    chunk_map_alloc(&w->chunk_map, total_world_size * 2);

    // generation step
    for (int z = -5; z < 5; ++z)
    {
        for (int y = -1; y < 0; ++y)
        {
            for (int x = -5; x < 5; ++x)
            {
                chunk *current_chunk = &w->chunk_list[w->chunk_count];

                chunk_generation(current_chunk, x, y, z);
                chunk_allocate(current_chunk);
                current_chunk->texture_id = w->texture_id;
                current_chunk->texture_width = w->texture_width;
                current_chunk->texture_height = w->texture_height;
                
                chunk_map_insert(&w->chunk_map, x, y, z, current_chunk);

                ++w->chunk_count;
            }
        }
    }

    for (int i = 0; i < w->chunk_count; ++i)
    {
        int x = w->chunk_list[i].x, y = w->chunk_list[i].y, z = w->chunk_list[i].z;

        chunk *this_chunk = chunk_map_lookup(&w->chunk_map, x, y, z);
        if (!this_chunk)
            continue;
        
        chunk *left_chunk = chunk_map_lookup(&w->chunk_map, x - 1, y, z);
        chunk *right_chunk = chunk_map_lookup(&w->chunk_map, x + 1, y, z);
        chunk *forwards_chunk = chunk_map_lookup(&w->chunk_map, x, y, z + 1);
        chunk *backwards_chunk = chunk_map_lookup(&w->chunk_map, x, y, z - 1);
        chunk *up_chunk = chunk_map_lookup(&w->chunk_map, x, y + 1, z);
        chunk *down_chunk = chunk_map_lookup(&w->chunk_map, x, y - 1, z);
        
        build_chunk_mesh(this_chunk, left_chunk, right_chunk, forwards_chunk, backwards_chunk, up_chunk, down_chunk);
    }
    chunk_map_cleanup(&w->chunk_map, cam->pos, render_distance);
}
void world_chunk_update(world *w, camera *cam, shader_list *shaders)
{
    double lookat_block_distance = DBL_MAX;
    for (int i = 0; i < w->chunk_count; ++i)
    {
        int x = w->chunk_list[i].x;
        int y = w->chunk_list[i].y;
        int z = w->chunk_list[i].z;

        int chunk_pos_x = x * CHUNK_EDGE_LENGTH;
        int chunk_pos_y = y * CHUNK_EDGE_LENGTH;
        int chunk_pos_z = z * CHUNK_EDGE_LENGTH;

        // make this distance2 I think
        // ok I did
        double cam_to_chunk_dist = glm_vec3_distance2((vec3){chunk_pos_x, chunk_pos_y, chunk_pos_z}, cam->pos);
        if (cam_to_chunk_dist > render_distance)
            continue;

        if (!chunk_map_lookup(&w->chunk_map, x, y, z))
            chunk_map_insert(&w->chunk_map, x, y, z, &w->chunk_list[i]);

        int temp_lookat = -1;
        int temp_lookat_chunk_norm = -1;

        chunk *right_chunk = chunk_map_lookup(&w->chunk_map, x + 1, y, z);
        chunk *left_chunk = chunk_map_lookup(&w->chunk_map, x - 1, y, z);
        chunk *forwards_chunk = chunk_map_lookup(&w->chunk_map, x, y, z + 1);
        chunk *backwards_chunk = chunk_map_lookup(&w->chunk_map, x, y, z - 1);
        chunk *up_chunk = chunk_map_lookup(&w->chunk_map, x, y + 1, z);
        chunk *down_chunk = chunk_map_lookup(&w->chunk_map, x, y - 1, z);

        update_chunk(&w->chunk_list[i], left_chunk, right_chunk, forwards_chunk, backwards_chunk, up_chunk, down_chunk,
                        cam, &temp_lookat, &w->lookat_block_normal, &lookat_block_distance, &temp_lookat_chunk_norm);
        if (temp_lookat != -1)
        {
            w->lookat_chunk = i;
            vec3 current_chunk_position = {w->chunk_list[i].x, w->chunk_list[i].y, w->chunk_list[i].z};
            vec3 new_chunk_position = {};
            glm_vec3_copy(current_chunk_position, new_chunk_position);
            w->placement_block = temp_lookat;
            switch (temp_lookat_chunk_norm)
            {
            case 0: // x
                --new_chunk_position[0];
                w->placement_block += CHUNK_EDGE_LENGTH;
                break;
            case 1:
                ++new_chunk_position[0];
                w->placement_block -= CHUNK_EDGE_LENGTH;
                break;
            case 2: // y
                --new_chunk_position[1];
                w->placement_block += CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
                break;
            case 3:
                ++new_chunk_position[1];
                w->placement_block -= CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
                break;
            case 4: // z
                --new_chunk_position[2];
                w->placement_block += (CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH);
                break;
            case 5:
                ++new_chunk_position[2];
                w->placement_block -= (CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH);
                break;
            }
            w->lookat_block = temp_lookat;
            w->placement_chunk = chunk_map_lookup(&w->chunk_map, new_chunk_position[0], new_chunk_position[1], new_chunk_position[2]);
        }
        draw_chunk(&w->chunk_list[i], shaders, w->lookat_block);
    }

    chunk_map_cleanup(&w->chunk_map, cam->pos, render_distance);

    // this is run after the 
    if (cam->pos[0] > 2 && !chunk_map_lookup(&w->chunk_map, 5, -1, 0))
    {
        world_new_chunk(w, 5, -1, 0);
    }
}
void world_exit(world *w)
{
    // save world to file here. Remember that the world does not need to be rendered to be loaded... the generated blocks are all
    // you need to send to a save file

    for (int i = 0; i < w->chunk_count; ++i)
    {
        chunk_free(&w->chunk_list[i]);
    }
    free(w->chunk_list);
    chunk_map_free(&w->chunk_map);

    free(w);
}