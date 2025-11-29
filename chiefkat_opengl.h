
#ifndef CHIEFKAT_OPENGL_H
#define CHIEFKAT_OPENGL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "definitions.h"
// so help me when I fix my math library this here include is getting the boot
#include <cglm/cglm.h>
#include "chiefkat_bmp.h"

#define SHADER_LIMIT 400
typedef enum
{
    SHADER_COMMON,
    SHADER_COLORED,
    SHADER_,
    SHADER_3
} shader_name;
typedef struct
{
    unsigned int id_list[SHADER_LIMIT];
    bool shader_exists_list[SHADER_LIMIT];
} shader_list;

#define MODEL_LIMIT 3
typedef enum
{
    MODEL_TRI,
    MODEL_QUAD,
    MODEL_CUBE,
} model_name;
typedef struct
{
    unsigned int vertex_count[MODEL_LIMIT];

    unsigned int VBO_list[MODEL_LIMIT];
    unsigned int VAO_list[MODEL_LIMIT];
} model_list;

void create_shader(shader_list *list, shader_name name, const char *vs_path, const char *fs_path);
void create_model(model_list *list, model_name name, int *vertices, int vertex_count);
void create_model_texture(model_list *list, model_name name, int *vertices, int vertex_count);
void create_model_texture_normals(model_list *list, model_name name, int *vertices, int vertex_count);

void use_shader(shader_list *list, shader_name name);
void draw_model(model_list *list, model_name name);

void shader_set_float(shader_list *list, shader_name name, const char *uniform_name, double value);
void shader_set_int(shader_list *list, shader_name name, const char *uniform_name, int value);
void shader_set_vec2(shader_list *list, shader_name name, const char *uniform_name, vec2 value);
void shader_set_vec3(shader_list *list, shader_name name, const char *uniform_name, vec3 value);
void shader_set_vec4(shader_list *list, shader_name name, const char *uniform_name, vec4 value);
void shader_set_mat4x4(shader_list *list, shader_name name, const char *uniform_name, mat4 value);

typedef struct
{
    vec3 pos, target, direction, xz_direction, up, right;
    mat4 view;
    double yaw, pitch;
    bool mouse_focused;
} camera;

void camera_setup(camera *cam, vec3 pos, vec3 target);
void camera_update(camera *cam, double yaw_update, double pitch_update, double delta_time);

int texture_load_from_bmp(int index, const char *path);

#endif