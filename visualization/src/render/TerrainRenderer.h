#pragma once

#include "common.h"
#include "render/Shader.h"
#include "data/DataManager.h"
#include <glad/glad.h>

namespace terrafirma {

class TerrainRenderer {
public:
    TerrainRenderer();
    ~TerrainRenderer();

    bool init();
    void render(TerrainGrid& terrain, const RenderSettings& settings,
                const glm::mat4& view, const glm::mat4& projection);

private:
    void updateMesh(const TerrainGrid& terrain);

    Shader m_shader;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    size_t m_indexCount = 0;
    float m_minHeight = 0.0f;
    float m_maxHeight = 1.0f;
};

} // namespace terrafirma

