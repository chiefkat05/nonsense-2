#include "world.h"

void world_break_block(world *w)
{
    if (w->lookat_block < 0 || w->lookat_block > CHUNK_TOTAL || !w->lookat_chunk)
        return;
    w->lookat_chunk->blocks[w->lookat_block].id = BLOCK_NULL;
    // w->lookat_chunk->blocks[w->lookat_block].dirty = true;
    // w->lookat_chunk->partial_dirty = true;
    w->lookat_chunk->dirty = true;

    vec3 block_pos = {};
    chunk_get_position_from_block(w->lookat_block, block_pos, __LINE__);

    int x = block_pos[0];
    int y = block_pos[1];
    int z = block_pos[2];
    // int leftb = chunk_get_block_from_position(x - 1, y, z);
    // int rightb = chunk_get_block_from_position(x + 1, y, z);
    // int forwardsb = chunk_get_block_from_position(x, y, z + 1);
    // int backwardsb = chunk_get_block_from_position(x, y, z - 1);
    // int upb = chunk_get_block_from_position(x, y + 1, z);
    // int downb = chunk_get_block_from_position(x, y - 1, z);

    // if (leftb != -1)
    //     w->lookat_chunk->blocks[leftb].dirty = true;
    // if (rightb != -1)
    //     w->lookat_chunk->blocks[rightb].dirty = true;
    // if (forwardsb != -1)
    //     w->lookat_chunk->blocks[forwardsb].dirty = true;
    // if (backwardsb != -1)
    //     w->lookat_chunk->blocks[backwardsb].dirty = true;
    // if (upb != -1)
    //     w->lookat_chunk->blocks[upb].dirty = true;
    // if (downb != -1)
    //     w->lookat_chunk->blocks[downb].dirty = true;

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
    if (w->placement_block > -1 && w->placement_block < CHUNK_TOTAL && w->placement_chunk)
    {
        vec3 block_pos = {};
        chunk_get_position_from_block(w->placement_block, block_pos, __LINE__);
        vec3 look_block_pos = {};
        chunk_get_position_from_block(w->lookat_block, look_block_pos, __LINE__);

        if (w->placement_chunk->blocks[w->placement_block].id != BLOCK_NULL)
        {
            printf("warning: placing a block where it shouldn't be possible. block %f %f %f is already id %i\n",
                block_pos[0], block_pos[1], block_pos[2], w->placement_chunk->blocks[w->placement_block].id);
        }
        w->placement_chunk->blocks[w->placement_block].id = BLOCK_CHERRY;
        w->placement_chunk->dirty = true;
    }
}
void world_chunk_manager(world *w, int current_tick)
{

}
void world_chunk_update(world *w, int current_tick)
{
    int neg_world_vision = -(world_local_edge_size / 2);
    int pos_world_vision = (world_local_edge_size / 2);
    int z_min = ((int)w->cam.real_position[2] / CHUNK_EDGE) + neg_world_vision;
    int z_max = ((int)w->cam.real_position[2] / CHUNK_EDGE) + pos_world_vision;
    int x_min = ((int)w->cam.real_position[0] / CHUNK_EDGE) + neg_world_vision;
    int x_max = ((int)w->cam.real_position[0] / CHUNK_EDGE) + pos_world_vision;

    // having y_min and y_max be anything other than -1 to 1 makes some or many (or all) of the pointers to the chunks
    // inside the chunkmap to randomly(? seems consistant) point to a different chunk, thus causing the update
    // array to skip a bunch of chunks and draw chunks in the wrong place because when it called the hash function
    // the hash function gave it back the pointer at the right index... which happens to be the same pointer as a different
    // chunk for some reason. Why this happens is unknown, and frankly nothing should be causing it and I'm pissed.
    // why it specifically doesn't break when y is between these values is also illogical, but less illogical than
    // the bug itself since the bug should not be possible on this mortal plane.
    int y_min = ((int)w->cam.position[1] / CHUNK_EDGE) + neg_world_vision;
    int y_max = ((int)w->cam.position[1] / CHUNK_EDGE) + pos_world_vision;
    // int y_min = -1;
    // int y_max = 0;
    // printf("------xmin=%i|xmax=%i--------ymin=%i|ymax=%i-----------zmin=%i|zmax=%i------\n",
            // x_min, x_max, y_min, y_max, z_min, z_max);
    w->closest_block_distance = DBL_MAX;

    bool new_chunk_tick_used = false;

    for (int z = z_min; z < z_max; ++z)
    {
        for (int y = y_min; y < y_max; ++y)
        {
            for (int x = x_min; x < x_max; ++x)
            {
                chunk *right = chunk_map_lookup(&w->chunk_map, x + 1, y, z);
                chunk *left = chunk_map_lookup(&w->chunk_map, x - 1, y, z);
                chunk *forwards = chunk_map_lookup(&w->chunk_map, x, y, z + 1);
                chunk *backwards = chunk_map_lookup(&w->chunk_map, x, y, z - 1);
                chunk *up = chunk_map_lookup(&w->chunk_map, x, y + 1, z);
                chunk *down = chunk_map_lookup(&w->chunk_map, x, y - 1, z);
                
                int current_id = chunk_map_function(x, y, z);
                chunk *current = chunk_map_lookup(&w->chunk_map, x, y, z);

                // if (current_tick % 10 == 0 && !new_chunk_tick_used)
                // {
                    // if (current)
                    // {
                    //     if (w->chunk_map.arr[current_id]->x != x || w->chunk_map.arr[current_id]->y != y || w->chunk_map.arr[current_id]->z != z)
                    //     {
                    //         vec3 loc = {current->x, current->y, current->z};
                    //         printf("%i %i %i problem, %f %f %f\n", x, y, z, loc[0], loc[1], loc[2]);
                    //         // chunk_map_remove(&w->chunk_map, x, y, z);
                    //         // current = NULL;
                    //     }
                    // }

                    bool new_chunk = false;
                    // if (!w->chunk_map.arr[current_id])
                    // {
                    //     w->chunk_map.arr[current_id] = (chunk *)calloc(1, sizeof(chunk));
                    //     chunk_generation(w->chunk_map.arr[current_id], x, y, z);
                    //     chunk_allocate(w->chunk_map.arr[current_id]);
                    //     w->chunk_map.arr[current_id]->texture_id = w->texture_id;
                    //     w->chunk_map.arr[current_id]->texture_width = w->texture_width;
                    //     w->chunk_map.arr[current_id]->texture_height = w->texture_height;

                    //     chunk_map_insert(&w->chunk_map, w->chunk_map.arr[current_id], __LINE__);
                    //     w->chunk_map.arr[current_id]->dirty = true;
                    //     new_chunk = true;
                    // }
                    if (!current)
                    {
                        current = (chunk *)calloc(1, sizeof(chunk));
                        chunk_generation(current, x, y, z);
                        chunk_allocate(current);
                        current->texture_id = w->texture_id;
                        current->texture_width = w->texture_width;
                        current->texture_height = w->texture_height;

                        chunk_map_insert(&w->chunk_map, current, __LINE__);
                        current->dirty = true;
                        new_chunk = true;
                    }
                    
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
                    // new_chunk_tick_used = true;
                // }
                if (!current)
                    continue;

                int temp_lookat = -1;
                update_chunk(current, left, right, forwards, backwards, up, down,
                                &w->cam, &temp_lookat, &w->lookat_block_normal, &w->closest_block_distance, &w->lookat_chunk_normal);

                // if (temp_lookat == -1) // some errors with trying to place blocks at certain angles please make sure it's all up to code here
                //     continue;

                // w->lookat_chunk = current;
                // vec3 current_chunk_position = {current->x, current->y, current->z};
                // vec3 new_chunk_position = {};
                // glm_vec3_copy(current_chunk_position, new_chunk_position);
                // w->placement_block = temp_lookat;
                // switch (w->lookat_chunk_normal)
                // {
                // case 0: // x
                //     --new_chunk_position[0];
                //     w->placement_block += CHUNK_EDGE;
                //     break;
                // case 1:
                //     ++new_chunk_position[0];
                //     w->placement_block -= CHUNK_EDGE;
                //     break;
                // case 2: // y
                //     --new_chunk_position[1];
                //     w->placement_block += CHUNK_EDGE * CHUNK_EDGE;
                //     break;
                // case 3:
                //     ++new_chunk_position[1];
                //     w->placement_block -= CHUNK_EDGE * CHUNK_EDGE;
                //     break;
                // case 4: // z
                //     --new_chunk_position[2];
                //     w->placement_block += CHUNK_TOTAL;
                //     break;
                // case 5:
                //     ++new_chunk_position[2];
                //     w->placement_block -= CHUNK_TOTAL;
                //     break;
                // }
                // switch (w->lookat_block_normal)
                // {
                // case 5:
                //     // pos z
                //     w->placement_block += CHUNK_SLAB;
                //     break;
                // case 4:
                //     // neg z
                //     w->placement_block -= CHUNK_SLAB;
                //     break;
                // case 3:
                //     // pos y
                //     w->placement_block += CHUNK_EDGE;
                //     break;
                // case 2:
                //     // neg y
                //     w->placement_block -= CHUNK_EDGE;
                //     break;
                // case 1:
                //     // pos x
                //     w->placement_block += 1;
                //     break;
                // case 0:
                //     // neg x
                //     w->placement_block -= 1;
                //     break;
                // default:
                //     break;
                // }
                // w->lookat_block = temp_lookat;
                // w->placement_chunk = chunk_map_lookup(&w->chunk_map, new_chunk_position[0], new_chunk_position[1], new_chunk_position[2]);
            }
        }
    }
    // chunk_map_print(&w->chunk_map);
    // chunk_duplicate_chunk
}
void world_draw(world *w)
{
    // make this a function or something
    int neg_world_vision = -(world_local_edge_size / 2);
    int pos_world_vision = (world_local_edge_size / 2);
    int z_min = ((int)w->cam.real_position[2] / CHUNK_EDGE) + neg_world_vision;
    int z_max = ((int)w->cam.real_position[2] / CHUNK_EDGE) + pos_world_vision;
    int x_min = ((int)w->cam.real_position[0] / CHUNK_EDGE) + neg_world_vision;
    int x_max = ((int)w->cam.real_position[0] / CHUNK_EDGE) + pos_world_vision;
    // maybe?
    int y_min = ((int)w->cam.real_position[1] / CHUNK_EDGE) + neg_world_vision;
    int y_max = ((int)w->cam.real_position[1] / CHUNK_EDGE) + pos_world_vision;

    // int y_min = -1;
    // int y_max = 0;

    for (int z = z_min; z < z_max; ++z)
    {
        for (int y = y_min; y < y_max; ++y)
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