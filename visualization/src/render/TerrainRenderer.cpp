#include "render/TerrainRenderer.h"
#include <vector>
#include <map>
#include <algorithm>

namespace terrafirma {

static const char* TERRAIN_VERT_SRC = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 view;
uniform mat4 projection;
uniform float minHeight;
uniform float maxHeight;

out vec3 Normal;
out float heightNorm;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    Normal = aNormal;
    
    // Y is height in our coordinate system
    float range = maxHeight - minHeight;
    if (range > 0.001) {
        heightNorm = clamp((aPos.y - minHeight) / range, 0.0, 1.0);
    } else {
        heightNorm = 0.5;
    }
}
)";

static const char* TERRAIN_FRAG_SRC = R"(
#version 330 core
in vec3 Normal;
in float heightNorm;

uniform bool useHeightColor;
uniform bool wireframeMode;

out vec4 FragColor;

vec3 heightToColor(float h) {
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
        color = vec3(0.2, 0.3, 0.4);
    }
    
    // Simple lighting
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(Normal), lightDir), 0.0);
    float ambient = 0.3;
    
    if (wireframeMode) {
        FragColor = vec4(color, 1.0);
    } else {
        vec3 result = color * (ambient + diff * 0.7);
        FragColor = vec4(result, 0.8);
    }
}
)";

TerrainRenderer::TerrainRenderer() {}

TerrainRenderer::~TerrainRenderer() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
}

bool TerrainRenderer::init() {
    if (!m_shader.loadFromSource(TERRAIN_VERT_SRC, TERRAIN_FRAG_SRC)) {
        return false;
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    return true;
}

void TerrainRenderer::updateMesh(const TerrainGrid& terrain) {
    // IMPORTANT: Copy the cells map to avoid race condition with network thread
    // The network thread may be modifying the map while we iterate
    const auto cells = terrain.getCells();  // Copy, not reference!
    if (cells.empty()) return;

    float cellSize = terrain.getCellSize();
    m_minHeight = terrain.getMinHeight();
    m_maxHeight = terrain.getMaxHeight();

    // Create a vertex for each cell
    // Coordinate system: X=horizontal, Y=height, Z=horizontal
    std::map<std::pair<int, int>, unsigned int> vertexIndices;
    std::vector<float> vertices; // pos (3) + normal (3)
    std::vector<unsigned int> indices;

    unsigned int idx = 0;
    for (const auto& [key, height] : cells) {
        float x = key.first * cellSize;   // X position
        float z = key.second * cellSize;  // Z position
        float y = height;                  // Y is height
        
        // Normal pointing up (Y direction)
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(0);
        vertices.push_back(1);  // Normal points up in Y
        vertices.push_back(0);
        
        vertexIndices[key] = idx++;
    }

    // Create triangles for adjacent cells
    // ONLY create triangles when ALL 4 corners of a quad exist
    for (const auto& [key, height] : cells) {
        (void)height;
        int cx = key.first;
        int cz = key.second;
        
        // Check for neighbors to form quads
        auto right = std::make_pair(cx + 1, cz);
        auto forward = std::make_pair(cx, cz + 1);
        auto rightForward = std::make_pair(cx + 1, cz + 1);
        
        bool hasRight = vertexIndices.find(right) != vertexIndices.end();
        bool hasForward = vertexIndices.find(forward) != vertexIndices.end();
        bool hasRightForward = vertexIndices.find(rightForward) != vertexIndices.end();
        
        // Only form triangles if we have ALL FOUR corners (complete quad)
        if (hasRight && hasForward && hasRightForward) {
            // Two triangles forming a quad
            indices.push_back(vertexIndices[key]);
            indices.push_back(vertexIndices[right]);
            indices.push_back(vertexIndices[rightForward]);
            
            indices.push_back(vertexIndices[key]);
            indices.push_back(vertexIndices[rightForward]);
            indices.push_back(vertexIndices[forward]);
        }
    }

    m_indexCount = indices.size();
    if (m_indexCount == 0) return;

    glBindVertexArray(m_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void TerrainRenderer::render(TerrainGrid& terrain, const RenderSettings& settings,
                             const glm::mat4& view, const glm::mat4& projection) {
    if (terrain.getCells().empty()) return;
    
    if (terrain.isDirty()) {
        updateMesh(terrain);
        terrain.clearDirty();
    }
    
    if (m_indexCount == 0) return;

    m_shader.use();
    m_shader.setMat4("view", view);
    m_shader.setMat4("projection", projection);
    m_shader.setFloat("minHeight", m_minHeight);
    m_shader.setFloat("maxHeight", m_maxHeight);
    m_shader.setInt("useHeightColor", settings.terrainHeightColors ? 1 : 0);

    glBindVertexArray(m_vao);

    // Render solid first (with polygon offset to prevent z-fighting)
    if (settings.terrainSolid) {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f); // Push solid surfaces slightly back
        m_shader.setInt("wireframeMode", 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, 0);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    // Render wireframe on top
    if (settings.terrainWireframe) {
        m_shader.setInt("wireframeMode", 1);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, 0);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(0);
}

} // namespace terrafirma

