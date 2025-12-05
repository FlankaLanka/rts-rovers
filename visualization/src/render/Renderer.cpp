#include "render/Renderer.h"
#include <iostream>

namespace terrafirma {

Renderer::Renderer() {}

Renderer::~Renderer() {}

bool Renderer::init() {
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable point size
    glEnable(GL_PROGRAM_POINT_SIZE);

    m_roverRenderer = std::make_unique<RoverRenderer>();
    m_pointCloudRenderer = std::make_unique<PointCloudRenderer>();
    m_terrainRenderer = std::make_unique<TerrainRenderer>();

    if (!m_roverRenderer->init()) {
        std::cerr << "Failed to initialize rover renderer\n";
        return false;
    }
    
    if (!m_pointCloudRenderer->init()) {
        std::cerr << "Failed to initialize point cloud renderer\n";
        return false;
    }
    
    if (!m_terrainRenderer->init()) {
        std::cerr << "Failed to initialize terrain renderer\n";
        return false;
    }

    return true;
}

void Renderer::begin(const glm::mat4& view, const glm::mat4& projection) {
    m_view = view;
    m_projection = projection;
}

void Renderer::end() {
    // Nothing specific needed
}

void Renderer::renderRover(const RoverState& rover, const glm::vec3& color, bool selected) {
    m_roverRenderer->render(rover, color, selected, m_view, m_projection);
}

void Renderer::renderPointCloud(PointCloud& cloud, const RenderSettings& settings) {
    m_pointCloudRenderer->render(cloud, settings, m_view, m_projection);
}

void Renderer::renderTerrain(TerrainGrid& terrain, const RenderSettings& settings) {
    m_terrainRenderer->render(terrain, settings, m_view, m_projection);
}

} // namespace terrafirma

