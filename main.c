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
void process_input(GLFWwindow *window, camera *cam, double tick_speed);

typedef struct
{
    unsigned int vao;
    unsigned int t_id;
} ui_element;

int ui_vertices[] = {
    -1, -1, 0,
     1, -1, 0,
     1,  1, 0,

    -1, -1, 0,
     1,  1, 0,
    -1,  1, 0
};
void make_ui_element()
{

}
void load_png_texture(const char *path, unsigned int opengl_texture_index)
{

}

const double tick_speed = 1.0 / 200.0;

// today's tasks:
// dda or voxel-raycast for the looking-at-blocks algorithm
// png loading with 1 simple texture on screen

// if you have time (but you can just take a break if you want):
// simple noise world generation
// get windows build working

// local-only hashmap is still maybe on the table to avoid heavy memory usage when travelling to outer blocks
// collision detection + gravity
// multithreading for chunk gen
// animate the texture with the most barebones timer countdown and frame increment

int chunk_dda_test(chunk *c, vec3 pos, vec3 dir, int *f)
{
    int chunk_x = c->x * CHUNK_EDGE;
    int chunk_y = c->y * CHUNK_EDGE;
    int chunk_z = c->z * CHUNK_EDGE;

    int x = floor(pos[0]) - chunk_x;
    int y = floor(pos[1]) - chunk_y;
    int z = floor(pos[2]) - chunk_z;

    double x_step = (fabs(dir[0])/dir[0]);
    double y_step = (fabs(dir[1])/dir[1]);
    double z_step = (fabs(dir[2])/dir[2]);


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

    int face_norm = -1;
    int block = -1;

    face_norm = -1;
    while ((block == -1 || c->blocks[block].id == BLOCK_NULL))
    {
        block = chunk_get_block_from_position(x, y, z);
        if (maxx < maxy && maxx < maxz)
        {
            x += x_step;
            if (x_step < 0 && x < 0 || x_step > 0 && x >= CHUNK_EDGE)
                return -1;

            maxx += delta_x;
            face_norm = x_step > 0 ? 0 : 1;
            block = chunk_get_block_from_position(x, y, z);
            continue;
        }
        if (maxy < maxx && maxy < maxz)
        {
            y += y_step;
            if (y_step < 0 && y < 0 || y_step > 0 && y >= CHUNK_EDGE)
                return -1;

            maxy += delta_y;
            face_norm = y_step > 0 ? 2 : 3;
            block = chunk_get_block_from_position(x, y, z);
            continue;
        }
        if (maxz < maxx && maxz < maxy)
        {
            z += z_step;
            if (z_step < 0 && z < 0 || z_step > 0 && z >= CHUNK_EDGE)
                return -1;

            maxz += delta_z;
            face_norm = z_step > 0 ? 4 : 5;
            block = chunk_get_block_from_position(x, y, z);
            continue;
        }
        return -1;
    }
    *f = face_norm;
    return block;
}

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

    srand(time(NULL));

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
        glm_quat_mul(game_world.cam.real_yaw_rotation, game_world.cam.real_pitch_rotation, temp_cam_rot);

        glm_quat_rotatev(game_world.cam.real_yaw_rotation, (vec3){0.0, 0.0, 1.0}, game_world.cam.walk_direction);
        glm_quat_rotatev(temp_cam_rot, (vec3){0.0, 0.0, 1.0}, game_world.cam.look_direction);
        glm_quat_rotatev(temp_cam_rot, (vec3){0.0, 0.0, -1.0}, game_world.cam.inv_look_direction);

        glm_quat_look(game_world.cam.real_position, temp_cam_rot, game_world.cam.view);

        shader_set_mat4x4(&game_world.shaders, SHADER_COMMON, "view", game_world.cam.view);
        shader_set_mat4x4(&game_world.shaders, SHADER_COMMON, "proj", proj);
        world_draw(&game_world);

        
        vec3 norm_cam_dir = {};
        glm_vec3_normalize_to(game_world.cam.inv_look_direction, norm_cam_dir);
        int block_see = chunk_dda_test(chunk_map_lookup(&game_world.chunk_map, 0, -1, 0),
            game_world.cam.real_position, norm_cam_dir, &game_world.lookat_block_normal); // yay
        game_world.lookat_block = block_see;
        game_world.placement_block = block_see;
        game_world.lookat_chunk = chunk_map_lookup(&game_world.chunk_map, 0, -1, 0);
        game_world.placement_chunk = chunk_map_lookup(&game_world.chunk_map, 0, -1, 0);
        if (game_world.lookat_block != -1)
        {
            vec3 block_pos = {};
            chunk_get_position_from_block(game_world.lookat_block, block_pos, __LINE__);
            // printf("looking at %f %f %f\n", block_pos[0], block_pos[1], block_pos[2]);
        }

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