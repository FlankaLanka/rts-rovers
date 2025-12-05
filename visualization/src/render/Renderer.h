#pragma once

#include "common.h"
#include "render/Shader.h"
#include "render/RoverRenderer.h"
#include "render/PointCloudRenderer.h"
#include "render/TerrainRenderer.h"
#include "data/PointCloud.h"
#include "data/DataManager.h"
#include <memory>
#include <array>

namespace terrafirma {

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init();
    
    void begin(const glm::mat4& view, const glm::mat4& projection);
    void end();
    
    void renderRover(const RoverState& rover, const glm::vec3& color, bool selected, bool engineRunning);
    void renderPointCloud(int roverIndex, PointCloud& cloud, const RenderSettings& settings);
    void renderTerrain(TerrainGrid& terrain, const RenderSettings& settings);

private:
    glm::mat4 m_view;
    glm::mat4 m_projection;

    std::unique_ptr<RoverRenderer> m_roverRenderer;
    // Separate point cloud renderer per rover to avoid GPU buffer conflicts
    std::array<std::unique_ptr<PointCloudRenderer>, NUM_ROVERS> m_pointCloudRenderers;
    std::unique_ptr<TerrainRenderer> m_terrainRenderer;
};

} // namespace terrafirma
