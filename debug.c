#include "debug.h"

void make_cube(cube_element *cube, int x, int y, int z, int w, int h, int d)
{
    glGenVertexArrays(1, &cube->vao);
    glGenBuffers(1, &cube->vbo);

    glBindVertexArray(cube->vao);
    glBindBuffer(GL_ARRAY_BUFFER, cube->vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(block_vertices), block_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_INT, GL_FALSE, 5 * sizeof(GL_INT), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_INT, GL_FALSE, 5 * sizeof(GL_INT), (void *)(3 * sizeof(GL_INT)));
    glEnableVertexAttribArray(1);

    cube->x = x;
    cube->y = y;
    cube->z = z;
    cube->w = w;
    cube->h = h;
    cube->d = d;
}
void draw_cube(cube_element *cube, shader_list *shaders, unsigned int t, unsigned int tw, unsigned int th)
{
    mat4 model;
    glm_mat4_identity(model);
    glActiveTexture(GL_TEXTURE0 + t);
    // transform model based on chunk position
    glm_translate(model, (vec3){cube->x, cube->y, cube->z});
    glm_scale(model, (vec3){cube->w, cube->h, cube->d});
    shader_set_mat4x4(shaders, SHADER_COMMON, "model", model);
    shader_set_int(shaders, SHADER_COMMON, "tex", t);
    shader_set_vec2(shaders, SHADER_COMMON, "texture_size", (vec2){tw, th});

    glBindVertexArray(cube->vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}
void move_cube(cube_element *cube, int x, int y, int z)
{
    cube->x = x;
    cube->y = y;
    cube->z = z;
}