#include "world.h"

void world_break_block(world *w)
{
    if (w->lookat_block < 0 || w->lookat_block > CHUNK_TOTAL || !w->lookat_chunk)
        return;
    w->lookat_chunk->blocks[w->lookat_block].id = BLOCK_NULL;
    w->lookat_chunk->dirty = true;

    vec3 block_pos = {};
    chunk_get_position_from_block(w->lookat_block, block_pos, __LINE__);

    int x = block_pos[0];
    int y = block_pos[1];
    int z = block_pos[2];

    int chunk_update_normal_x = 0, chunk_update_normal_y = 0, chunk_update_normal_z = 0;

    if (x == 0)
        chunk_update_normal_x = -1;
    if (x + 1 >= CHUNK_EDGE)
        chunk_update_normal_x = 1;
    if (y == 0)
        chunk_update_normal_y = -1;
    if (y + 1 >= CHUNK_EDGE)
        chunk_update_normal_y = 1;
    if (z == 0)
        chunk_update_normal_z = -1;
    if (z + 1 >= CHUNK_EDGE)
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
    int slab_size = CHUNK_EDGE * CHUNK_EDGE;
    int row_size = CHUNK_EDGE;

    int new_block_pos = -1;
    int x = w->placement_block % CHUNK_EDGE;
    int y = (w->placement_block / CHUNK_EDGE) % CHUNK_EDGE;
    int z = w->placement_block / (CHUNK_EDGE * CHUNK_EDGE);
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

    if (new_block_pos > -1 && new_block_pos < CHUNK_TOTAL && w->placement_chunk)
    {
        w->placement_chunk->blocks[new_block_pos].id = BLOCK_ROCK;
        w->placement_chunk->dirty = true;
    }
}
void world_chunk_update(world *w, double *lookat_block_distance)
{
    int neg_world_vision = -(world_local_edge_size / 2);
    int pos_world_vision = (world_local_edge_size / 2);
    int z_min = (int)(w->cam.position[2] / CHUNK_EDGE) + neg_world_vision;
    int z_max = (int)(w->cam.position[2] / CHUNK_EDGE) + pos_world_vision;
    int x_min = (int)(w->cam.position[0] / CHUNK_EDGE) + neg_world_vision;
    int x_max = (int)(w->cam.position[0] / CHUNK_EDGE) + pos_world_vision;
    // maybe?
    // int y_min = (int)(w->cam.position[1] / CHUNK_EDGE) + neg_world_vision;
    // int y_max = (int)(w->cam.position[1] / CHUNK_EDGE) + pos_world_vision;
    for (int z = z_min; z < z_max; ++z)
    {
        for (int y = -1; y < 0; ++y)
        {
            for (int x = x_min; x < x_max; ++x)
            {
                vec3 chunk_position = {((double)x + 0.5) * (double)CHUNK_EDGE,
                                       ((double)y + 0.5) * (double)CHUNK_EDGE,
                                       ((double)z + 0.5) * (double)CHUNK_EDGE};
                double cam_dist = glm_vec3_distance2(chunk_position, w->cam.position);
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
                                &w->cam, &temp_lookat, &w->lookat_block_normal, lookat_block_distance, &w->lookat_chunk_normal);

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
                    w->placement_block += CHUNK_EDGE;
                    break;
                case 1:
                    ++new_chunk_position[0];
                    w->placement_block -= CHUNK_EDGE;
                    break;
                case 2: // y
                    --new_chunk_position[1];
                    w->placement_block += CHUNK_EDGE * CHUNK_EDGE;
                    break;
                case 3:
                    ++new_chunk_position[1];
                    w->placement_block -= CHUNK_EDGE * CHUNK_EDGE;
                    break;
                case 4: // z
                    --new_chunk_position[2];
                    w->placement_block += CHUNK_TOTAL;
                    break;
                case 5:
                    ++new_chunk_position[2];
                    w->placement_block -= CHUNK_TOTAL;
                    break;
                }
                w->lookat_block = temp_lookat;
                w->placement_chunk = chunk_map_lookup(&w->chunk_map, new_chunk_position[0], new_chunk_position[1], new_chunk_position[2]);
            }
        }
    }
}
void world_draw(world *w)
{
    // make this a function or something
    int neg_world_vision = -(world_local_edge_size / 2);
    int pos_world_vision = (world_local_edge_size / 2);
    int z_min = (int)(w->cam.real_position[2] / CHUNK_EDGE) + neg_world_vision;
    int z_max = (int)(w->cam.real_position[2] / CHUNK_EDGE) + pos_world_vision;
    int x_min = (int)(w->cam.real_position[0] / CHUNK_EDGE) + neg_world_vision;
    int x_max = (int)(w->cam.real_position[0] / CHUNK_EDGE) + pos_world_vision;
    // maybe?
    // int y_min = (int)(w->cam.real_position[1] / CHUNK_EDGE) + neg_world_vision;
    // int y_max = (int)(w->cam.real_position[1] / CHUNK_EDGE) + pos_world_vision;

    for (int z = z_min; z < z_max; ++z)
    {
        for (int y = -1; y < 0; ++y)
        {
            for (int x = x_min; x < x_max; ++x)
            {
                chunk *current = chunk_map_lookup(&w->chunk_map, x, y, z);
                if (!current)
                    continue;


                draw_chunk(current, &w->shaders);
            }
        }
    }
}
void world_exit(world *w)
{
    // save world to file here.
    chunk_map_free(&w->chunk_map);
}