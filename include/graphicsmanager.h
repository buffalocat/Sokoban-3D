#ifndef GRAPHICSMANAGER_H
#define GRAPHICSMANAGER_H

#include "common.h"
#include "shader.h"

class GLFWwindow;

// Reading off the texture atlas starting from top, left to right
enum class Texture {
    Blank,
    Corners,
    Edges,
    UNUSED_A,
    Cross,
    SwitchUp,
    SwitchDown,
};

class GraphicsManager {
public:
    GraphicsManager(GLFWwindow*);
    GLFWwindow* window();

    void set_model(glm::mat4);
    void set_view(glm::mat4);
    void set_projection(glm::mat4);
    void set_color(glm::vec4);
    void set_tex(glm::vec2);

    void draw_cube();
    //void draw_trail();

private:
    GLFWwindow* window_;
    Shader shader_;

    glm::mat4 model_;
    glm::mat4 view_;
    glm::mat4 projection_;

    glm::vec4 color_;
    glm::vec2 tex_;

    void init_vertex_attributes();
    void init_cube_buffer();
    void load_texture_atlas();
};

#endif // GRAPHICSMANAGER_H
