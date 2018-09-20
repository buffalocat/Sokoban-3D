// For good measure, turn off all of the pedantic warnings for these includes
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Winline"
#pragma GCC diagnostic ignored "-Wpedantic"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <dear/imgui.h>
#include <dear/imgui_impl_glfw.h>
#include <dear/imgui_impl_opengl3.h>

#pragma GCC diagnostic pop
// Then turn them back on for standard includes and headers from this project

#include <iostream>
#include <fstream>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <chrono>

#include "gameobject.h"
#include "block.h"
#include "delta.h"
#include "worldmap.h"
#include "shader.h"
#include "loader.h"
#include "moveprocessor.h"
#include "editor.h"
#include "camera.h"

static PushBlock FAKE_PLAYER(0,0,true,StickyLevel::None);


bool window_init(GLFWwindow*&);

Block* find_player(WorldMap*);

void print_controls();

enum class ViewMode {
    Perspective,
    Ortho,
};

// Code copied from https://stackoverflow.com/questions/1739259/how-to-use-queryperformancecounter for profiling!

#include <windows.h>

double PCFreq = 0.0;
__int64 CounterStart = 0;

void StartCounter()
{
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
    std::cout << "QueryPerformanceFrequency failed!\n";

    PCFreq = double(li.QuadPart)/1000000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}
double GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/PCFreq;
}
// End copied code

int main(void) {
    GLFWwindow* window;
    if (!window_init(window)) {
        return -1;
    }

    Editor editor(window);

    print_controls();

    float cubeVertices[24];
    for (int i = 0; i < 8; ++i) {
        cubeVertices[3*i] = (i & 1) - 0.5f;
        cubeVertices[3*i+1] = ((i >> 1) & 1) - 0.5f;
        cubeVertices[3*i+2] = ((i >> 2) & 1) - 0.5f;
    }

    int cubeTriangles[36];

    int i = 0;
    for (int a : {1,2,4}) {
        for (int b : {1,2,4}) {
            if (a == b) continue;
            for (int x : {0, a, a|b, 7, 7 & (~a), 7 & (~(a|b))}) {
                cubeTriangles[i] = x;
                ++i;
            }
        }
    }

    // Vertex Array Object
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Vertex Buffer Object
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) nullptr);
    glEnableVertexAttribArray(0);

    // Element Buffer Object
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeTriangles), cubeTriangles, GL_STATIC_DRAW);

    // Load and ccompile shaders

    Shader shader = Shader("shaders\\shader.vs", "shaders\\shader.fs");

    glClearColor(0.2f, 0.1f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    shader.use();

    // Init game logic stuff

    auto world_map = std::make_unique<WorldMap>(DEFAULT_BOARD_WIDTH, DEFAULT_BOARD_HEIGHT);
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(1,1,true,StickyLevel::None)));
    world_map->set_initial_state();
    Block* player = find_player(world_map.get());

    world_map.reset(Loader::load("tworooms.map"));
    world_map->set_initial_state();
    player = find_player(world_map.get());

    Camera camera(world_map.get(), 16.0, player->pos());
    camera.push_context(std::make_unique<FixedCameraContext>(6,6,4,4,2,7.0f,8,8));
    camera.push_context(std::make_unique<NullCameraContext>(5,5,6,6,1));
    camera.push_context(std::make_unique<FixedCameraContext>(11,2,3,6,2,10.0f,13,5));
    camera.push_context(std::make_unique<NullCameraContext>(10,1,5,8,1));

    int cooldown = 0;

    const glm::vec4 YELLOW = glm::vec4(0.7f, 0.7f, 0.3f, 1.0f);

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    float cam_radius = 16.0f;
    float cam_incline = 1.0f;
    float cam_rotation = 0.0f;
    float cam_x;
    float cam_y;
    float cam_z;

    std::unordered_map<int, Point> movement_keys {
        {GLFW_KEY_RIGHT, Point {1, 0}},
        {GLFW_KEY_LEFT,  Point {-1,0}},
        {GLFW_KEY_DOWN,  Point {0, 1}},
        {GLFW_KEY_UP,    Point {0,-1}},
    };

    UndoStack undo_stack(1000);

    glfwSwapInterval(0);

    ViewMode view_mode = ViewMode::Perspective;

    bool edit_mode = false;

    // ImGui init

    const char* glsl_version = "#version 330";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGui::StyleColorsDark();

    bool show_demo_window = true;

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }
        /*
        // Handle input
        auto delta_frame = std::make_unique<DeltaFrame>();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        if (view_mode == ViewMode::Perspective) {
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                cam_rotation -= .01f;
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                cam_rotation += .01f;
            }
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                cam_incline += .01f;
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                cam_incline -= .01f;
            }
        }
        if (edit_mode && view_mode == ViewMode::Ortho) {
            editor.handle_input(delta_frame.get(), world_map.get(), player->pos());
        }
        if (DEV_MODE) {
            if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                    Loader::save_dialog(world_map.get());
                }
                if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
                    WorldMap* new_world_map = Loader::load_dialog();
                    if (new_world_map) {
                        world_map.reset(new_world_map);
                        world_map->set_initial_state();
                        player = find_player(world_map.get());
                        undo_stack = UndoStack(1000);
                    }
                }
                if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
                    WorldMap* new_world_map = Loader::blank_map();
                    if (new_world_map) {
                        world_map.reset(new_world_map);
                        world_map->set_initial_state();
                        player = find_player(world_map.get());
                        undo_stack = UndoStack(1000);
                    }
                }
                if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
                    if (cooldown == 0) {
                        player = find_player(world_map.get());
                        cooldown = 3*MAX_COOLDOWN;
                    }
                }
                if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
                    if (cooldown == 0) {
                        edit_mode = !edit_mode;
                        if (edit_mode) {
                            std::cout << "Editor mode activated" << std::endl;
                        } else {
                            std::cout << "Editor mode deactivated" << std::endl;
                        }
                        cooldown = 3*MAX_COOLDOWN;
                    }
                }
                if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
                    if (cooldown == 0) {
                        world_map->set_initial_state();
                        std::cout << "Resynced Map" << std::endl;
                        cooldown = 3*MAX_COOLDOWN;
                    }
                }
                if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
                    if (cooldown == 0) {
                        if (view_mode == ViewMode::Perspective) {
                            view_mode = ViewMode::Ortho;
                        } else {
                            view_mode = ViewMode::Perspective;
                        }
                        // It's a little too easy to do this more times than you mean to...
                        cooldown = 3*MAX_COOLDOWN;
                    }
                }
            }
        }
        if (cooldown == 0) {
            for (auto p : movement_keys) {
                // For starters, we'll use a naive input processing method.
                // In particular, the precedence of keys is arbitrary
                // and there is no buffering.
                if (glfwGetKey(window, p.first) == GLFW_PRESS) {
                    //StartCounter();
                    MoveProcessor(world_map.get(), p.second).try_move(delta_frame.get());
                    camera.set_target(player->pos());
                    //world_map->print_snake_info();
                    //std::cout << "Move took " << GetCounter() << std::endl;
                    cooldown = MAX_COOLDOWN;
                    break;
                }
            }
        }
        // Only allow undo if we didn't just move
        if (cooldown == 0) {
            if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
                undo_stack.pop(world_map.get());
                cooldown = MAX_COOLDOWN;
            }
        } else {
            --cooldown;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.update();
        cam_radius = camera.get_radius();
        FPoint target_pos = camera.get_pos();

        cam_x = cos(cam_incline) * sin(cam_rotation) * cam_radius;
        cam_y = sin(cam_incline) * cam_radius;
        cam_z = cos(cam_incline) * cos(cam_rotation) * cam_radius;

        if (view_mode == ViewMode::Perspective) {
            view = glm::lookAt(glm::vec3(cam_x + target_pos.x, cam_y, cam_z + target_pos.y),
                               glm::vec3(target_pos.x, 0.0f, target_pos.y),
                               glm::vec3(0.0f, 1.0f, 0.0f));
            view = glm::translate(view, glm::vec3(0.5, 0.0, 0.5));
            projection = glm::perspective(glm::radians(60.0f), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100.0f);
        } else { // view_mode == ViewMode::Ortho
            view = glm::lookAt(glm::vec3(target_pos.x, 2.0f, target_pos.y),
                               glm::vec3(target_pos.x, 0.0f, target_pos.y),
                               glm::vec3(0.0f, 0.0f, -1.0f));
            projection = glm::ortho(-ORTHO_WIDTH/2.0f, ORTHO_WIDTH/2.0f, -ORTHO_HEIGHT/2.0f, ORTHO_HEIGHT/2.0f, 0.0f, 3.0f);
        }

        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        world_map->draw(&shader);

        // Draw the floor
        model = glm::translate(glm::mat4(), glm::vec3(-0.5, -0.1, -0.5));
        model = glm::scale(model, glm::vec3(world_map->width(), 0.1, world_map->height()));
        model = glm::translate(model, glm::vec3(0.5, -0.1, 0.5));
        shader.setMat4("model", model);

        shader.setVec4("color", YELLOW);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
        */
        ImGui::Render();

        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);

        //undo_stack.push(std::move(delta_frame));

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    glfwTerminate();
    return 0;
}

bool window_init(GLFWwindow*& window) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sokoban 3D", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    return true;
}

Block* find_player(WorldMap* world_map) {
    Block* player = world_map->prime_mover();
    // If there was no player, we make a "fake" one and don't put it on the map
    // It's not a great solution
    if (!player) {
        return &FAKE_PLAYER;
    }
    return player;
}

void print_controls() {
    std::cout << "Sokoban by Chris Hunt\n\n"
        "Arrow keys to move\n"
        "Z to undo\n"
        "WASD to rotate camera\n"
         << std::endl;
    if (!DEV_MODE) {
        return;
    }
    std::cout << "\nOther commands:\n\n"
        "ctrl+S to Save the current map\n"
        "ctrl+L to Load a map from the \\maps directory\n"
        "ctrl+N to create a New map (destroys current map!)\n"
        "ctrl+V to switch View Mode between Perspective and Orthographic\n"
        "ctrl+E to toggle Edit Mode (only usable in Ortho view)\n"
        "ctrl+P to cycle through Players to center the camera on\n"
        "ctrl+F to Force map reinitialization (activating links, etc)\n"
        << std::endl;
    std::cout << "\nIn Edit Mode:\n\n"
        "Use number keys to change active object\n"
        "Left click to place active object on tile, Right click to delete\n"
        "Note: you can undo creates/deletes in edit mode with Z\n"
        << std::endl;
}
