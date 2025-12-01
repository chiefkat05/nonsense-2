#include "shader.h"
#include "chunk.h"
#include "definitions.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
static bool global_resize_alert;
static int global_window_width;
static int global_window_height;

void framebuffer_size(GLFWwindow *window, int width, int height);
void process_input(GLFWwindow *window, camera *cam, double delta_time);

// texture atlas
// limit chunk editing to inside chunk
// chunk mesh generator
// collision detection

typedef struct
{
    chunk *chunk_list;
    int chunk_count;
    int world_x_count, world_y_count, world_z_count;
    unsigned int texture_id, texture_width, texture_height;
    int lookat_block, placement_block;
    int lookat_block_normal;
    int lookat_chunk, placement_chunk;
} world;

// void world_allocate_chunk(world *w)
// {
//     w->chunk_list + w->chunk_count = (chunk *)malloc(sizeof(chunk));
//     ++w->chunk_count;
// }

int world_get_chunk_from_position(world *w, int x, int y, int z)
{
    int world_slab = w->world_x_count * w->world_y_count;
    int world_row = w->world_x_count;

    if (z < 0 || z >= w->world_z_count || y < 0 || y >= w->world_y_count || x < 0 || x >= w->world_x_count)
        return -1;
    return z * world_slab + y * world_row + x;
}

// I haven't the slightest idea, thank you to Ross Millikan from https://math.stackexchange.com/questions/19765/calculating-coordinates-from-a-flattened-3d-array-when-you-know-the-size-index
void world_get_position_from_chunk(world *w, int i, vec3 output)
{
    int z = i / (w->world_x_count * w->world_y_count);
    int y = (i - (z * w->world_x_count * w->world_y_count)) / w->world_x_count;
    int x = i - (z * w->world_x_count * w->world_y_count) - (y * w->world_x_count);
    glm_vec3_copy((vec3){x, y, z}, output);
}
void world_chunk_generation(world *w)
{
    w->world_x_count = 10;
    w->world_y_count = 2;
    w->world_z_count = 10;
    int total_world_size = w->world_x_count * w->world_y_count * w->world_z_count;
    w->chunk_list = (chunk *)malloc(sizeof(chunk) * total_world_size);
    verify(w->chunk_list, "failed to allocate world chunk area", __LINE__);
    for (int z = 0; z < w->world_z_count; ++z)
    {
        for (int y = 0; y < w->world_y_count; ++y)
        {
            for (int x = 0; x < w->world_x_count; ++x)
            {
                int chunk_index = world_get_chunk_from_position(w, x, y, z);

                chunk_generation(&w->chunk_list[chunk_index], z, y, x);
                chunk_allocate(&w->chunk_list[chunk_index]);
                w->chunk_list[chunk_index].texture_id = w->texture_id;
                w->chunk_list[chunk_index].texture_width = w->texture_width;
                w->chunk_list[chunk_index].texture_height = w->texture_height;

                // messy please make better
                chunk *left_chunk = NULL, *right_chunk = NULL, *forwards_chunk = NULL, *backwards_chunk = NULL, *up_chunk = NULL, *down_chunk = NULL;
                int left_chunk_index = -1, right_chunk_index = -1, forwards_chunk_index = -1, backwards_chunk_index = -1, up_chunk_index = -1, down_chunk_index = -1;
                left_chunk_index = world_get_chunk_from_position(w, x - 1, y, z);
                right_chunk_index = world_get_chunk_from_position(w, x + 1, y, z);
                forwards_chunk_index = world_get_chunk_from_position(w, x, y, z + 1);
                backwards_chunk_index = world_get_chunk_from_position(w, x, y, z - 1);
                up_chunk_index = world_get_chunk_from_position(w, x, y + 1, z);
                down_chunk_index = world_get_chunk_from_position(w, x, y - 1, z);

                if (left_chunk_index != -1)
                    left_chunk = &w->chunk_list[left_chunk_index];
                if (right_chunk_index != -1)
                    right_chunk = &w->chunk_list[right_chunk_index];
                if (forwards_chunk_index != -1)
                    forwards_chunk = &w->chunk_list[forwards_chunk_index];
                if (backwards_chunk_index != -1)
                    backwards_chunk = &w->chunk_list[backwards_chunk_index];
                if (up_chunk_index != -1)
                    up_chunk = &w->chunk_list[up_chunk_index];
                if (down_chunk_index != -1)
                    down_chunk = &w->chunk_list[down_chunk_index];

                build_chunk_mesh(&w->chunk_list[chunk_index], left_chunk, right_chunk, forwards_chunk, backwards_chunk, up_chunk, down_chunk);
                ++w->chunk_count;
            }
        }
    }
}
void world_chunk_update(world *w, camera *cam, shader_list *shaders)
{
    double lookat_block_distance = DBL_MAX;
    for (int i = 0; i < w->chunk_count; ++i)
    {
        int chunk_center_x = (w->chunk_list[i].x) * CHUNK_EDGE_LENGTH;
        int chunk_center_y = (w->chunk_list[i].y) * CHUNK_EDGE_LENGTH;
        int chunk_center_z = (w->chunk_list[i].z) * CHUNK_EDGE_LENGTH;

        double cam_to_chunk_dist = glm_vec3_distance((vec3){chunk_center_x, chunk_center_y, chunk_center_z}, cam->pos);
        if (cam_to_chunk_dist < 50.0)
        {
            int temp_lookat = -1;
            int temp_lookat_chunk_norm = -1;
            update_chunk(&w->chunk_list[i], cam, &temp_lookat, &w->lookat_block_normal, &lookat_block_distance, &temp_lookat_chunk_norm);
            if (temp_lookat != -1)
            {
                w->lookat_chunk = i;
                vec3 current_chunk_position = {};
                world_get_position_from_chunk(w, i, current_chunk_position);
                vec3 new_chunk_position = {};
                glm_vec3_copy(current_chunk_position, new_chunk_position);
                w->placement_block = temp_lookat;
                switch (temp_lookat_chunk_norm)
                {
                case 0: // x
                    --new_chunk_position[2];
                    w->placement_block += CHUNK_EDGE_LENGTH;
                    break;
                case 1:
                    ++new_chunk_position[2];
                    w->placement_block -= CHUNK_EDGE_LENGTH;
                    break;
                case 2: // y
                    --new_chunk_position[1];
                    w->placement_block += CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
                    break;
                case 3:
                    ++new_chunk_position[1];
                    w->placement_block -= CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
                    break;
                case 4: // z
                    --new_chunk_position[0];
                    w->placement_block += (CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH);
                    break;
                case 5:
                    ++new_chunk_position[0];
                    w->placement_block -= (CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH);
                    break;
                }
                w->lookat_block = temp_lookat;
                w->placement_chunk = world_get_chunk_from_position(w, new_chunk_position[0], new_chunk_position[1], new_chunk_position[2]);
                if (w->placement_chunk < 0 || w->placement_chunk > (w->world_x_count * w->world_y_count * w->world_z_count))
                    w->placement_chunk = i;
            }
            draw_chunk(&w->chunk_list[i], shaders, w->lookat_block);
        }
    }
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

    shader_list shaders = {};

    mat4 proj;
    glm_mat4_identity(proj);

    camera cam = {};
    camera_setup(&cam, (vec3){0.0, 0.0, -3.0}, (vec3){0.0, 0.0, 0.0});
    glm_perspective(45, (double)WINDOW_WIDTH / (double)WINDOW_HEIGHT, 0.1, 100.0, proj);

    create_shader(&shaders, SHADER_COMMON, "./vertex.shader", "./fragment.shader");

    world test_world = {};
    test_world.texture_id = texture_load_from_bmp(1, "./atlas.bmp", &test_world.texture_width, &test_world.texture_height);
    test_world.texture_width /= 16.0;
    test_world.texture_height /= 16.0;
    world_chunk_generation(&test_world);

    double mouseX = 0, mouseY = 0;
    double lastMouseX = mouseY, lastMouseY = mouseY;
    bool left_held = false, right_held = false;

    const int chunk_slab = CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
    const int chunk_row = CHUNK_EDGE_LENGTH;

    double delta_time = 0.0;
    double last_time = 0.0;
    double current_time = 0.0;
    while (!glfwWindowShouldClose(window))
    {
        if (global_resize_alert)
        {
            glm_perspective(45, (double)global_window_width / (double)global_window_height, 0.1, 100.0, proj);
            global_resize_alert = false;
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
        shader_set_mat4x4(&shaders, SHADER_COMMON, "view", cam.view);
        shader_set_mat4x4(&shaders, SHADER_COMMON, "proj", proj);

        world_chunk_update(&test_world, &cam, &shaders);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) && !left_held)
        {
            break_chunk_block(&test_world.chunk_list[test_world.lookat_chunk], test_world.lookat_block);
            left_held = true;
        }
        if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
        {
            left_held = false;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) && !right_held)
        {
            // pass in the correct chunk here
            vec3 chunk_place_pos = {};
            world_get_position_from_chunk(&test_world, test_world.placement_chunk, chunk_place_pos);
            // printf("trying to place in chunk %i at position %f %f %f\n", test_world.placement_chunk,
            //        chunk_place_pos[0], chunk_place_pos[1], chunk_place_pos[2]);
            place_chunk_block(&test_world.chunk_list[test_world.placement_chunk], test_world.placement_block, test_world.lookat_block_normal);
            right_held = true;
        }
        if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2))
        {
            right_held = false;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

void framebuffer_size(GLFWwindow *window, int width, int height)
{
    global_window_width = width;
    global_window_height = height;
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

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL))
    {
        speed *= 3.0;
    }

    glm_normalize(movement);
    glm_vec3_scale(movement, speed, movement);
    glm_vec3_scale(movement, delta_time, movement);
    glm_vec3_add(cam->pos, movement, cam->pos);
}