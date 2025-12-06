#pragma once

#include "common.h"
#include "render/Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

namespace terrafirma {

class PathRenderer {
public:
    PathRenderer();
    ~PathRenderer();
    
    bool init();
    
    // Render a path as a colored line strip
    void renderPath(
        const std::vector<glm::vec3>& path,
        const glm::vec3& color,
        float lineWidth,
        const glm::mat4& view,
        const glm::mat4& projection
    );
    
    // Render a destination marker (beacon/cylinder)
    void renderMarker(
        const glm::vec3& position,
        const glm::vec3& color,
        float height,
        const glm::mat4& view,
        const glm::mat4& projection
    );
    
private:
    void createMarkerMesh();
    
    Shader m_pathShader;
    Shader m_markerShader;
    
    // Path rendering
    GLuint m_pathVAO = 0;
    GLuint m_pathVBO = 0;
    
    // Marker rendering
    GLuint m_markerVAO = 0;
    GLuint m_markerVBO = 0;
    GLuint m_markerEBO = 0;
    size_t m_markerIndexCount = 0;
};

} // namespace terrafirma

