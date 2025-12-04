#include "shader.h"

void create_shader(shader_list *list, shader_name name, const char *vs_path, const char *fs_path)
{
    char vs_source[BUFSIZ];
    FILE *vertex_file = fopen(vs_path, "rb");
    fseek(vertex_file, 0, SEEK_END);
    unsigned int vertex_file_length = ftell(vertex_file);
    fseek(vertex_file, 0, SEEK_SET);

    fread(vs_source, sizeof(char), vertex_file_length, vertex_file);
    fclose(vertex_file);
    if (vs_source[vertex_file_length - 1] != '\0')
    {
        vs_source[vertex_file_length++] = '\0';
    }

    const char *vertex_source = vs_source;

    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_source, NULL);
    glCompileShader(vertex_shader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
        verify(false, infoLog, __LINE__);
    }

    char fs_source[BUFSIZ];
    FILE *fragment_file = fopen(fs_path, "rb");
    fseek(fragment_file, 0, SEEK_END);
    unsigned int fragment_file_length = ftell(fragment_file);
    fseek(fragment_file, 0, SEEK_SET);

    fread(fs_source, 1, fragment_file_length, fragment_file);
    fclose(fragment_file);
    if (fs_source[fragment_file_length - 1] != '\0')
    {
        fs_source[fragment_file_length++] = '\0';
    }
    const char *fragment_source = fs_source;

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        verify(false, infoLog, __LINE__);
    }

    list->id_list[name] = glCreateProgram();
    glAttachShader(list->id_list[name], vertex_shader);
    glAttachShader(list->id_list[name], fragment_shader);
    glLinkProgram(list->id_list[name]);

    glGetProgramiv(list->id_list[name], GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(list->id_list[name], 512, NULL, infoLog);
        verify(false, infoLog, __LINE__);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    list->shader_exists_list[name] = true;
}

void use_shader(shader_list *list, shader_name name)
{
    if (!list->shader_exists_list[name])
    {
        Logerror("shader being used without initialization!", name);
    }
    glUseProgram(list->id_list[name]);
}
void shader_set_float(shader_list *list, shader_name name, const char *uniform_name, double value)
{
    use_shader(list, name);
    int uniform_location = glGetUniformLocation(list->id_list[name], uniform_name);
    glUniform1f(uniform_location, value);
}
void shader_set_int(shader_list *list, shader_name name, const char *uniform_name, int value)
{
    use_shader(list, name);
    int uniform_location = glGetUniformLocation(list->id_list[name], uniform_name);
    glUniform1i(uniform_location, value);
}
void shader_set_bool(shader_list *list, shader_name name, const char *uniform_name, bool value)
{
    use_shader(list, name);
    int uniform_location = glGetUniformLocation(list->id_list[name], uniform_name);
    glUniform1i(uniform_location, value);
}
void shader_set_vec2(shader_list *list, shader_name name, const char *uniform_name, vec2 value)
{
    use_shader(list, name);
    int uniform_location = glGetUniformLocation(list->id_list[name], uniform_name);
    glUniform2f(uniform_location, value[0], value[1]);
}
void shader_set_vec3(shader_list *list, shader_name name, const char *uniform_name, vec3 value)
{
    use_shader(list, name);
    int uniform_location = glGetUniformLocation(list->id_list[name], uniform_name);
    glUniform3f(uniform_location, value[0], value[1], value[2]);
}
void shader_set_vec4(shader_list *list, shader_name name, const char *uniform_name, vec4 value)
{
    use_shader(list, name);
    int uniform_location = glGetUniformLocation(list->id_list[name], uniform_name);
    glUniform4f(uniform_location, value[0], value[1], value[2], value[3]);
}
void shader_set_mat4x4(shader_list *list, shader_name name, const char *uniform_name, mat4 value)
{
    use_shader(list, name);
    int uniform_location = glGetUniformLocation(list->id_list[name], uniform_name);
    glUniformMatrix4fv(uniform_location, 1, false, &value[0][0]);
}

void camera_setup(camera *cam, vec3 pos, vec3 target)
{
    glm_vec3_copy((vec3){0.0, 1.0, 0.0}, cam->up);
    glm_vec3_copy(pos, cam->position);
    // glm_vec3_copy(target, cam->target);
    // glm_vec3_copy((vec3){0.0, 0.0, -1.0}, cam->direction);

    cam->yaw = -90.0;
    // cam->direction[0] = cos(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
    // cam->direction[1] = sin(glm_rad(cam->pitch));
    // cam->direction[2] = sin(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));

    // glm_cross(cam->direction, cam->up, cam->right);

    // glm_lookat(cam->position, cam->target, cam->up, cam->view);
}
void camera_update(camera *cam, double yaw_update, double pitch_update, double tick_speed)
{
    double mouse_sensitivity = 10.0;
    if (cam->mouse_focused)
    {
        cam->yaw += yaw_update * tick_speed * mouse_sensitivity;
        cam->pitch += pitch_update * tick_speed * mouse_sensitivity;
    }
    if (cam->pitch > 89.0)
    {
        cam->pitch = 89.0;
    }
    if (cam->pitch < -89.0)
    {
        cam->pitch = -89.0;
    }
    
    glm_quat(cam->yaw_rotation, glm_rad(cam->yaw), 0.0, 1.0, 0.0);
    glm_quat(cam->pitch_rotation, glm_rad(cam->pitch), 1.0, 0.0, 0.0);
}

int texture_load_from_bmp(int index, const char *path, unsigned int *width, unsigned int *height)
{
    bmp_data image = read_bmp(path);
    unsigned int texture = index;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    glGenerateMipmap(GL_TEXTURE_2D);

    *width = image.width;
    *height = image.height;

    free_image(&image);

    return texture;
}
