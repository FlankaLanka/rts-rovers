#pragma once

#include "common.h"
#include "render/Shader.h"
#include "render/RoverRenderer.h"
#include "render/PointCloudRenderer.h"
#include "render/TerrainRenderer.h"
#include "data/PointCloud.h"
#include "data/DataManager.h"
#include <memory>

namespace terrafirma {

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init();
    
    void begin(const glm::mat4& view, const glm::mat4& projection);
    void end();
    
    void renderRover(const RoverState& rover, const glm::vec3& color, bool selected);
    void renderPointCloud(PointCloud& cloud, const RenderSettings& settings);
    void renderTerrain(TerrainGrid& terrain, const RenderSettings& settings);

private:
    glm::mat4 m_view;
    glm::mat4 m_projection;

    std::unique_ptr<RoverRenderer> m_roverRenderer;
    std::unique_ptr<PointCloudRenderer> m_pointCloudRenderer;
    std::unique_ptr<TerrainRenderer> m_terrainRenderer;
};

} // namespace terrafirma

