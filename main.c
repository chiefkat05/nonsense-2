#include "shader.h"
#include "world.h"
#include "definitions.h"
#include <png.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
static bool global_resize_alert;
static int global_window_width;
static int global_window_height;

unsigned int world_local_edge_size = 10;
double render_distance = 5000.0;
float *camera_position;

void framebuffer_size(GLFWwindow *window, int width, int height);
void process_input(GLFWwindow *window, camera *cam, double delta_time);

void load_png_texture(const char *path, unsigned int opengl_texture_index)
{
}

const double tick_speed = 1.0 / 200.0;

// today's tasks:
// png loading with 1 simple texture on screen
// dda or voxel-raycast for the looking-at-blocks algorithm

// if you have time (but you can just take a break if you want):
// simple noise world generation

// local-only hashmap is still maybe on the table to avoid heavy memory usage when travelling to outer blocks
// collision detection + gravity
// multithreading for chunk gen
// animate the texture with the most barebones timer countdown and frame increment

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

    mat4 proj;
    glm_mat4_identity(proj);

    world game_world = {};
    camera_setup(&game_world.cam, (vec3){0.0, 0.0, -3.0}, (vec3){0.0, 0.0, 0.0});
    glm_perspective(45, (double)WINDOW_WIDTH / (double)WINDOW_HEIGHT, 0.1, 100.0, proj);

    create_shader(&game_world.shaders, SHADER_COMMON, "./vertex.shader", "./fragment.shader");

    chunk_map_alloc(&game_world.chunk_map, world_local_edge_size * world_local_edge_size * world_local_edge_size);
    game_world.texture_id = texture_load_from_bmp(1, "./atlas.bmp", &game_world.texture_width, &game_world.texture_height);
    game_world.texture_width /= 16;
    game_world.texture_height /= 16;

    double mouseX = 0, mouseY = 0;
    double lastMouseX = mouseY, lastMouseY = mouseY;
    bool left_held = false, right_held = false;

    double frame_time = 0.0;
    double previous_time = 0.0;
    double current_time = 0.0;
    double accumulated_time = 0.0;
    while (!glfwWindowShouldClose(window))
    {
        previous_time = current_time;
        current_time = glfwGetTime();
        frame_time = current_time - previous_time;
        accumulated_time += frame_time;

        if (global_resize_alert)
        {
            glm_perspective(45, (double)global_window_width / (double)global_window_height, 0.1, 100.0, proj);
            global_resize_alert = false;
        }
        lastMouseX = mouseX;
        lastMouseY = mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        glClearColor(0.3, 0.7, 0.9, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // this should be a function that sets input states that can then be referenced in the update loop
        // process_input(window, &game_world.cam, tick_speed);

        // update loop
        while (accumulated_time >= tick_speed)
        {
            glm_vec3_copy(game_world.cam.position, game_world.cam.last_position);
            glm_quat_copy(game_world.cam.pitch_rotation, game_world.cam.last_pitch_rotation);
            glm_quat_copy(game_world.cam.yaw_rotation, game_world.cam.last_yaw_rotation);

            process_input(window, &game_world.cam, tick_speed);
            camera_update(&game_world.cam, lastMouseX - mouseX, lastMouseY - mouseY, tick_speed);
            
            camera_position = game_world.cam.position;
            double cam_block_distance = FLT_MAX;
            world_chunk_update(&game_world, &cam_block_distance);

            accumulated_time -= tick_speed;
        }

        double alpha = accumulated_time / tick_speed;
        // update accumulate stuff
        glm_vec3_lerp(game_world.cam.last_position, game_world.cam.position, alpha, game_world.cam.real_position);
        glm_quat_slerp(game_world.cam.last_pitch_rotation, game_world.cam.pitch_rotation, alpha, game_world.cam.real_pitch_rotation);
        glm_quat_slerp(game_world.cam.last_yaw_rotation, game_world.cam.yaw_rotation, alpha, game_world.cam.real_yaw_rotation);

        versor temp_cam_rot = {};
        // glm_quat_mul(game_world.cam.real_pitch_rotation, game_world.cam.real_yaw_rotation, temp_cam_rot);
        // glm_quat_rotatev(game_world.cam.real_yaw_rotation, (vec3){0.0, 0.0, 1.0}, game_world.cam.walk_direction);
        glm_quat_mul(game_world.cam.yaw_rotation, game_world.cam.pitch_rotation, temp_cam_rot);
        // glm_quat_copy(game_world.cam.real_pitch_rotation, temp_cam_rot);

        glm_quat_rotatev(game_world.cam.real_yaw_rotation, (vec3){0.0, 0.0, 1.0}, game_world.cam.walk_direction);
        glm_quat_rotatev(temp_cam_rot, (vec3){0.0, 0.0, 1.0}, game_world.cam.look_direction);

        glm_quat_look(game_world.cam.real_position, temp_cam_rot, game_world.cam.view);

        shader_set_mat4x4(&game_world.shaders, SHADER_COMMON, "view", game_world.cam.view);
        shader_set_mat4x4(&game_world.shaders, SHADER_COMMON, "proj", proj);
        world_draw(&game_world);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) && !left_held)
        {
            world_break_block(&game_world);
            left_held = true;
        }
        if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
        {
            left_held = false;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) && !right_held)
        {
            world_place_block(&game_world);
            right_held = true;
        }
        if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2))
        {
            right_held = false;
        }
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    world_exit(&game_world);
    glfwTerminate();
}

void framebuffer_size(GLFWwindow *window, int width, int height)
{
    global_window_width = width;
    global_window_height = height;
    glViewport(0, 0, width, height);
    global_resize_alert = true;
}

void process_input(GLFWwindow *window, camera *cam, double tick_speed)
{
    vec3 movement = {};
    double speed = 4.0;

    glm_cross(cam->walk_direction, cam->up, cam->right);

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
        glm_vec3_sub(movement, cam->walk_direction, movement);
    }
    if (glfwGetKey(window, GLFW_KEY_S))
    {
        glm_vec3_add(movement, cam->walk_direction, movement);
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

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL))
    {
        speed *= 3.0;
    }

    glm_normalize(movement);
    glm_vec3_scale(movement, speed, movement);
    glm_vec3_scale(movement, tick_speed, movement);
    glm_vec3_add(cam->position, movement, cam->position);
}