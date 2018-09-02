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

#include "worldmap.h"
#include "shader.h"


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

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

    {int i = 0;
    for (int a : {1,2,4}) {
        for (int b : {1,2,4}) {
            if (a == b) continue;
            for (int x : {0, a, a|b, 7, 7 & (~a), 7 & (~(a|b))}) {
                cubeTriangles[i] = x;
                ++i;
            }
        }
    }}

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


    /*glm::vec3 cubePositions[] = {
      glm::vec3( 0.0f,  0.0f,  0.0f),
      glm::vec3( 2.0f,  5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f),
      glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3( 2.4f, -0.4f, -3.5f),
      glm::vec3(-1.7f,  3.0f, -7.5f),
      glm::vec3( 1.3f, -2.0f, -2.5f),
      glm::vec3( 1.5f,  2.0f, -2.5f),
      glm::vec3( 1.5f,  0.2f, -1.5f),
      glm::vec3(-1.3f,  1.0f, -1.5f)
    };*/

    shader.use();

    // Init game logic stuff

    WorldMap world_map(10, 10);
    auto player = std::make_unique<GameObject>(5,5);
    GameObject* player_ptr = player.get();
    world_map.put_quiet(std::move(player));
    int cooldown = 0;

    const glm::vec4 GREEN = glm::vec4(0.6f, 0.9f, 0.7f, 1.0f);
    const glm::vec4 PINK = glm::vec4(0.9f, 0.6f, 0.7f, 1.0f);
    const glm::vec4 YELLOW = glm::vec4(0.9f, 0.9f, 0.4f, 1.0f);

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    while(!glfwWindowShouldClose(window)) {
        // Handle input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        } else if (cooldown == 0) {
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                world_map.move_player(player_ptr, Point {1,0});
                cooldown = 10;
            } else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                world_map.move_player(player_ptr, Point {-1,0});
                cooldown = 10;
            } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
                world_map.move_player(player_ptr, Point {0,-1});
                cooldown = 10;
            } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                world_map.move_player(player_ptr, Point {0,1});
                cooldown = 10;
            }
        } else {
            --cooldown;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float radius = 10.0f;
        float camX = sin(glfwGetTime()/3) * radius;
        float camZ = cos(glfwGetTime()/3) * radius;

        Point player_pos = player_ptr->pos();

        model = glm::translate(glm::mat4(), glm::vec3(player_pos.x - 5.0f, 0.5f, player_pos.y - 5.0f));
        view = glm::lookAt(glm::vec3(camX, 3.0f, camZ),
                           glm::vec3(0.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, 1.0f, 0.0f));
        projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100.0f);

        // Draw the player cube
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);


        if (camX * camZ > 0) {
            shader.setVec4("color", GREEN);
        } else {
            shader.setVec4("color", PINK);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

        // Draw the floor
        model = glm::translate(glm::mat4(), glm::vec3(-0.5, -0.1, -0.5));
        model = glm::scale(model, glm::vec3(10,0.1,10));
        shader.setMat4("model", model);

        shader.setVec4("color", YELLOW);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);


        /*for(int i = 0; i < 10; i++) {
            glm::mat4 model;
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            shader.setMat4("model", model);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
        }*/

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
