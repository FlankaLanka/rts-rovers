#include "render/CircleRenderer.h"
#include "terrain/TerrainRaycast.h"
#include <cmath>
#include <vector>

namespace terrafirma {

static const char* CIRCLE_VERT_SRC = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";

static const char* CIRCLE_FRAG_SRC = R"(
#version 330 core
uniform vec4 color;

out vec4 FragColor;

void main() {
    FragColor = color;
}
)";

CircleRenderer::CircleRenderer() {}

CircleRenderer::~CircleRenderer() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

bool CircleRenderer::init() {
    if (!m_shader.loadFromSource(CIRCLE_VERT_SRC, CIRCLE_FRAG_SRC)) {
        return false;
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    
    // Pre-allocate buffer for triangle fan (center + segments + 1 to close)
    size_t maxVertices = CIRCLE_SEGMENTS + 2;
    glBufferData(GL_ARRAY_BUFFER, maxVertices * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return true;
}

void CircleRenderer::updateMesh(const glm::vec2& center, float radius, const TerrainGrid& terrain) {
    std::vector<glm::vec3> vertices;
    vertices.reserve(CIRCLE_SEGMENTS + 2);
    
    // Get center height
    float centerHeight = 0.0f;
    if (!getTerrainHeightAt(terrain, center.x, center.y, centerHeight)) {
        // No terrain data at center, use min height as fallback
        centerHeight = terrain.getMinHeight();
    }
    
    // Center vertex (slightly above terrain to prevent z-fighting)
    vertices.push_back(glm::vec3(center.x, centerHeight + 0.1f, center.y));
    
    // Edge vertices
    for (int i = 0; i <= CIRCLE_SEGMENTS; i++) {
        float angle = (2.0f * 3.14159265f * i) / CIRCLE_SEGMENTS;
        float x = center.x + radius * std::cos(angle);
        float z = center.y + radius * std::sin(angle);
        
        // Get terrain height at this point
        float height = centerHeight;
        getTerrainHeightAt(terrain, x, z, height);
        
        // Slightly above terrain
        vertices.push_back(glm::vec3(x, height + 0.1f, z));
    }
    
    m_vertexCount = vertices.size();
    
    // Upload to GPU
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), vertices.data());
}

void CircleRenderer::render(const glm::vec2& center, float radius, const glm::vec4& color,
                            const TerrainGrid& terrain,
                            const glm::mat4& view, const glm::mat4& projection) {
    if (radius <= 0.0f) return;
    
    // Update mesh vertices based on terrain
    updateMesh(center, radius, terrain);
    
    if (m_vertexCount == 0) return;
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Disable depth writing but keep depth test (so circle appears on terrain)
    glDepthMask(GL_FALSE);
    
    m_shader.use();
    m_shader.setMat4("view", view);
    m_shader.setMat4("projection", projection);
    m_shader.setVec4("color", color);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(m_vertexCount));
    glBindVertexArray(0);
    
    // Restore state
    glDepthMask(GL_TRUE);
}

} // namespace terrafirma

