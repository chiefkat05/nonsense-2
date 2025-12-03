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

typedef struct
{
    unsigned int count;
    chunk **arr;
} chunk_hashmap;
void chunk_map_alloc(chunk_hashmap *map, uint32 size)
{
    map->arr = (chunk **)calloc(size, sizeof(chunk *));
    verify(map->arr, "failed to allocate chunk hashmap", __LINE__);
    map->count = size;
}
void chunk_map_free(chunk_hashmap *map)
{
    free(map->arr);
    map->count = 0;
}
const unsigned int chunk_render_area_edge = 10;
const double render_distance = 2500.0;
int chunk_map_function(int x, int y, int z)
{
    // ! idea: make this 'not' perfectly unique, by subtracting the player position or something, keeping the hash function always
    // centered around where the player is. That will make the hash map stay as small as the render_area (render distance) at all times
    // but don't bother rushing to implement this, just get it done before 1.0 release
    int out_x = abs(x) * 2;
    int out_y = abs(y) * 2;
    int out_z = abs(z) * 2;
    if (x < 0) out_x += 1;
    if (y < 0) out_y += 1;
    if (z < 0) out_z += 1;

    int index = out_z * chunk_render_area_edge * chunk_render_area_edge + out_y * chunk_render_area_edge + out_x;

    return index;
}
void chunk_map_rehash(chunk_hashmap *map)
{
    int new_size = map->count * 2;
    chunk **new_map_arr = (chunk **)calloc(new_size, sizeof(chunk *));
    verify(new_map_arr, "failed to allocate memory for chunk map rehash", new_size);
    // memset(new_map_arr, 0, new_size);
    memcpy(new_map_arr, map->arr, map->count * sizeof(chunk *));
    free(map->arr);
    map->arr = new_map_arr;
    map->count *= 2;
}
void chunk_map_insert(chunk_hashmap *map, int x, int y, int z, chunk *p_chunk)
{
    unsigned int index = chunk_map_function(x, y, z);

    while (index >= map->count)
    {
        chunk_map_rehash(map);
    }
    map->arr[index] = p_chunk;
}
void chunk_map_remove(chunk_hashmap *map, int x, int y, int z)
{
    unsigned int index = chunk_map_function(x, y, z);
    if (index >= map->count)
        return;

    map->arr[index] = NULL;
}
chunk *chunk_map_lookup(chunk_hashmap *map, int x, int y, int z)
{
    unsigned int index = chunk_map_function(x, y, z);
    if (index >= map->count || !map->arr[index])
        return NULL;
        
    return map->arr[index];
}
void chunk_map_print(chunk_hashmap *map)
{
    printf("--------------------hashmap data--------------------\n");
    for (int i = 0; i < map->count; ++i)
    {
        if (map->arr[i] == NULL)
            continue;

        printf("%i / %i chunk at pointer %p\n", i, map->count, (void *)map->arr[i]);
        printf("pos: %i %i %i\n", map->arr[i]->x, map->arr[i]->y, map->arr[i]->z);
    }
    printf("---end hashmap data. hashmap total size: %d bytes---\n", map->count * sizeof(chunk *));
}
// this only runs once per tick... still too much?
// (btw implement ticks)
// I've seen a lot of suggestions that the flat array of chunks + pointers to sibling chunks is faster than hashmaps, look into that please
void chunk_map_cleanup(chunk_hashmap *map, vec3 player_pos)
{
    for (int i = 0; i < map->count; ++i)
    {
        if (!map->arr[i])
            continue;

        vec3 chunk_pos = {map->arr[i]->x * CHUNK_EDGE_LENGTH, map->arr[i]->y * CHUNK_EDGE_LENGTH, map->arr[i]->z * CHUNK_EDGE_LENGTH};
        double distance = glm_vec3_distance2(player_pos, chunk_pos);
        if (distance > render_distance)
        {
            chunk_map_remove(map, map->arr[i]->x, map->arr[i]->y, map->arr[i]->z);
        }
    }
}

typedef struct
{
    chunk *chunk_list;
    int chunk_count;
    int world_x_count, world_y_count, world_z_count;
    unsigned int texture_id, texture_width, texture_height;
    int lookat_block, placement_block;
    int lookat_block_normal;
    int lookat_chunk;
    chunk *placement_chunk;
    chunk_hashmap chunk_map;
} world;

// void world_allocate_chunk(world *w)
// {
//     w->chunk_list + w->chunk_count = (chunk *)malloc(sizeof(chunk));
//     ++w->chunk_count;
// }

void world_break_block(world *w)
{
    if (w->lookat_block < 0)
        return;
    w->chunk_list[w->lookat_chunk].blocks[w->lookat_block].id = BLOCK_NULL;
    w->chunk_list[w->lookat_chunk].dirty = true;

    vec3 block_pos = {};
    chunk_get_position_from_block(&w->chunk_list[w->lookat_chunk], w->lookat_block, block_pos, __LINE__);

    int x = block_pos[0];
    int y = block_pos[1];
    int z = block_pos[2];

    int chunk_update_normal_x = 0, chunk_update_normal_y = 0, chunk_update_normal_z = 0;

    if (x == 0)
        chunk_update_normal_x = -1;
    if (x + 1 >= CHUNK_EDGE_LENGTH)
        chunk_update_normal_x = 1;
    if (y == 0)
        chunk_update_normal_y = -1;
    if (y + 1 >= CHUNK_EDGE_LENGTH)
        chunk_update_normal_y = 1;
    if (z == 0)
        chunk_update_normal_z = -1;
    if (z + 1 >= CHUNK_EDGE_LENGTH)
        chunk_update_normal_z = 1;

    int chx = w->chunk_list[w->lookat_chunk].x;
    int chy = w->chunk_list[w->lookat_chunk].y;
    int chz = w->chunk_list[w->lookat_chunk].z;

    if (chunk_update_normal_x != 0)
    {
        // int upd_index = world_get_chunk_from_position(w, chx + chunk_update_normal_x, chy, chz);
        // w->chunk_list[upd_index].dirty = true;
        chunk *p_chunk = chunk_map_lookup(&w->chunk_map, chx + chunk_update_normal_x, chy, chz);
        if (p_chunk)
            p_chunk->dirty = true;
    }
    if (chunk_update_normal_y != 0)
    {
        // int upd_index = world_get_chunk_from_position(w, chx, chy + chunk_update_normal_y, chz);
        // w->chunk_list[upd_index].dirty = true;
        chunk *p_chunk = chunk_map_lookup(&w->chunk_map, chx, chy + chunk_update_normal_y, chz);
        if (p_chunk)
            p_chunk->dirty = true;
    }
    if (chunk_update_normal_z != 0)
    {
        // int upd_index = world_get_chunk_from_position(w, chx, chy, chz + chunk_update_normal_z);
        // w->chunk_list[upd_index].dirty = true;
        chunk *p_chunk = chunk_map_lookup(&w->chunk_map, chx, chy, chz + chunk_update_normal_z);
        if (p_chunk)
            p_chunk->dirty = true;
    }
}
void world_place_block(world *w)
{
    int chunk_size = CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
    int slab_size = CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH;
    int row_size = CHUNK_EDGE_LENGTH;

    int new_block_pos = -1;
    int x = w->placement_block % CHUNK_EDGE_LENGTH;
    int y = (w->placement_block / CHUNK_EDGE_LENGTH) % CHUNK_EDGE_LENGTH;
    int z = w->placement_block / (CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH);
    switch (w->lookat_block_normal)
    {
    case 5:
        // pos z
        new_block_pos = w->placement_block + slab_size;
        ++z;
        break;
    case 4:
        // neg z
        new_block_pos = w->placement_block - slab_size;
        --z;
        break;
    case 3:
        // pos y
        new_block_pos = w->placement_block + row_size;
        ++y;
        break;
    case 2:
        // neg y
        new_block_pos = w->placement_block - row_size;
        --y;
        break;
    case 1:
        // pos x
        new_block_pos = w->placement_block + 1;
        ++x;
        break;
    case 0:
        // neg x
        new_block_pos = w->placement_block - 1;
        --x;
        break;
    default:
        break;
    }

    if (new_block_pos > -1 && new_block_pos < chunk_size && w->placement_chunk)
    {
        w->placement_chunk->blocks[new_block_pos].id = BLOCK_ROCK;
        w->placement_chunk->dirty = true;
    }
}
void world_chunk_generation(world *w, camera *cam)
{
    w->world_x_count = 10;
    w->world_y_count = 4;
    w->world_z_count = 10;
    int total_world_size = w->world_x_count * w->world_y_count * w->world_z_count;
    w->chunk_list = (chunk *)calloc(total_world_size, sizeof(chunk));
    verify(w->chunk_list, "failed to allocate world chunk area", __LINE__); // windows build -> x11 fps test (let's go)

    chunk_map_alloc(&w->chunk_map, total_world_size * 2);

    // generation step
    for (int z = -2; z < 8; ++z)
    {
        for (int y = -3; y < 0; ++y)
        {
            for (int x = -2; x < 8; ++x) // figure out how to have negative chunks. You'll probably need to take away the single array of chunks, but idk what you'd replace it with
            {                                          // you could do an arbitrary array/pool, but then you'd need to go through each use of world_get_chunk_pos or world_get_chunk_id and replace it with a held world variable or something
                // int chunk_index = z * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH + y * CHUNK_EDGE_LENGTH + x; // yea chunk edge length has nothing to do with this
                chunk *current_chunk = &w->chunk_list[w->chunk_count];

                chunk_generation(current_chunk, x, y, z);
                chunk_allocate(current_chunk);
                current_chunk->texture_id = w->texture_id;
                current_chunk->texture_width = w->texture_width;
                current_chunk->texture_height = w->texture_height;
                
                chunk_map_insert(&w->chunk_map, x, y, z, current_chunk);

                ++w->chunk_count;
            }
        }
    }

    for (int i = 0; i < w->chunk_count; ++i)
    {
        int x = w->chunk_list[i].x, y = w->chunk_list[i].y, z = w->chunk_list[i].z;

        chunk *this_chunk = chunk_map_lookup(&w->chunk_map, x, y, z);
        if (!this_chunk)
            continue;
        
        chunk *left_chunk = chunk_map_lookup(&w->chunk_map, x - 1, y, z);
        chunk *right_chunk = chunk_map_lookup(&w->chunk_map, x + 1, y, z);
        chunk *forwards_chunk = chunk_map_lookup(&w->chunk_map, x, y, z + 1);
        chunk *backwards_chunk = chunk_map_lookup(&w->chunk_map, x, y, z - 1);
        chunk *up_chunk = chunk_map_lookup(&w->chunk_map, x, y + 1, z);
        chunk *down_chunk = chunk_map_lookup(&w->chunk_map, x, y - 1, z);
        
        build_chunk_mesh(this_chunk, left_chunk, right_chunk, forwards_chunk, backwards_chunk, up_chunk, down_chunk);
    }
    chunk_map_cleanup(&w->chunk_map, cam->pos);
}
void world_chunk_update(world *w, camera *cam, shader_list *shaders)
{
    double lookat_block_distance = DBL_MAX;
    for (int i = 0; i < w->chunk_count; ++i)
    {
        int x = w->chunk_list[i].x;
        int y = w->chunk_list[i].y;
        int z = w->chunk_list[i].z;
        // THIS IS THE CHUNK POSITION MULTIPLIED BY THE CHUNK EDGE SIZE NOT BY HOW MANY CHUNKS THERE ARE IN THE WORLD WRAP YOUR HEAD AROUND THAT
        int chunk_pos_x = x * CHUNK_EDGE_LENGTH;
        int chunk_pos_y = y * CHUNK_EDGE_LENGTH;
        int chunk_pos_z = z * CHUNK_EDGE_LENGTH;

        // make this distance2 I think
        // ok I did
        double cam_to_chunk_dist = glm_vec3_distance2((vec3){chunk_pos_x, chunk_pos_y, chunk_pos_z}, cam->pos);
        if (cam_to_chunk_dist > render_distance)
            continue;

        if (!chunk_map_lookup(&w->chunk_map, x, y, z))
            chunk_map_insert(&w->chunk_map, x, y, z, &w->chunk_list[i]);

        int temp_lookat = -1;
        int temp_lookat_chunk_norm = -1;

        chunk *right_chunk = chunk_map_lookup(&w->chunk_map, x + 1, y, z);
        chunk *left_chunk = chunk_map_lookup(&w->chunk_map, x - 1, y, z);
        chunk *forwards_chunk = chunk_map_lookup(&w->chunk_map, x, y, z + 1);
        chunk *backwards_chunk = chunk_map_lookup(&w->chunk_map, x, y, z - 1);
        chunk *up_chunk = chunk_map_lookup(&w->chunk_map, x, y + 1, z);
        chunk *down_chunk = chunk_map_lookup(&w->chunk_map, x, y - 1, z);

        update_chunk(&w->chunk_list[i], left_chunk, right_chunk, forwards_chunk, backwards_chunk, up_chunk, down_chunk,
                        cam, &temp_lookat, &w->lookat_block_normal, &lookat_block_distance, &temp_lookat_chunk_norm);
        if (temp_lookat != -1)
        {
            w->lookat_chunk = i;
            vec3 current_chunk_position = {w->chunk_list[i].x, w->chunk_list[i].y, w->chunk_list[i].z};
            vec3 new_chunk_position = {};
            glm_vec3_copy(current_chunk_position, new_chunk_position);
            w->placement_block = temp_lookat;
            switch (temp_lookat_chunk_norm)
            {
            case 0: // x
                --new_chunk_position[0];
                w->placement_block += CHUNK_EDGE_LENGTH;
                break;
            case 1:
                ++new_chunk_position[0];
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
                --new_chunk_position[2];
                w->placement_block += (CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH);
                break;
            case 5:
                ++new_chunk_position[2];
                w->placement_block -= (CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH * CHUNK_EDGE_LENGTH);
                break;
            }
            w->lookat_block = temp_lookat;
            w->placement_chunk = chunk_map_lookup(&w->chunk_map, new_chunk_position[0], new_chunk_position[1], new_chunk_position[2]);
        }
        draw_chunk(&w->chunk_list[i], shaders, w->lookat_block);
    }

    chunk_map_cleanup(&w->chunk_map, cam->pos);
}
void world_exit(world *w)
{
    for (int i = 0; i < w->chunk_count; ++i)
    {
        chunk_free(&w->chunk_list[i]);
    }
    free(w->chunk_list);
    chunk_map_free(&w->chunk_map);

    free(w);
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

    world *test_world = calloc(1, sizeof(world));
    test_world->texture_id = texture_load_from_bmp(1, "./atlas.bmp", &test_world->texture_width, &test_world->texture_height);
    test_world->texture_width /= 16.0;
    test_world->texture_height /= 16.0;
    world_chunk_generation(test_world, &cam);

    double mouseX = 0, mouseY = 0;
    double lastMouseX = mouseY, lastMouseY = mouseY;
    bool left_held = false, right_held = false;

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

        world_chunk_update(test_world, &cam, &shaders);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) && !left_held)
        {
            world_break_block(test_world);
            left_held = true;
        }
        if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
        {
            left_held = false;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) && !right_held)
        {
            world_place_block(test_world);
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
    world_exit(test_world);
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