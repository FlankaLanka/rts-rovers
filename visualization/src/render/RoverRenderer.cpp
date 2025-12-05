#include "render/RoverRenderer.h"

namespace terrafirma {

// Shader sources
static const char* ROVER_VERT_SRC = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 FragPos;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

static const char* ROVER_FRAG_SRC = R"(
#version 330 core
in vec3 Normal;
in vec3 FragPos;

uniform vec3 color;
uniform float alpha;
uniform bool selected;
uniform bool engineRunning;

out vec4 FragColor;

void main() {
    // Simple lighting
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(Normal), lightDir), 0.0);
    float ambient = 0.3;
    
    vec3 result = color * (ambient + diff * 0.7);
    
    // Dim and desaturate when engine is stopped
    if (!engineRunning) {
        // Convert to grayscale and darken
        float gray = dot(result, vec3(0.299, 0.587, 0.114));
        result = mix(vec3(gray), result, 0.3) * 0.5;  // 70% desaturated, 50% brightness
        result += vec3(0.1, 0.0, 0.0);  // Add slight red tint
    }
    
    // Add glow for selected
    if (selected) {
        result += vec3(0.2, 0.4, 0.5);
    }
    
    FragColor = vec4(result, alpha);
}
)";

RoverRenderer::RoverRenderer() {}

RoverRenderer::~RoverRenderer() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
}

bool RoverRenderer::init() {
    if (!m_shader.loadFromSource(ROVER_VERT_SRC, ROVER_FRAG_SRC)) {
        return false;
    }
    createRoverMesh();
    return true;
}

void RoverRenderer::createRoverMesh() {
    // Create a simple box with a cone on top (for direction)
    // Box: 4m x 3m x 2m (width x depth x height)
    // Cone tip points in +X direction
    
    float hw = 2.0f;  // half width
    float hd = 1.5f;  // half depth
    float hh = 1.0f;  // half height
    
    // Vertices: position (3) + normal (3)
    std::vector<float> vertices = {
        // Bottom face (z = -hh)
        -hw, -hd, -hh,  0, 0, -1,
         hw, -hd, -hh,  0, 0, -1,
         hw,  hd, -hh,  0, 0, -1,
        -hw,  hd, -hh,  0, 0, -1,
        
        // Top face (z = hh)
        -hw, -hd, hh,  0, 0, 1,
         hw, -hd, hh,  0, 0, 1,
         hw,  hd, hh,  0, 0, 1,
        -hw,  hd, hh,  0, 0, 1,
        
        // Front face (x = hw)
        hw, -hd, -hh,  1, 0, 0,
        hw,  hd, -hh,  1, 0, 0,
        hw,  hd,  hh,  1, 0, 0,
        hw, -hd,  hh,  1, 0, 0,
        
        // Back face (x = -hw)
        -hw, -hd, -hh,  -1, 0, 0,
        -hw,  hd, -hh,  -1, 0, 0,
        -hw,  hd,  hh,  -1, 0, 0,
        -hw, -hd,  hh,  -1, 0, 0,
        
        // Right face (y = hd)
        -hw, hd, -hh,  0, 1, 0,
         hw, hd, -hh,  0, 1, 0,
         hw, hd,  hh,  0, 1, 0,
        -hw, hd,  hh,  0, 1, 0,
        
        // Left face (y = -hd)
        -hw, -hd, -hh,  0, -1, 0,
         hw, -hd, -hh,  0, -1, 0,
         hw, -hd,  hh,  0, -1, 0,
        -hw, -hd,  hh,  0, -1, 0,
        
        // Direction cone (pointing in +X)
        // Cone base center at (hw, 0, 0), tip at (hw + 3, 0, 0)
        hw, -1.0f, -0.5f,  0.5f, -0.5f, 0,
        hw,  1.0f, -0.5f,  0.5f,  0.5f, 0,
        hw + 3.0f, 0, 0,   1, 0, 0,
        
        hw,  1.0f, -0.5f,  0.5f, 0.5f, 0,
        hw,  1.0f,  0.5f,  0.5f, 0.5f, 0,
        hw + 3.0f, 0, 0,   1, 0, 0,
        
        hw,  1.0f,  0.5f,  0.5f, 0.5f, 0,
        hw, -1.0f,  0.5f,  0.5f, -0.5f, 0,
        hw + 3.0f, 0, 0,   1, 0, 0,
        
        hw, -1.0f,  0.5f,  0.5f, -0.5f, 0,
        hw, -1.0f, -0.5f,  0.5f, -0.5f, 0,
        hw + 3.0f, 0, 0,   1, 0, 0,
    };

    std::vector<unsigned int> indices;
    // Box faces
    for (int face = 0; face < 6; face++) {
        unsigned int base = face * 4;
        indices.push_back(base);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }
    // Cone triangles (already in triangle form)
    for (int i = 0; i < 12; i++) {
        indices.push_back(24 + i);
    }

    m_indexCount = indices.size();

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void RoverRenderer::render(const RoverState& rover, const glm::vec3& color, bool selected,
                           bool engineRunning, const glm::mat4& view, const glm::mat4& projection) {
    m_shader.use();
    
    glm::mat4 model = rover.getModelMatrix();
    
    m_shader.setMat4("model", model);
    m_shader.setMat4("view", view);
    m_shader.setMat4("projection", projection);
    m_shader.setVec3("color", color);
    m_shader.setFloat("alpha", rover.online ? 1.0f : 0.5f);
    m_shader.setInt("selected", selected ? 1 : 0);
    m_shader.setInt("engineRunning", engineRunning ? 1 : 0);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

} // namespace terrafirma

