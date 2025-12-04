
#ifndef CHIEFKAT_OPENGL_H
#define CHIEFKAT_OPENGL_H

#include "definitions.h"
#include "chiefkat_bmp.h"

#define SHADER_LIMIT 4
typedef enum
{
    SHADER_COMMON,
    SHADER_COLORED,
    SHADER_INSTANCED,
    SHADER_LIGHTING
} shader_name;
typedef struct
{
    unsigned int id_list[SHADER_LIMIT];
    bool shader_exists_list[SHADER_LIMIT];
} shader_list;

void create_shader(shader_list *list, shader_name name, const char *vs_path, const char *fs_path);

void use_shader(shader_list *list, shader_name name);

void shader_set_float(shader_list *list, shader_name name, const char *uniform_name, double value);
void shader_set_int(shader_list *list, shader_name name, const char *uniform_name, int value);
void shader_set_bool(shader_list *list, shader_name name, const char *uniform_name, bool value);
void shader_set_vec2(shader_list *list, shader_name name, const char *uniform_name, vec2 value);
void shader_set_vec3(shader_list *list, shader_name name, const char *uniform_name, vec3 value);
void shader_set_vec4(shader_list *list, shader_name name, const char *uniform_name, vec4 value);
void shader_set_mat4x4(shader_list *list, shader_name name, const char *uniform_name, mat4 value);

typedef struct
{
    vec3 real_position;
    versor real_pitch_rotation, real_yaw_rotation;

    versor pitch_rotation, last_pitch_rotation, yaw_rotation, last_yaw_rotation;
    vec3 position, last_position, look_direction, walk_direction, up, right;
    mat4 view;
    double yaw, pitch;
    bool mouse_focused;
} camera;

void camera_setup(camera *cam, vec3 pos, vec3 target);
void camera_update(camera *cam, double yaw_update, double pitch_update, double delta_time);

int texture_load_from_bmp(int index, const char *path, unsigned int *width, unsigned int *height);

#endif