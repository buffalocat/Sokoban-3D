#include "graphicsmanager.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wswitch-default"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma GCC diagnostic pop

#include "shader.h"

GraphicsManager::GraphicsManager(GLFWwindow* window):
window_ {window},
shader_ {Shader("shaders\\shader.vs", "shaders\\shader.fs")} {
    init_cube_buffer();
    init_vertex_attributes();
    load_texture_atlas();
    shader_.use();
    glEnable(GL_DEPTH_TEST);
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
    float cubeVertices[8*STRIDE*TEXTURE_ATLAS_SIZE*TEXTURE_ATLAS_SIZE];
    for (int u = 0; u < TEXTURE_ATLAS_SIZE; ++u) {
        for (int v = 0; v < TEXTURE_ATLAS_SIZE; ++v) {
            int offset = 8*STRIDE*(u + TEXTURE_ATLAS_SIZE*v);
            for (int i = 0; i < 8; ++i) {
                int a, b, c;
                a = i & 1;
                b = (i >> 1) & 1;
                c = (i >> 2) & 1;
                cubeVertices[offset + STRIDE*i] = a - 0.5f;
                cubeVertices[offset + STRIDE*i+1] = b - 0.5f;
                cubeVertices[offset + STRIDE*i+2] = c - 0.5f;
                cubeVertices[offset + STRIDE*i+3] = ((i == 0 || i == 7) ? u+1 : u)/((float)TEXTURE_ATLAS_SIZE);
                cubeVertices[offset + STRIDE*i+4] = ((a + b + c > 1) ? v+1 : v)/((float)TEXTURE_ATLAS_SIZE);
            }
        }
    }

    int cubeTriangles[36*TEXTURE_ATLAS_SIZE*TEXTURE_ATLAS_SIZE];

    int i = 0;
    for (int u = 0; u < TEXTURE_ATLAS_SIZE; ++u) {
        for (int v = 0; v < TEXTURE_ATLAS_SIZE; ++v) {
            int offset = 36*(u + TEXTURE_ATLAS_SIZE*v);
            for (int a : {1,2,4}) {
                for (int b : {1,2,4}) {
                    if (a == b) continue;
                    for (int x : {0, a, a|b, 7, 7 & (~a), 7 & (~(a|b))}) {
                        cubeTriangles[i] = x + offset;
                        ++i;
                    }
                }
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

void GraphicsManager::draw_cube(Texture t) {
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)((int)t * 36 * sizeof(int)));
}
