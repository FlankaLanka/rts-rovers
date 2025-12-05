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
    m_terrainRenderer = std::make_unique<TerrainRenderer>();

    // Create separate point cloud renderer for each rover
    for (int i = 0; i < NUM_ROVERS; i++) {
        m_pointCloudRenderers[i] = std::make_unique<PointCloudRenderer>();
    }

    if (!m_roverRenderer->init()) {
        std::cerr << "Failed to initialize rover renderer\n";
        return false;
    }
    
    // Initialize all point cloud renderers
    for (int i = 0; i < NUM_ROVERS; i++) {
        if (!m_pointCloudRenderers[i]->init()) {
            std::cerr << "Failed to initialize point cloud renderer for rover " << (i+1) << "\n";
            return false;
        }
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

void Renderer::renderRover(const RoverState& rover, const glm::vec3& color, bool selected, bool engineRunning) {
    m_roverRenderer->render(rover, color, selected, engineRunning, m_view, m_projection);
}

void Renderer::renderPointCloud(int roverIndex, PointCloud& cloud, const RenderSettings& settings) {
    if (roverIndex >= 0 && roverIndex < NUM_ROVERS) {
        m_pointCloudRenderers[roverIndex]->render(cloud, settings, m_view, m_projection);
    }
}

void Renderer::renderTerrain(TerrainGrid& terrain, const RenderSettings& settings) {
    m_terrainRenderer->render(terrain, settings, m_view, m_projection);
}

} // namespace terrafirma
