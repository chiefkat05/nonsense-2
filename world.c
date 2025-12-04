#include "world.h"

void world_break_block(world *w)
{
    if (w->lookat_block < 0 || !w->lookat_chunk)
        return;
    w->lookat_chunk->blocks[w->lookat_block].id = BLOCK_NULL;
    w->lookat_chunk->dirty = true;

    vec3 block_pos = {};
    chunk_get_position_from_block(w->lookat_chunk, w->lookat_block, block_pos, __LINE__);

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

    int chx = w->lookat_chunk->x;
    int chy = w->lookat_chunk->y;
    int chz = w->lookat_chunk->z;

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
void world_chunk_update(world *w, camera *cam, shader_list *shaders, double *lookat_block_distance)
{
    int z_min = (int)(cam->pos[2] / CHUNK_EDGE_LENGTH) - 5;
    int z_max = (int)(cam->pos[2] / CHUNK_EDGE_LENGTH) + 5;
    int x_min = (int)(cam->pos[0] / CHUNK_EDGE_LENGTH) - 5;
    int x_max = (int)(cam->pos[0] / CHUNK_EDGE_LENGTH) + 5;
    // maybe?
    int y_min = (int)(cam->pos[1] / CHUNK_EDGE_LENGTH) - 5;
    int y_max = (int)(cam->pos[1] / CHUNK_EDGE_LENGTH) + 5;
    for (int z = z_min; z < z_max; ++z)
    {
        for (int y = -1; y < 0; ++y)
        {
            for (int x = x_min; x < x_max; ++x)
            {
                vec3 chunk_position = {x * CHUNK_EDGE_LENGTH, y * CHUNK_EDGE_LENGTH, z * CHUNK_EDGE_LENGTH};
                double cam_dist = glm_vec3_distance2(chunk_position, cam->pos);
                if (cam_dist > render_distance)
                {
                    continue;
                }

                chunk *current = chunk_map_lookup(&w->chunk_map, x, y, z);
                bool new_chunk = false;
                if (!current)
                {
                    current = (chunk *)calloc(1, sizeof(chunk));
                    chunk_generation(current, x, y, z);
                    chunk_allocate(current);
                    current->texture_id = w->texture_id;
                    current->texture_width = w->texture_width;
                    current->texture_height = w->texture_height;

                    chunk_map_insert(&w->chunk_map, x, y, z, current);
                    current->dirty = true;
                    new_chunk = true;
                }

                chunk *right = chunk_map_lookup(&w->chunk_map, x + 1, y, z);
                chunk *left = chunk_map_lookup(&w->chunk_map, x - 1, y, z);
                chunk *forwards = chunk_map_lookup(&w->chunk_map, x, y, z + 1);
                chunk *backwards = chunk_map_lookup(&w->chunk_map, x, y, z - 1);
                chunk *up = chunk_map_lookup(&w->chunk_map, x, y + 1, z);
                chunk *down = chunk_map_lookup(&w->chunk_map, x, y - 1, z);
                
                if (new_chunk)
                {
                    if (right)
                        right->dirty = true;
                    if (left)
                        left->dirty = true;
                    if (forwards)
                        forwards->dirty = true;
                    if (backwards)
                        backwards->dirty = true;
                    if (up)
                        up->dirty = true;
                    if (down)
                        down->dirty = true;
                }
                
                int temp_lookat = -1;
                update_chunk(current, left, right, forwards, backwards, up, down,
                                cam, &temp_lookat, &w->lookat_block_normal, lookat_block_distance, &w->lookat_chunk_normal);

                draw_chunk(current, shaders);

                if (temp_lookat == -1)
                    continue;

                w->lookat_chunk = current;
                vec3 current_chunk_position = {current->x, current->y, current->z};
                vec3 new_chunk_position = {};
                glm_vec3_copy(current_chunk_position, new_chunk_position);
                w->placement_block = temp_lookat;
                switch (w->lookat_chunk_normal)
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
        }
    }
}
void world_exit(world *w)
{
    // save world to file here.
    chunk_map_free(&w->chunk_map);
}