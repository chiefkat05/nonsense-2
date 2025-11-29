#include "definitions.h"
#include <cglm/cglm.h>
#include "chiefkat_opengl.h"
#include "models.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
static bool global_resize_alert;

void framebuffer_size(GLFWwindow *window, int width, int height);
void process_input(GLFWwindow *window, camera *cam, double delta_time);

int main()
{
    verify(glfwInit(), "glfw failure", __LINE__);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "voxel sandbox world application", NULL, NULL);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    glfwMakeContextCurrent(window);
    verify(glewInit() == 0, "glew failure", __LINE__);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glfwSetFramebufferSizeCallback(window, framebuffer_size);

    shader_list shaders = {};
    model_list models = {};

    create_shader(&shaders, SHADER_COMMON, "./vertex.shader", "./fragment.shader");

    create_model_texture_normals(&models, MODEL_CUBE, cube_vertices, 36);
    int grass_tex = texture_load_from_bmp(1, "./grass.bmp");
    int rock_tex = texture_load_from_bmp(2, "./rock.bmp");
    mat4 model, proj;
    glm_mat4_identity(proj);

    camera cam = {};
    camera_setup(&cam, (vec3){0.0, 0.0, -3.0}, (vec3){0.0, 0.0, 0.0});

    glm_perspective(45, (double)WINDOW_HEIGHT / (double)WINDOW_WIDTH, 0.1, 100.0, proj);
    double mouseX = 0, mouseY = 0;
    double lastMouseX = mouseY, lastMouseY = mouseY;

    double delta_time = 0.0;
    double last_time = 0.0;
    double current_time = 0.0;
    while (!glfwWindowShouldClose(window))
    {
        if (global_resize_alert)
        {
            glm_perspective(45, (double)WINDOW_HEIGHT / (double)WINDOW_WIDTH, 0.1, 100.0, proj);
        }
        last_time = current_time;
        current_time = glfwGetTime();
        delta_time = current_time - last_time;
        lastMouseX = mouseX;
        lastMouseY = mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        process_input(window, &cam, delta_time);
        camera_update(&cam, mouseX - lastMouseX, mouseY - lastMouseY, delta_time);

        glm_mat4_identity(model);
        vec3 pos = {0.0, -4.0, 4.0};
        versor rot;
        glm_quat_identity(rot);
        vec3 scale = {1.0, 1.0, 1.0};
        glm_quat_rotate(model, rot, model);
        glm_translate(model, pos);
        glm_scale(model, scale);

        shader_set_mat4x4(&shaders, SHADER_COMMON, "model", model);
        shader_set_mat4x4(&shaders, SHADER_COMMON, "view", cam.view);
        shader_set_mat4x4(&shaders, SHADER_COMMON, "proj", proj);
        shader_set_vec3(&shaders, SHADER_COMMON, "texture_scale", (vec3){1.0, 1.0, 1.0});
        shader_set_vec2(&shaders, SHADER_COMMON, "texture_pixel_scale", (vec2){1.0, 1.0});

        use_shader(&shaders, SHADER_COMMON);
        shader_set_int(&shaders, SHADER_COMMON, "tex", rock_tex);
        draw_model(&models, MODEL_CUBE);
        glm_mat4_identity(model);
        glm_translate(model, (vec3){-2.0, -4.0, 4.0});
        shader_set_mat4x4(&shaders, SHADER_COMMON, "model", model);
        shader_set_int(&shaders, SHADER_COMMON, "tex", grass_tex);
        draw_model(&models, MODEL_CUBE);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

void framebuffer_size(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    global_resize_alert = true;
}

void process_input(GLFWwindow *window, camera *cam, double delta_time)
{
    vec3 movement = {};
    double speed = 4.0;

    if (glfwGetKey(window, GLFW_KEY_G))
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        cam->mouse_focused = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F))
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        cam->mouse_focused = false;
    }
    if (glfwGetKey(window, GLFW_KEY_W))
    {
        glm_vec3_sub(movement, cam->xz_direction, movement);
    }
    if (glfwGetKey(window, GLFW_KEY_S))
    {
        glm_vec3_add(movement, cam->xz_direction, movement);
    }
    if (glfwGetKey(window, GLFW_KEY_A))
    {
        glm_vec3_add(movement, cam->right, movement);
    }
    if (glfwGetKey(window, GLFW_KEY_D))
    {
        glm_vec3_sub(movement, cam->right, movement);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
    {
        glm_vec3_sub(movement, cam->up, movement);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE))
    {
        glm_vec3_add(movement, cam->up, movement);
    }

    glm_normalize(movement);
    glm_vec3_scale(movement, speed, movement);
    glm_vec3_scale(movement, delta_time, movement);
    glm_vec3_add(cam->pos, movement, cam->pos);
}