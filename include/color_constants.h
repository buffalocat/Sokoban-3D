#ifndef COLOR_CONSTANTS_H_INCLUDED
#define COLOR_CONSTANTS_H_INCLUDED

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma GCC diagnostic pop

const glm::vec4 COLORS[] = {
    glm::vec4(0.6f, 0.9f, 0.7f, 1.0f),
    glm::vec4(0.9f, 0.6f, 0.7f, 1.0f),
    glm::vec4(0.7f, 0.5f, 0.9f, 1.0f),
    glm::vec4(0.3f, 0.2f, 0.6f, 1.0f),
    glm::vec4(0.0f, 0.3f, 0.8f, 1.0f),
    glm::vec4(1.0f, 0.5f, 0.5f, 1.0f),
    glm::vec4(0.6f, 0.0f, 0.1f, 1.0f),
    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
    glm::vec4(0.7f, 0.7f, 0.7f, 1.0f),
    glm::vec4(1.0f, 0.7f, 0.3f, 1.0f),
    glm::vec4(0.7f, 0.7f, 0.3f, 1.0f),
    glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
    glm::vec4(0.1f, 0.1f, 0.1f, 1.0f),
};

const int NUM_GREYS = 9;

const glm::vec4 GREYS[NUM_GREYS] = {
    glm::vec4(0.1f, 0.1f, 0.1f, 1.0f),
    glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
    glm::vec4(0.3f, 0.3f, 0.3f, 1.0f),
    glm::vec4(0.4f, 0.4f, 0.4f, 1.0f),
    glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
    glm::vec4(0.6f, 0.6f, 0.6f, 1.0f),
    glm::vec4(0.7f, 0.7f, 0.7f, 1.0f),
    glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
    glm::vec4(0.9f, 0.9f, 0.9f, 1.0f),
};

#endif // COLOR_CONSTANTS_H_INCLUDED
