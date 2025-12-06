#include "chunk.h"

int chunk_get_block_from_position(int x, int y, int z)
{
    if (x < 0 || x >= CHUNK_EDGE || y < 0 || y >= CHUNK_EDGE || z < 0 || z >= CHUNK_EDGE)
    {
        return -1;
    }
    return z * CHUNK_SLAB + y * CHUNK_EDGE + x;
}
void chunk_get_position_from_block(int i, vec3 output, int line)
{
    if (i < 0 || i >= CHUNK_TOTAL)
    {
        printf("block %i does not exist in chunk.\n", i);
        return;
    }

    int z = i / (CHUNK_SLAB);
    int y = (i - (z * CHUNK_SLAB)) / CHUNK_EDGE;
    int x = i - (z * CHUNK_SLAB) - (y * CHUNK_EDGE);
    glm_vec3_copy((vec3){x, y, z}, output);
}
void chunk_generation(chunk *c, int _x, int _y, int _z)
{
    c->x = _x;
    c->y = _y;
    c->z = _z;
    // calc noise
    int landscape[CHUNK_EDGE][CHUNK_EDGE][CHUNK_EDGE];
    for (int z = 0; z < CHUNK_EDGE; ++z)
    {
        for (int y = 0; y < CHUNK_EDGE; ++y)
        {
            for (int x = 0; x < CHUNK_EDGE; ++x)
            {
                landscape[x][y][z] = BLOCK_NULL;
                
                if (y < z)
                    landscape[x][y][z] = BLOCK_GRASS;
                if (y < 2)
                    landscape[x][y][z] = BLOCK_ROCK;
                if (c->x == 0 && c->y == -1 && c->z == 0)
                    landscape[x][y][z] = BLOCK_ROCK;
            }
        }
    }

    for (int z = 0; z < CHUNK_EDGE; ++z)
    {
        for (int y = 0; y < CHUNK_EDGE; ++y)
        {
            for (int x = 0; x < CHUNK_EDGE; ++x)
            {
                c->blocks[z * CHUNK_EDGE * CHUNK_EDGE + y * CHUNK_EDGE + x].id = landscape[x][y][z];
            }
        }
    }
    c->generated = true;
}

void chunk_allocate(chunk *c)
{
    c->mesh = (int *)calloc(CHUNK_TOTAL, sizeof(block_vertices));
    verify(c->mesh, "couldn't allocate a chunk of memory", __LINE__);
}
void chunk_free(chunk *c)
{
    if (!c)
        return;

    free(c->mesh);
}

void build_chunk_mesh(chunk *c, chunk *left, chunk *right, chunk *forwards, chunk *backwards, chunk *up, chunk *down)
{
    verify(c, "cannot build chunk mesh, chunk does not exist.", __LINE__);
    verify(c->mesh, "cannot build chunk mesh, mesh pointer is NULL. Maybe it's not been allocated?", __LINE__);
    memset(c->mesh, 0, CHUNK_TOTAL * sizeof(block_vertices));

    const int chunk_slab = CHUNK_EDGE * CHUNK_EDGE;
    const int chunk_row = CHUNK_EDGE;
    int mesh_size = 0;
    for (int block_index = 0; block_index < CHUNK_TOTAL; ++block_index)
    {
        if (c->blocks[block_index].id == BLOCK_NULL)
        {
            continue;
        }
        
        vec3 block_pos = {};
        chunk_get_position_from_block(block_index, block_pos, __LINE__);
        int x = block_pos[0];
        int y = block_pos[1];
        int z = block_pos[2]; 

        int v_count = sizeof(block_vertices) / sizeof(block_vertices[0]);
        int cube_tri_count = 36;
        int vertex_data_size = v_count / cube_tri_count;

        int face_vertex_count = vertex_data_size * 6;
        int offset_cube[v_count];

        int block_mesh_size = 0;
        int normal_index = 0;

        for (int face_index = 0; face_index < v_count; face_index += face_vertex_count)
        {
            normal_index = face_index / face_vertex_count;
            bool skip_face = false;
            switch (normal_index)
            {
            case 0:
                if (z < CHUNK_EDGE - 1)
                {
                    if (c->blocks[block_index + chunk_slab].id != BLOCK_NULL)
                        skip_face = true;
                }
                else if (forwards != NULL)
                {
                    int other_chunk_block_index = block_index - CHUNK_TOTAL + chunk_slab;
                    block_id other_chunk_block_id = forwards->blocks[other_chunk_block_index].id;

                    if (other_chunk_block_id != BLOCK_NULL)
                        skip_face = true;
                }
                break;
            case 1:
                if (z > 0)
                {
                    if (c->blocks[block_index - chunk_slab].id != BLOCK_NULL)
                        skip_face = true;
                }
                else if (backwards != NULL)
                {
                    int other_chunk_block_index = block_index + CHUNK_TOTAL - chunk_slab;
                    block_id other_chunk_block_id = backwards->blocks[other_chunk_block_index].id;

                    if (other_chunk_block_id != BLOCK_NULL)
                        skip_face = true;
                }
                break;
            case 2:
                if (y < CHUNK_EDGE - 1)
                {
                    if (c->blocks[block_index + chunk_row].id != BLOCK_NULL)
                        skip_face = true;
                }
                else if (up != NULL)
                {
                    int other_chunk_block_index = block_index - chunk_slab + chunk_row;
                    block_id other_chunk_block_id = up->blocks[other_chunk_block_index].id;

                    if (other_chunk_block_id != BLOCK_NULL)
                        skip_face = true;
                }
                break;
            case 3:
                if (y > 0)
                {
                    if (c->blocks[block_index - chunk_row].id != BLOCK_NULL)
                        skip_face = true;
                }
                else if (down != NULL)
                {
                    int other_chunk_block_index = block_index + chunk_slab - chunk_row;
                    block_id other_chunk_block_id = down->blocks[other_chunk_block_index].id;

                    if (other_chunk_block_id != BLOCK_NULL)
                        skip_face = true;
                }
                break;
            case 4:
                if (x < CHUNK_EDGE - 1)
                {
                    if (c->blocks[block_index + 1].id != BLOCK_NULL)
                        skip_face = true;
                }
                else if (right != NULL)
                {
                    int other_chunk_block_index = block_index - chunk_row + 1;
                    block_id other_chunk_block_id = right->blocks[other_chunk_block_index].id;

                    if (other_chunk_block_id != BLOCK_NULL)
                        skip_face = true;
                }
                break;
            case 5:
                if (x > 0)
                {
                    if (c->blocks[block_index - 1].id != BLOCK_NULL)
                        skip_face = true;
                }
                else if (left != NULL)
                {
                    int other_chunk_block_index = block_index + chunk_row - 1;
                    block_id other_chunk_block_id = left->blocks[other_chunk_block_index].id;

                    if (other_chunk_block_id != BLOCK_NULL)
                        skip_face = true;
                }
                break;
            default:
                skip_face = true;
                break;
            }

            if (skip_face)
                continue;

            vec3 vertex_offset = {x, y, z};
                int t_x = (c->blocks[block_index].id % c->texture_width) + 1;
                int t_y = c->blocks[block_index].id / c->texture_width;
            for (int vertex_index = 0; vertex_index < face_vertex_count; vertex_index += vertex_data_size)
            {
                // position
                offset_cube[block_mesh_size + vertex_index + 0] = block_vertices[face_index + vertex_index + 0] + vertex_offset[0];
                offset_cube[block_mesh_size + vertex_index + 1] = block_vertices[face_index + vertex_index + 1] + vertex_offset[1];
                offset_cube[block_mesh_size + vertex_index + 2] = block_vertices[face_index + vertex_index + 2] + vertex_offset[2];
                // texture
                offset_cube[block_mesh_size + vertex_index + 3] = block_vertices[face_index + vertex_index + 3] + t_x;
                offset_cube[block_mesh_size + vertex_index + 4] = block_vertices[face_index + vertex_index + 4] + t_y;
            }
            block_mesh_size += face_vertex_count;
        }
        memcpy((c->mesh + mesh_size), offset_cube, sizeof(int) * block_mesh_size);
        mesh_size += block_mesh_size;
    }
    c->mesh_size = mesh_size;

    // pass to opengl
    glGenVertexArrays(1, &c->vao);
    glGenBuffers(1, &c->vbo);

    glBindVertexArray(c->vao);
    glBindBuffer(GL_ARRAY_BUFFER, c->vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * c->mesh_size, c->mesh, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_INT, GL_FALSE, 5 * sizeof(GL_INT), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_INT, GL_FALSE, 5 * sizeof(GL_INT), (void *)(3 * sizeof(GL_INT)));
    glEnableVertexAttribArray(1);
}
void update_chunk(chunk *c, chunk *left_chunk, chunk *right_chunk, chunk *forwards_chunk,
                  chunk *backwards_chunk, chunk *up_chunk, chunk *down_chunk, camera *cam,
                  int *lookat_block, int *lookat_block_normal, double *lookat_block_distance, int *lookat_chunk_normal)
{
    if (c->dirty)
    {
        build_chunk_mesh(c, left_chunk, right_chunk, forwards_chunk, backwards_chunk, up_chunk, down_chunk);
        c->dirty = false;
    }
    // double closest_block_dist = DBL_MAX;
    // int closest_block_normal = -1;
    // for (int i = 0; i < CHUNK_TOTAL; ++i)
    // {
    //     block_id chunk_block_id = c->blocks[i].id;
    //     if (chunk_block_id == BLOCK_NULL)
    //     {
    //         continue;
    //     }
    //     vec3 block_pos = {i % CHUNK_EDGE,
    //                       (i / CHUNK_EDGE) % CHUNK_EDGE,
    //                       i / (CHUNK_EDGE * CHUNK_EDGE)};
    //     vec3 chunk_pos = {c->x * CHUNK_EDGE,
    //                       c->y * CHUNK_EDGE,
    //                       c->z * CHUNK_EDGE};
    //     vec3 real_block_position = {};
    //     glm_vec3_add(chunk_pos, block_pos, real_block_position);
    //     vec3 block_min = {}, block_max = {};
    //     glm_vec3_copy(real_block_position, block_min);
    //     glm_vec3_add(real_block_position, (vec3){1.0, 1.0, 1.0}, block_max);
    //     vec3 cam_com_dir = {
    //         -cam->look_direction[0],
    //         -cam->look_direction[1],
    //         -cam->look_direction[2]};

    //     bool looking_at = ray_voxel_colliding(cam->position, cam_com_dir, block_min, block_max, &closest_block_normal);
    //     if (!looking_at)
    //         continue;
        
    //     float dist = glm_vec3_distance2(cam->position, real_block_position);
    //     if (dist < closest_block_dist)
    //     {
    //         closest_block_dist = dist;
    //         double *look_block_dist = lookat_block_distance;
    //         if (closest_block_dist < *look_block_dist)
    //         {
    //             *lookat_block = i;
    //             *lookat_block_normal = closest_block_normal;
    //             *look_block_dist = closest_block_dist;
    //             *lookat_chunk_normal = -1;

    //             if (*lookat_block_normal == 1 && block_pos[0] >= CHUNK_EDGE - 1)
    //                 *lookat_chunk_normal = 1;
    //             if (*lookat_block_normal == 0 && block_pos[0] <= 0)
    //                 *lookat_chunk_normal = 0;
    //             if (*lookat_block_normal == 3 && block_pos[1] >= CHUNK_EDGE - 1)
    //                 *lookat_chunk_normal = 3;
    //             if (*lookat_block_normal == 2 && block_pos[1] <= 0)
    //                 *lookat_chunk_normal = 2;
    //             if (*lookat_block_normal == 5 && block_pos[2] >= CHUNK_EDGE - 1)
    //                 *lookat_chunk_normal = 5;
    //             if (*lookat_block_normal == 4 && block_pos[2] <= 0)
    //                 *lookat_chunk_normal = 4;
    //         }
    //     }
    // }
}
void draw_chunk(chunk *c, shader_list *shaders)
{
    mat4 model;
    glm_mat4_identity(model);
    // transform model based on chunk position
    glm_translate(model, (vec3){c->x * CHUNK_EDGE, c->y * CHUNK_EDGE, c->z * CHUNK_EDGE});
    shader_set_mat4x4(shaders, SHADER_COMMON, "model", model);
    shader_set_int(shaders, SHADER_COMMON, "tex", 1);
    shader_set_vec2(shaders, SHADER_COMMON, "texture_size", (vec2){2.0, 1.0});

    glBindVertexArray(c->vao);
    glDrawArrays(GL_TRIANGLES, 0, c->mesh_size);
}
