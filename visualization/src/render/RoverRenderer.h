#pragma once

#include "common.h"
#include "render/Shader.h"
#include <glad/glad.h>

namespace terrafirma {

class RoverRenderer {
public:
    RoverRenderer();
    ~RoverRenderer();

    bool init();
    void render(const RoverState& rover, const glm::vec3& color, bool selected,
                bool engineRunning, const glm::mat4& view, const glm::mat4& projection);

private:
    void createRoverMesh();

    Shader m_shader;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    size_t m_indexCount = 0;
};

} // namespace terrafirma

