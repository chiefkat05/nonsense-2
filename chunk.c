#include "chunk.h"

void chunk_generation(chunk *c, int _x, int _y, int _z)
{
    c->x = _x;
    c->y = _y;
    c->z = _z;
    for (int z = 0; z < CHUNK_EDGE_LENGTH; ++z)
    {
        for (int y = 0; y < CHUNK_EDGE_LENGTH; ++y)
        {
            for (int x = 0; x < CHUNK_EDGE_LENGTH; ++x)
            {
                voxel v = {
                    .id = BLOCK_GRASS,
                };
                // if (y < 3)
                //     v.id = BLOCK_ROCK;
                // if (y > 2 && y < 4)
                //     v.id = BLOCK_GRASS;

                c->blocks[z * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH + y * CHUNK_EDGE_LENGTH + x] = v;
            }
        }
    }
}

void chunk_allocate(chunk *c)
{
    c->mesh = (int *)malloc(CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * sizeof(block_vertices));
    verify(c->mesh, "couldn't allocate a chunk of memory", __LINE__);
}
void chunk_free(chunk *c)
{
    free(c->mesh);
    verify(!c->mesh, "failed to free chunk mesh data", __LINE__);
}

void build_chunk_mesh(chunk *c, chunk *left, chunk *right, chunk *forwards, chunk *backwards, chunk *up, chunk *down)
{
    verify(c->mesh, "cannot build chunk mesh, mesh pointer is NULL. Maybe it's not been allocated?", __LINE__);
    memset(c->mesh, 0, block_limit * sizeof(block_vertices));

    const int chunk_total = CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
    const int chunk_slab = CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
    const int chunk_row = CHUNK_EDGE_LENGTH;
    int mesh_size = 0;
    for (int block_index = 0; block_index < block_limit; ++block_index)
    {
        if (c->blocks[block_index].id == BLOCK_NULL)
        {
            continue;
        }
        int x = block_index % CHUNK_EDGE_LENGTH;
        int y = (block_index / CHUNK_EDGE_LENGTH) % CHUNK_EDGE_LENGTH;
        int z = (block_index / (CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH));
        int v_count = sizeof(block_vertices) / sizeof(block_vertices[0]);
        int cube_tri_count = 36;
        int vertex_data_size = v_count / cube_tri_count;

        int face_vertex_count = vertex_data_size * 6;
        int offset_cube[v_count];

        int block_mesh_size = 0;
        int normal_index = 0;

        vec3 vertex_offset = {(double)x, (double)y, (double)z};
        for (int face_index = 0; face_index < v_count; face_index += face_vertex_count)
        {
            normal_index = face_index / face_vertex_count;
            bool skip_face = false;
            switch (normal_index)
            {
            case 0:
                if (z < CHUNK_EDGE_LENGTH - 1)
                {
                    if (c->blocks[block_index + chunk_slab].id != BLOCK_NULL)
                        skip_face = true;
                }
                else
                {
                    // printf("block = %i\n", block_index);
                    // block_id other_chunk_block_id = forwards->blocks[block_index].id;
                    block_id other_chunk_block_id = BLOCK_NULL;
                    printf("-id = %i\n", other_chunk_block_id);
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
                else
                {
                    // block_id other_chunk_block_id = backwards->blocks[block_index + chunk_total].id;
                    block_id other_chunk_block_id = BLOCK_NULL;
                    // printf("id = %i\n", other_chunk_block_id);
                    if (other_chunk_block_id != BLOCK_NULL)
                        skip_face = true;
                }
                break;
            case 2:
                if (x < CHUNK_EDGE_LENGTH - 1)
                {
                    if (c->blocks[block_index + 1].id != BLOCK_NULL)
                        skip_face = true;
                }
                break;
            case 3:
                if (x > 0)
                {
                    if (c->blocks[block_index - 1].id != BLOCK_NULL)
                        skip_face = true;
                }
                break;
            case 4:
                if (y < CHUNK_EDGE_LENGTH - 1)
                {
                    if (c->blocks[block_index + chunk_row].id != BLOCK_NULL)
                        skip_face = true;
                }
                break;
            case 5:
                if (y > 0)
                {
                    if (c->blocks[block_index - chunk_row].id != BLOCK_NULL)
                        skip_face = true;
                }
                break;
            default:
                skip_face = true;
                break;
            }

            if (skip_face)
                continue;

            for (int vertex_index = 0; vertex_index < face_vertex_count; vertex_index += vertex_data_size)
            {
                // position
                offset_cube[block_mesh_size + vertex_index + 0] = block_vertices[face_index + vertex_index + 0] + vertex_offset[0];
                offset_cube[block_mesh_size + vertex_index + 1] = block_vertices[face_index + vertex_index + 1] + vertex_offset[1];
                offset_cube[block_mesh_size + vertex_index + 2] = block_vertices[face_index + vertex_index + 2] + vertex_offset[2];
                // texture

                int t_x = c->blocks[block_index].id % c->texture_width;
                int t_y = c->blocks[block_index].id / c->texture_width;
                t_x += 1;
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

    glBufferData(GL_ARRAY_BUFFER, sizeof(int) * 5 * c->mesh_size, c->mesh, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_INT, GL_FALSE, 5 * sizeof(GL_INT), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_INT, GL_FALSE, 5 * sizeof(GL_INT), (void *)(3 * sizeof(GL_INT)));
    glEnableVertexAttribArray(1);
}
void update_chunk(chunk *c, camera *cam, int *lookat_block, int *lookat_block_normal, double *lookat_block_distance, int *lookat_chunk_normal)
{
    if (c->dirty)
    {
        // reimplement
        // build_chunk_mesh(c, );
        c->dirty = false;
    }
    double closest_block_dist = DBL_MAX;
    int closest_block_normal = -1;
    for (int i = 0; i < block_limit; ++i)
    {
        if (c->blocks[i].id == BLOCK_NULL)
        {
            continue;
        }
        vec3 block_pos = {i % CHUNK_EDGE_LENGTH,
                          (i / CHUNK_EDGE_LENGTH) % CHUNK_EDGE_LENGTH,
                          i / (CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH)};
        vec3 chunk_pos = {c->x * CHUNK_EDGE_LENGTH,
                          c->y * CHUNK_EDGE_LENGTH,
                          c->z * CHUNK_EDGE_LENGTH};
        vec3 real_block_position = {};
        glm_vec3_add(chunk_pos, block_pos, real_block_position);
        vec3 block_min = {}, block_max = {};
        glm_vec3_copy(real_block_position, block_min);
        glm_vec3_add(real_block_position, (vec3){1.0, 1.0, 1.0}, block_max);
        vec3 cam_com_dir = {
            -cam->direction[0],
            -cam->direction[1],
            -cam->direction[2]};
        if (!ray_voxel_colliding(cam->pos, cam_com_dir, block_min, block_max, &closest_block_normal))
        {
            continue;
        }
        float dist = glm_vec3_distance2(cam->pos, real_block_position);
        if (dist < closest_block_dist)
        {
            closest_block_dist = dist;
            if (closest_block_dist < *lookat_block_distance)
            {
                *lookat_block = i;
                *lookat_block_normal = closest_block_normal;
                *lookat_block_distance = closest_block_dist;
                // printf("looking at block %f %f %f, block normal = %i\n", block_pos[0], block_pos[1], block_pos[2], *lookat_block_normal);
                *lookat_chunk_normal = -1;
                // if (*lookat_block_normal == 1 && block_pos[0] >= CHUNK_EDGE_LENGTH - 1)
                //     *lookat_chunk_normal = 1;
                // if (*lookat_block_normal == 0 && block_pos[0] <= 0)
                //     *lookat_chunk_normal = 0;
                // if (block_pos[1] >= CHUNK_EDGE_LENGTH - 1)
                //     *lookat_chunk_normal = 2;
                // if (block_pos[1] <= 0)
                //     *lookat_chunk_normal = 3;
                // if (*lookat_block_normal == 5 && block_pos[2] >= CHUNK_EDGE_LENGTH - 1)
                //     *lookat_chunk_normal = 5;
                // if (*lookat_block_normal == 4 && block_pos[2] <= 0)
                //     *lookat_chunk_normal = 4;

                if (*lookat_block_normal == 1 && block_pos[0] >= CHUNK_EDGE_LENGTH - 1)
                    *lookat_chunk_normal = 1;
                if (*lookat_block_normal == 0 && block_pos[0] <= 0)
                    *lookat_chunk_normal = 0;
                if (*lookat_block_normal == 3 && block_pos[1] >= CHUNK_EDGE_LENGTH - 1)
                    *lookat_chunk_normal = 3;
                if (*lookat_block_normal == 2 && block_pos[1] <= 0)
                    *lookat_chunk_normal = 2;
                if (*lookat_block_normal == 5 && block_pos[2] >= CHUNK_EDGE_LENGTH - 1)
                    *lookat_chunk_normal = 5;
                if (*lookat_block_normal == 4 && block_pos[2] <= 0)
                    *lookat_chunk_normal = 4;
            }
        }
    }
}
void draw_chunk(chunk *c, shader_list *shaders, int lookat_block)
{
    mat4 model;
    glm_mat4_identity(model);
    // transform model based on chunk position
    glm_translate(model, (vec3){c->x * CHUNK_EDGE_LENGTH, c->y * CHUNK_EDGE_LENGTH, c->z * CHUNK_EDGE_LENGTH});
    shader_set_mat4x4(shaders, SHADER_COMMON, "model", model);
    shader_set_int(shaders, SHADER_COMMON, "tex", 1);
    shader_set_vec2(shaders, SHADER_COMMON, "texture_size", (vec2){2.0, 1.0});

    glBindVertexArray(c->vao);
    glDrawArrays(GL_TRIANGLES, 0, c->mesh_size);
}
void break_chunk_block(chunk *c, int lookat_block)
{
    if (lookat_block < 0)
        return;
    c->blocks[lookat_block].id = BLOCK_NULL;
    c->dirty = true;
}

void place_chunk_block(chunk *c, int origin_block, int origin_normal)
{
    int chunk_size = CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
    int slab_size = CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
    int row_size = CHUNK_EDGE_LENGTH;

    int new_block_pos = -1;
    int x = origin_block % CHUNK_EDGE_LENGTH;
    int y = (origin_block / CHUNK_EDGE_LENGTH) % CHUNK_EDGE_LENGTH;
    int z = origin_block / (CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH);
    switch (origin_normal)
    {
    case 5:
        // pos z, get block that's a slab past this block
        new_block_pos = origin_block + slab_size;
        ++z;
        break;
    case 4:
        // neg z
        new_block_pos = origin_block - slab_size;
        --z;
        break;
    case 3:
        // pos y
        new_block_pos = origin_block + row_size;
        ++y;
        break;
    case 2:
        // neg y
        new_block_pos = origin_block - row_size;
        --y;
        break;
    case 1:
        // pos x
        new_block_pos = origin_block + 1;
        ++x;
        break;
    case 0:
        // neg x
        new_block_pos = origin_block - 1;
        --x;
        break;
    default:
        break;
    }

    if (new_block_pos > -1 && new_block_pos < chunk_size)
    {
        c->blocks[new_block_pos].id = BLOCK_ROCK;
        c->dirty = true;
    }
}