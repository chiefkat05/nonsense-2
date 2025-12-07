#include "chunk.h"

int chunk_dda_test(chunk *c, vec3 pos, vec3 dir, int *b_norm, int *ch_norm)
{
    int chunk_x = c->x * CHUNK_EDGE;
    int chunk_y = c->y * CHUNK_EDGE;
    int chunk_z = c->z * CHUNK_EDGE;

    int x = floor(pos[0]) - chunk_x;
    int y = floor(pos[1]) - chunk_y;
    int z = floor(pos[2]) - chunk_z;

    int x_step = (fabs(dir[0])/dir[0]);
    int y_step = (fabs(dir[1])/dir[1]);
    int z_step = (fabs(dir[2])/dir[2]);


    double maxx = 0.0;
    double maxy = 0.0;
    double maxz = 0.0;
    
    double delta_x = x_step / dir[0];
    double delta_y = y_step / dir[1];
    double delta_z = z_step / dir[2];

    if (x_step > 0)
    {
        maxx = ((floor(pos[0]) + 1.0) - pos[0]) * delta_x;
    }
    if (x_step < 0)
    {
        maxx = (pos[0] - floor(pos[0])) * delta_x;
    }
    if (y_step > 0)
    {
        maxy = ((floor(pos[1]) + 1.0) - pos[1]) * delta_y;
    }
    if (y_step < 0)
    {
        maxy = (pos[1] - floor(pos[1])) * delta_y;
    }
    if (z_step > 0)
    {
        maxz = ((floor(pos[2]) + 1.0) - pos[2]) * delta_z;
    }
    if (z_step < 0)
    {
        maxz = (pos[2] - floor(pos[2])) * delta_z;
    }

    int block_norm = -1;
    int block = -1;

    block_norm = -1;
    while ((block == -1 || c->blocks[block].id == BLOCK_NULL))
    {
        // block = chunk_get_block_from_position(x, y, z);
        if (maxx < maxy && maxx < maxz)
        {
            x += x_step;
            if ((x_step < 0 && x < 0) || (x_step > 0 && x >= CHUNK_EDGE))
                return -1;

            maxx += delta_x;
            block_norm = x_step > 0 ? 0 : 1;
            block = chunk_get_block_from_position(x, y, z);
            continue;
        }
        if (maxy < maxx && maxy < maxz)
        {
            y += y_step;
            if ((y_step < 0 && y < 0) || (y_step > 0 && y >= CHUNK_EDGE))
                return -1;

            maxy += delta_y;
            block_norm = y_step > 0 ? 2 : 3;
            block = chunk_get_block_from_position(x, y, z);
            continue;
        }
        if (maxz < maxx && maxz < maxy)
        {
            z += z_step;
            if ((z_step < 0 && z < 0) || (z_step > 0 && z >= CHUNK_EDGE))
                return -1;

            maxz += delta_z;
            block_norm = z_step > 0 ? 4 : 5;
            block = chunk_get_block_from_position(x, y, z);
            continue;
        }
        return -1;
    }
    int chunk_norm = -1;
    if (block_norm == 0 && x == 0)
    {
        chunk_norm = 0;
    }
    if (block_norm == 1 && x == CHUNK_EDGE - 1)
    {
        chunk_norm = 1;
    }
    if (block_norm == 2 && y == 0)
    {
        chunk_norm = 2;
    }
    if (block_norm == 3 && y == CHUNK_EDGE - 1)
    {
        chunk_norm = 3;
    }
    if (block_norm == 4 && z == 0)
    {
        chunk_norm = 4;
    }
    if (block_norm == 5 && z == CHUNK_EDGE - 1)
    {
        chunk_norm = 5;
    }
    *ch_norm = chunk_norm;
    *b_norm = block_norm;

    // printf("see block %i %i %i at chunk %i %i %i\n", x, y, z, chunk_x, chunk_y, chunk_z);
    return block;
}

int chunk_get_block_from_position(int x, int y, int z)
{
    if (x < 0 || x >= CHUNK_EDGE || y < 0 || y >= CHUNK_EDGE || z < 0 || z >= CHUNK_EDGE)
    {
        return -1;
    }
    return z * CHUNK_SLAB + y * CHUNK_EDGE + x;
}
bool chunk_get_position_from_block(int i, vec3 output, int line)
{
    if (i < 0 || i >= CHUNK_TOTAL)
    {
        return false;
    }

    int z = i / (CHUNK_SLAB);
    int y = (i - (z * CHUNK_SLAB)) / CHUNK_EDGE;
    int x = i - (z * CHUNK_SLAB) - (y * CHUNK_EDGE);
    glm_vec3_copy((vec3){x, y, z}, output);

    return true;
}

double fract(double d)
{
    return d - (int)d;
}
double vrandom(vec2 v)
{
    return fract(sin(glm_vec2_dot(v, (vec2){12.9898,54.233})) * 43758.5453123);
}

double noise_value(vec2 v)
{
    vec2 i = {};
    glm_vec2_floor(v, i);
    vec2 f = {};
    glm_vec2_fract(v, f);
    
    vec2 i2 = {};
    glm_vec2_add(i, (vec2){1.0, 0.0}, i2);
    vec2 i3 = {};
    glm_vec2_add(i, (vec2){0.0, 1.0}, i3);
    vec2 i4 = {};
    glm_vec2_add(i, (vec2){1.0, 1.0}, i4);

    double tl = vrandom(i);
    double tr = vrandom(i2);
    double bl = vrandom(i3);
    double br = vrandom(i4);

    glm_smoothstep(0.0, 1.0, f[0]);
    glm_smoothstep(0.0, 1.0, f[2]);

    return glm_lerp(glm_lerp(tl, tr, f[0]), glm_lerp(bl, br, f[0]), f[1]);
}

double noise_sine(vec2 v)
{
    return sin(v[0] * 4.0) + sin(v[1] * 4.0);
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
        for (int y = CHUNK_EDGE - 1; y > -1; --y)
        {
            for (int x = 0; x < CHUNK_EDGE; ++x)
            {
                landscape[x][y][z] = BLOCK_NULL;

                if (c->y > -1)
                    continue;

                double x_input = (double)x / (double)CHUNK_EDGE;
                double z_input = (double)z / (double)CHUNK_EDGE;
                double ny = noise_sine((vec2){x_input, z_input}) * (double)6.0;
                
                if (y < ny)
                    landscape[x][y][z] = BLOCK_GRASS;
                if (y < 2)
                    landscape[x][y][z] = BLOCK_ROCK;
                if (y + 1 < CHUNK_EDGE && landscape[x][y + 1][z] != BLOCK_NULL)
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

// this should be working whyyyyy
// void chunk_build_partial_mesh(chunk *c)
// {
//     for (int i = 0; i < CHUNK_TOTAL; ++i)
//     {
//         if (!c->blocks[i].dirty)
//             continue;
        
//         int *mesh = c->mesh;
//         mesh += c->blocks[i].mesh_location;
//         int *mesh_top_half = mesh + c->blocks[i].size;
//         // if (c->blocks[i].id == BLOCK_NULL)
//         // {
//         memcpy(mesh, mesh_top_half, c->mesh_size - c->blocks[i].size);
//         c->mesh_size -= c->blocks[i].size;
//         // }

//         printf("removed block of size %i, mesh size now %i.\n", c->blocks[i].size, c->mesh_size);

//         c->blocks[i].dirty = false;
//     }
// }
void chunk_build_mesh(chunk *c, chunk *left, chunk *right, chunk *forwards, chunk *backwards, chunk *up, chunk *down)
{
    verify(c, "cannot build chunk mesh, chunk does not exist.", __LINE__);
    verify(c->mesh, "cannot build chunk mesh, mesh pointer is NULL. Maybe it's not been allocated?", __LINE__);
    // memset(c->mesh, 0, CHUNK_TOTAL * sizeof(block_vertices));

    const int chunk_slab = CHUNK_EDGE * CHUNK_EDGE;
    const int chunk_row = CHUNK_EDGE;
    int mesh_size = 0;
    int v_count = sizeof(block_vertices) / sizeof(block_vertices[0]);
    int cube_tri_count = 36;
    int vertex_data_size = v_count / cube_tri_count;

    int face_vertex_count = vertex_data_size * 6;
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
            
            int t_x = texture_locs[c->blocks[block_index].id][0];
            int t_y = texture_locs[c->blocks[block_index].id][1];

            switch(c->blocks[block_index].id)
            {
                case BLOCK_GRASS:
                    if ((rand() % 90) < 1)
                    {
                        t_x = 1;
                        t_y = 0;
                    }
                    if ((rand() % 20) < 1)
                    {
                        t_x = 0;
                        t_y = 0;
                    }
                    break;
                default:
                    break;
            }
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
        chunk_build_mesh(c, left_chunk, right_chunk, forwards_chunk, backwards_chunk, up_chunk, down_chunk);
        c->dirty = false;
    }
    // if (c->partial_dirty)
    // {
    //     chunk_build_partial_mesh(c);
    //     c->partial_dirty = false;
    // }

    int block = chunk_dda_test(c, cam->real_position, cam->inv_look_direction, lookat_block_normal, lookat_chunk_normal);

    vec3 block_position = {};
    if (!chunk_get_position_from_block(block, block_position, __LINE__))
    {
        return;
    }

    vec3 global_block_position = {c->x * CHUNK_EDGE + block_position[0],
                                c->y * CHUNK_EDGE + block_position[1],
                                c->z * CHUNK_EDGE + block_position[2]};

    double block_dist = glm_vec3_distance2(global_block_position, cam->position);
    if (block_dist < *lookat_block_distance)
    {
        // printf("d'uh, block dist was %f before but %f is better so we're going with that. Okay boss?\n", *lookat_block_distance, block_dist);
        *lookat_block_distance = block_dist;
        *lookat_block = block;
    }
}
void draw_chunk(chunk *c, shader_list *shaders)
{
    mat4 model;
    glm_mat4_identity(model);
    // transform model based on chunk position
    glm_translate(model, (vec3){c->x * CHUNK_EDGE, c->y * CHUNK_EDGE, c->z * CHUNK_EDGE});
    shader_set_mat4x4(shaders, SHADER_COMMON, "model", model);
    shader_set_int(shaders, SHADER_COMMON, "tex", 1);
    glActiveTexture(GL_TEXTURE0 + 1);
    shader_set_vec2(shaders, SHADER_COMMON, "texture_size", (vec2){c->texture_width, c->texture_height});

    glBindVertexArray(c->vao);
    glDrawArrays(GL_TRIANGLES, 0, c->mesh_size);
}
