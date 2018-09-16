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

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int MESH_SIZE = 50;

const float ORTHO_WIDTH = (float)SCREEN_WIDTH/(float)MESH_SIZE;
const float ORTHO_HEIGHT = (float)SCREEN_HEIGHT/(float)MESH_SIZE;

const int DEFAULT_BOARD_WIDTH = 17;
const int DEFAULT_BOARD_HEIGHT = 13;

const int MAX_COOLDOWN = 5;

bool windowInit(GLFWwindow*&);

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
    if (!windowInit(window)) {
        return -1;
    }

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
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(5,10,true,StickyLevel::Weak)));

    world_map->put_quiet(std::move(std::make_unique<Wall>(1,1)));

    world_map->put_quiet(std::move(std::make_unique<PushBlock>(3,1,false,StickyLevel::Weak)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(4,1,false,StickyLevel::Weak)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(4,2,false,StickyLevel::Weak)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(4,3,false,StickyLevel::Weak)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(4,4,false,StickyLevel::Weak)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(3,4,false,StickyLevel::Weak)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(2,4,false,StickyLevel::Weak)));

    world_map->put_quiet(std::move(std::make_unique<PushBlock>(1,2,false,StickyLevel::Strong)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(2,2,false,StickyLevel::Strong)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(3,2,false,StickyLevel::Strong)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(2,3,false,StickyLevel::Strong)));

    world_map->put_quiet(std::move(std::make_unique<Wall>(9,1)));

    world_map->put_quiet(std::move(std::make_unique<PushBlock>(7,1,false,StickyLevel::Weak)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(6,1,false,StickyLevel::Weak)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(6,2,false,StickyLevel::Weak)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(6,3,false,StickyLevel::Weak)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(6,4,false,StickyLevel::Weak)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(7,4,false,StickyLevel::Weak)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(8,4,false,StickyLevel::Weak)));

    world_map->put_quiet(std::move(std::make_unique<PushBlock>(9,2,false,StickyLevel::Strong)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(8,2,false,StickyLevel::Strong)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(7,2,false,StickyLevel::Strong)));
    world_map->put_quiet(std::move(std::make_unique<PushBlock>(8,3,false,StickyLevel::Strong)));

    world_map->set_initial_state();
    auto player = world_map->prime_mover();

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

    while(!glfwWindowShouldClose(window)) {
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
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
                Loader::save(world_map.get());
            }
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
                WorldMap* new_world_map = Loader::load();
                if (new_world_map) {
                    world_map.reset(new_world_map);
                    world_map->set_initial_state();
                    player = world_map->prime_mover();
                    undo_stack = UndoStack(1000);
                }
            }
        }
        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
            if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
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
        if (cooldown == 0) {
            for (auto p : movement_keys) {
                // For starters, we'll use a naive input processing method.
                // In particular, the precedence of keys is arbitrary
                // and there is no buffering.
                if (glfwGetKey(window, p.first) == GLFW_PRESS) {
                    //StartCounter();
                    MoveProcessor(world_map.get(), p.second).try_move(delta_frame.get());
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

        cam_x = cos(cam_incline) * sin(cam_rotation) * cam_radius;
        cam_y = sin(cam_incline) * cam_radius;
        cam_z = cos(cam_incline) * cos(cam_rotation) * cam_radius;

        Point player_pos = Point {0, 0};
        if (player) {
            player_pos = player->pos();
        }

        if (view_mode == ViewMode::Perspective) {
            view = glm::lookAt(glm::vec3(cam_x + player_pos.x, cam_y, cam_z + player_pos.y),
                               glm::vec3(player_pos.x, 0.0f, player_pos.y),
                               glm::vec3(0.0f, 1.0f, 0.0f));
            view = glm::translate(view, glm::vec3(0.5, 0.0, 0.5));
            projection = glm::perspective(glm::radians(60.0f), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100.0f);
        } else { // view_mode == ViewMode::Ortho
            view = glm::lookAt(glm::vec3(player_pos.x, 2.0f, player_pos.y),
                               glm::vec3(player_pos.x, 0.0f, player_pos.y),
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

        glfwPollEvents();
        glfwSwapBuffers(window);

        undo_stack.push(std::move(delta_frame));

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    glfwTerminate();
    return 0;
}

bool windowInit(GLFWwindow*& window) {
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
