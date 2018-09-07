// For good measure, turn off all of the pedantic warnings for these includes
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"

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

#include "delta.h"
#include "worldmap.h"
#include "shader.h"
#include "gameobject.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int MAX_COOLDOWN = 5;

/*
void processInput(GLFWwindow *window, Point& pos) {
}*/

int main(void) {
    // Standard Init things
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sokoban 3D", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

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

    WorldMap world_map(BOARD_SIZE, BOARD_SIZE);
    auto player = std::make_unique<PushBlock>(5,5);
    player->set_car(true);
    Block* player_ptr = player.get();
    world_map.put_quiet(std::move(player));
    world_map.put_quiet(std::make_unique<PushBlock>(8,8));
    world_map.put_quiet(std::make_unique<PushBlock>(7,8));
    world_map.put_quiet(std::make_unique<PushBlock>(5,8));
    for (int j = 3; j != 8; ++j) {
        world_map.put_quiet(std::make_unique<Wall>(2,j));
        world_map.put_quiet(std::make_unique<Wall>(j,3));
    }


    world_map.put_quiet(std::make_unique<PushBlock>(12,5,StickyLevel::Strong));
    world_map.put_quiet(std::make_unique<PushBlock>(12,6,StickyLevel::Strong));
    world_map.put_quiet(std::make_unique<PushBlock>(14,5,StickyLevel::Strong));
    world_map.put_quiet(std::make_unique<PushBlock>(14,6,StickyLevel::Strong));

    world_map.put_quiet(std::make_unique<PushBlock>(12,7,StickyLevel::Weak));
    world_map.put_quiet(std::make_unique<PushBlock>(12,8,StickyLevel::Weak));
    world_map.put_quiet(std::make_unique<PushBlock>(12,9,StickyLevel::Weak));
    world_map.put_quiet(std::make_unique<PushBlock>(12,10,StickyLevel::Weak));
    world_map.put_quiet(std::make_unique<PushBlock>(12,11,StickyLevel::Weak));
    world_map.put_quiet(std::make_unique<PushBlock>(12,12,StickyLevel::Weak));
    world_map.put_quiet(std::make_unique<PushBlock>(12,13,StickyLevel::Weak));

    world_map.put_quiet(std::make_unique<PushBlock>(4,10,StickyLevel::Strong));
    world_map.put_quiet(std::make_unique<PushBlock>(4,9,StickyLevel::Strong));
    world_map.put_quiet(std::make_unique<PushBlock>(5,9,StickyLevel::Strong));
    world_map.put_quiet(std::make_unique<PushBlock>(6,9,StickyLevel::Strong));
    world_map.put_quiet(std::make_unique<PushBlock>(7,9,StickyLevel::Strong));
    world_map.put_quiet(std::make_unique<PushBlock>(8,9,StickyLevel::Strong));
    world_map.put_quiet(std::make_unique<PushBlock>(9,9,StickyLevel::Strong));
    world_map.put_quiet(std::make_unique<PushBlock>(8,10,StickyLevel::Strong));

    world_map.put_quiet(std::make_unique<SnakeBlock>(4,11,2));
    world_map.put_quiet(std::make_unique<SnakeBlock>(5,11,2));
    world_map.put_quiet(std::make_unique<SnakeBlock>(6,11,2));
    world_map.put_quiet(std::make_unique<SnakeBlock>(7,11,2));
    world_map.put_quiet(std::make_unique<SnakeBlock>(8,11,2));
    world_map.put_quiet(std::make_unique<SnakeBlock>(9,11,2));
    world_map.put_quiet(std::make_unique<SnakeBlock>(10,11,2)); //*/


    world_map.set_initial_state();

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

    while(!glfwWindowShouldClose(window)) {
        // Handle input

        auto delta_frame = std::make_unique<DeltaFrame>();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
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
        if (cooldown == 0) {
            for (auto p : movement_keys) {
                // For starters, we'll use a naive input processing method.
                // In particular, the precedence of keys is arbitrary
                // and there is no buffering.
                if (glfwGetKey(window, p.first) == GLFW_PRESS) {
                    world_map.move_solid(player_ptr->pos(), p.second, delta_frame.get());
                    cooldown = MAX_COOLDOWN;
                    break;
                }
            }
        }
        // Only allow undo if we didn't just move
        if (cooldown == 0) {
            if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
                undo_stack.pop(&world_map);
                cooldown = MAX_COOLDOWN;
            }
        } else {
            --cooldown;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cam_x = cos(cam_incline) * sin(cam_rotation) * cam_radius;
        cam_y = sin(cam_incline) * cam_radius;
        cam_z = cos(cam_incline) * cos(cam_rotation) * cam_radius;

        view = glm::lookAt(glm::vec3(cam_x, cam_y, cam_z),
                           glm::vec3(0.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(0.5, 0.0, 0.5));
        projection = glm::perspective(glm::radians(60.0f), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100.0f);

        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        world_map.draw(&shader);

        // Draw the floor
        model = glm::translate(glm::mat4(), glm::vec3(-0.5, -0.1, -0.5));
        model = glm::scale(model, glm::vec3(BOARD_SIZE, 0.1, BOARD_SIZE));
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
