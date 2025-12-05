#include "render/PointCloudRenderer.h"

namespace terrafirma {

static const char* POINT_VERT_SRC = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform float minHeight;
uniform float maxHeight;
uniform float pointSize;

out float heightNorm;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = pointSize;
    
    // Y is height in our coordinate system
    float range = maxHeight - minHeight;
    if (range > 0.001) {
        heightNorm = clamp((aPos.y - minHeight) / range, 0.0, 1.0);
    } else {
        heightNorm = 0.5;
    }
}
)";

static const char* POINT_FRAG_SRC = R"(
#version 330 core
in float heightNorm;

uniform bool useHeightColor;

out vec4 FragColor;

vec3 heightToColor(float h) {
    // Blue -> Cyan -> Green -> Yellow -> Red
    if (h < 0.25) {
        float t = h / 0.25;
        return mix(vec3(0.0, 0.4, 1.0), vec3(0.0, 1.0, 1.0), t);
    } else if (h < 0.5) {
        float t = (h - 0.25) / 0.25;
        return mix(vec3(0.0, 1.0, 1.0), vec3(0.0, 1.0, 0.4), t);
    } else if (h < 0.75) {
        float t = (h - 0.5) / 0.25;
        return mix(vec3(0.0, 1.0, 0.4), vec3(1.0, 1.0, 0.0), t);
    } else {
        float t = (h - 0.75) / 0.25;
        return mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.27, 0.0), t);
    }
}

void main() {
    vec3 color;
    if (useHeightColor) {
        color = heightToColor(heightNorm);
    } else {
        color = vec3(0.0, 1.0, 1.0); // Cyan
    }
    FragColor = vec4(color, 1.0);
}
)";

PointCloudRenderer::PointCloudRenderer() {}

PointCloudRenderer::~PointCloudRenderer() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

bool PointCloudRenderer::init() {
    if (!m_shader.loadFromSource(POINT_VERT_SRC, POINT_FRAG_SRC)) {
        return false;
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    
    // Pre-allocate GPU buffer
    m_gpuBufferCapacity = INITIAL_CAPACITY;
    glBufferData(GL_ARRAY_BUFFER, m_gpuBufferCapacity * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return true;
}

void PointCloudRenderer::render(PointCloud& cloud, const RenderSettings& settings,
                                const glm::mat4& view, const glm::mat4& projection) {
    const glm::vec3* data = nullptr;
    size_t totalCount = 0;
    float minH, maxH;
    
    // Get new points (incremental)
    size_t newPoints = cloud.getNewPointsForRendering(&data, &totalCount, &minH, &maxH);
    
    if (data && totalCount > 0) {
        m_minHeight = minH;
        m_maxHeight = maxH;
        
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        
        // Check if we need to expand buffer
        if (totalCount > m_gpuBufferCapacity) {
            m_gpuBufferCapacity = totalCount * 2;
            glBufferData(GL_ARRAY_BUFFER, m_gpuBufferCapacity * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
            // Need to re-upload all data after resize
            glBufferSubData(GL_ARRAY_BUFFER, 0, totalCount * sizeof(glm::vec3), data);
        } else if (newPoints > 0) {
            // Only upload new points (incremental update)
            size_t offset = (totalCount - newPoints) * sizeof(glm::vec3);
            glBufferSubData(GL_ARRAY_BUFFER, offset, newPoints * sizeof(glm::vec3), data + (totalCount - newPoints));
        }
        
        m_gpuPointCount = totalCount;
    }
    
    // Render
    if (m_gpuPointCount == 0) return;

    m_shader.use();
    m_shader.setMat4("view", view);
    m_shader.setMat4("projection", projection);
    m_shader.setFloat("minHeight", m_minHeight);
    m_shader.setFloat("maxHeight", m_maxHeight);
    m_shader.setFloat("pointSize", settings.pointSize);
    m_shader.setInt("useHeightColor", settings.pointCloudHeightColors ? 1 : 0);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_gpuPointCount));
    glBindVertexArray(0);
}

} // namespace terrafirma
