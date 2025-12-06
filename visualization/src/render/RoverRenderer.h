#pragma once

#include "common.h"
#include "render/Shader.h"
#include <glad/glad.h>
#include <string>

namespace terrafirma {

class RoverRenderer {
public:
    RoverRenderer();
    ~RoverRenderer();

    bool init();
    void render(const RoverState& rover, const glm::vec3& color, bool selected,
                bool engineRunning, const glm::mat4& view, const glm::mat4& projection);

private:
    bool loadModel();  // Tries to load any supported format
    bool loadOBJ(const std::string& path);
    bool loadGLTF(const std::string& path);  // Handles both .gltf and .glb
    void createFallbackMesh();  // Used if no model file found

    Shader m_shader;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    size_t m_indexCount = 0;
    
    bool m_modelLoaded = false;
    float m_modelScale = 1.0f;  // Scale factor for the model
};

} // namespace terrafirma
