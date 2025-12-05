#pragma once

#include "common.h"
#include "render/Shader.h"
#include "data/PointCloud.h"
#include <glad/glad.h>

namespace terrafirma {

class PointCloudRenderer {
public:
    PointCloudRenderer();
    ~PointCloudRenderer();

    bool init();
    void render(PointCloud& cloud, const RenderSettings& settings,
                const glm::mat4& view, const glm::mat4& projection);

private:
    Shader m_shader;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    
    size_t m_gpuPointCount = 0;
    size_t m_gpuBufferCapacity = 0;
    float m_minHeight = 0.0f;
    float m_maxHeight = 100.0f;
    
    static constexpr size_t INITIAL_CAPACITY = 1000000; // 1M points
};

} // namespace terrafirma
