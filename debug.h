#ifndef DEBUG_H
#define DEBUG_H

#include "definitions.h"
#include "block.h"
#include "shader.h"

typedef struct
{
    unsigned vao, vbo;
    int x, y, z, w, h, d;
} cube_element;

void make_cube(cube_element *cube, int x, int y, int z, int w, int h, int d);
void draw_cube(cube_element *cube, shader_list *shaders, unsigned int t, unsigned int tw, unsigned int th);
void move_cube(cube_element *cube, int x, int y, int z);

#endif