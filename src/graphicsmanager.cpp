#include "graphicsmanager.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#pragma GCC diagnostic pop

GraphicsManager::GraphicsManager(GLFWwindow* window):
window_ {window},
shader_ {Shader("shaders\\shader.vs", "shaders\\shader.fs")} {
    init_cube_buffer();
    init_vertex_attributes();
    load_texture_atlas();
    shader_.use();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

GLFWwindow* GraphicsManager::window() {
    return window_;
}

const int STRIDE = 5;

void GraphicsManager::init_vertex_attributes() {
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*) nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, STRIDE * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

// The atlas is a square of 2^k * 2^k square textures
const int TEXTURE_ATLAS_SIZE = 4;

void GraphicsManager::init_cube_buffer() {
    float cubeVertices[8*STRIDE];
    for (int i = 0; i < 8; ++i) {
        int a, b, c;
        a = i & 1;
        b = (i >> 1) & 1;
        c = (i >> 2) & 1;
        cubeVertices[STRIDE*i] = a - 0.5f;
        cubeVertices[STRIDE*i+1] = b - 0.5f;
        cubeVertices[STRIDE*i+2] = c - 0.5f;
        cubeVertices[STRIDE*i+3] = (i == 0 || i == 7) ? 1 : 0;
        cubeVertices[STRIDE*i+4] = (a + b + c > 1) ? 1 : 0;
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

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeTriangles), cubeTriangles, GL_STATIC_DRAW);
}

void GraphicsManager::load_texture_atlas() {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, channels;
    unsigned char *texture_data = stbi_load("resources\\textures.png", &width, &height, &channels, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
    stbi_image_free(texture_data);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"

void GraphicsManager::set_model(glm::mat4 model) {
    if (model != model_) {
        model_ = model;
        shader_.setMat4("model", model);
    }
}

void GraphicsManager::set_view(glm::mat4 view) {
    if (view != view_) {
        view_ = view;
        shader_.setMat4("view", view);
    }
}

void GraphicsManager::set_projection(glm::mat4 projection) {
    if (projection != projection_) {
        projection_ = projection;
        shader_.setMat4("projection", projection);
    }
}

void GraphicsManager::set_color(glm::vec4 color) {
    if (color != color_) {
        color_ = color;
        shader_.setVec4("color", color);
    }
}

void GraphicsManager::set_tex(Texture tex) {
    if (tex != tex_) {
        tex_ = tex;
        float u = (int)tex % TEXTURE_ATLAS_SIZE;
        float v = (int)tex / TEXTURE_ATLAS_SIZE;
        shader_.setVec2("tex", glm::vec2(u,v));
    }
}

#pragma GCC diagnostic pop

void GraphicsManager::draw_cube() {
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}
