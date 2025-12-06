#pragma once

#include "common.h"
#include "render/Shader.h"
#include "data/DataManager.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace terrafirma {

class CircleRenderer {
public:
    CircleRenderer();
    ~CircleRenderer();

    bool init();
    
    // Render a filled circle on the terrain
    // center: XZ position of circle center
    // radius: circle radius in world units
    // color: RGBA color (use alpha < 1 for transparency)
    void render(const glm::vec2& center, float radius, const glm::vec4& color,
                const TerrainGrid& terrain,
                const glm::mat4& view, const glm::mat4& projection);

private:
    void updateMesh(const glm::vec2& center, float radius, const TerrainGrid& terrain);

    Shader m_shader;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    
    static constexpr int CIRCLE_SEGMENTS = 32;
    size_t m_vertexCount = 0;
};

} // namespace terrafirma

