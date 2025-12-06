#include "pathfinding/PathRenderer.h"
#include <cmath>

namespace terrafirma {

// Simple shader for path lines
static const char* PATH_VERT_SRC = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";

static const char* PATH_FRAG_SRC = R"(
#version 330 core
uniform vec3 color;
out vec4 FragColor;

void main() {
    FragColor = vec4(color, 0.9);
}
)";

// Shader for marker with simple lighting
static const char* MARKER_VERT_SRC = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;

void main() {
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

static const char* MARKER_FRAG_SRC = R"(
#version 330 core
in vec3 Normal;

uniform vec3 color;

out vec4 FragColor;

void main() {
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(normalize(Normal), lightDir), 0.0);
    float ambient = 0.4;
    
    vec3 result = color * (ambient + diff * 0.6);
    FragColor = vec4(result, 0.85);
}
)";

PathRenderer::PathRenderer() {}

PathRenderer::~PathRenderer() {
    if (m_pathVAO) glDeleteVertexArrays(1, &m_pathVAO);
    if (m_pathVBO) glDeleteBuffers(1, &m_pathVBO);
    if (m_markerVAO) glDeleteVertexArrays(1, &m_markerVAO);
    if (m_markerVBO) glDeleteBuffers(1, &m_markerVBO);
    if (m_markerEBO) glDeleteBuffers(1, &m_markerEBO);
}

bool PathRenderer::init() {
    if (!m_pathShader.loadFromSource(PATH_VERT_SRC, PATH_FRAG_SRC)) {
        return false;
    }
    if (!m_markerShader.loadFromSource(MARKER_VERT_SRC, MARKER_FRAG_SRC)) {
        return false;
    }
    
    // Create path VAO/VBO (dynamic, will be updated each frame)
    glGenVertexArrays(1, &m_pathVAO);
    glGenBuffers(1, &m_pathVBO);
    
    glBindVertexArray(m_pathVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_pathVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    // Create marker mesh
    createMarkerMesh();
    
    return true;
}

void PathRenderer::createMarkerMesh() {
    // Create a cylinder/beacon shape
    const int segments = 12;
    const float radius = 0.5f;
    const float height = 1.0f;
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Bottom circle center
    vertices.insert(vertices.end(), {0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f});
    
    // Bottom circle vertices
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * 3.14159f * i / segments;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        vertices.insert(vertices.end(), {x, 0.0f, z, 0.0f, -1.0f, 0.0f});
    }
    
    // Bottom circle triangles
    for (int i = 0; i < segments; i++) {
        indices.push_back(0);
        indices.push_back(1 + i);
        indices.push_back(1 + (i + 1) % segments);
    }
    
    // Top circle center
    int topCenterIdx = static_cast<int>(vertices.size() / 6);
    vertices.insert(vertices.end(), {0.0f, height, 0.0f, 0.0f, 1.0f, 0.0f});
    
    // Top circle vertices
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * 3.14159f * i / segments;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        vertices.insert(vertices.end(), {x, height, z, 0.0f, 1.0f, 0.0f});
    }
    
    // Top circle triangles
    for (int i = 0; i < segments; i++) {
        indices.push_back(topCenterIdx);
        indices.push_back(topCenterIdx + 1 + (i + 1) % segments);
        indices.push_back(topCenterIdx + 1 + i);
    }
    
    // Side walls
    int sideBaseIdx = static_cast<int>(vertices.size() / 6);
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * 3.14159f * i / segments;
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        float nx = std::cos(angle);
        float nz = std::sin(angle);
        
        // Bottom vertex
        vertices.insert(vertices.end(), {x, 0.0f, z, nx, 0.0f, nz});
        // Top vertex
        vertices.insert(vertices.end(), {x, height, z, nx, 0.0f, nz});
    }
    
    // Side triangles
    for (int i = 0; i < segments; i++) {
        int curr = sideBaseIdx + i * 2;
        int next = sideBaseIdx + ((i + 1) % segments) * 2;
        
        indices.push_back(curr);
        indices.push_back(next);
        indices.push_back(curr + 1);
        
        indices.push_back(next);
        indices.push_back(next + 1);
        indices.push_back(curr + 1);
    }
    
    m_markerIndexCount = indices.size();
    
    glGenVertexArrays(1, &m_markerVAO);
    glGenBuffers(1, &m_markerVBO);
    glGenBuffers(1, &m_markerEBO);
    
    glBindVertexArray(m_markerVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_markerVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_markerEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void PathRenderer::renderPath(
    const std::vector<glm::vec3>& path,
    const glm::vec3& color,
    float lineWidth,
    const glm::mat4& view,
    const glm::mat4& projection
) {
    if (path.size() < 2) return;
    
    // Prepare vertex data with height offset
    std::vector<float> vertices;
    for (const auto& p : path) {
        vertices.push_back(p.x);
        vertices.push_back(p.y + 0.5f);  // Slight offset above terrain
        vertices.push_back(p.z);
    }
    
    // Update VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_pathVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    
    // Render
    m_pathShader.use();
    m_pathShader.setMat4("view", view);
    m_pathShader.setMat4("projection", projection);
    m_pathShader.setVec3("color", color);
    
    glLineWidth(lineWidth);
    glBindVertexArray(m_pathVAO);
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(path.size()));
    glBindVertexArray(0);
    glLineWidth(1.0f);
}

void PathRenderer::renderMarker(
    const glm::vec3& position,
    const glm::vec3& color,
    float height,
    const glm::mat4& view,
    const glm::mat4& projection
) {
    m_markerShader.use();
    
    // Create model matrix with position and scale
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position + glm::vec3(0.0f, 0.5f, 0.0f));  // Offset above terrain
    model = glm::scale(model, glm::vec3(1.5f, height, 1.5f));
    
    m_markerShader.setMat4("model", model);
    m_markerShader.setMat4("view", view);
    m_markerShader.setMat4("projection", projection);
    m_markerShader.setVec3("color", color);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindVertexArray(m_markerVAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_markerIndexCount), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    glDisable(GL_BLEND);
}

} // namespace terrafirma

