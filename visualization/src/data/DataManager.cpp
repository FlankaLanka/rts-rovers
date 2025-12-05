#include "data/DataManager.h"
#include <GLFW/glfw3.h>

namespace terrafirma {

// TerrainGrid implementation
TerrainGrid::TerrainGrid(float cellSize) : m_cellSize(cellSize) {}

void TerrainGrid::addPoint(const glm::vec3& point) {
    // X and Z are horizontal, Y is height
    int cx = static_cast<int>(std::floor(point.x / m_cellSize));
    int cz = static_cast<int>(std::floor(point.z / m_cellSize));
    auto key = std::make_pair(cx, cz);
    
    auto it = m_cells.find(key);
    if (it == m_cells.end()) {
        m_cells[key] = point.y;  // Y is height
    } else {
        // Use maximum height
        it->second = std::max(it->second, point.y);
    }
    
    // Update height range (Y is height)
    if (m_cells.size() == 1) {
        m_minHeight = point.y;
        m_maxHeight = point.y;
    } else {
        m_minHeight = std::min(m_minHeight, point.y);
        m_maxHeight = std::max(m_maxHeight, point.y);
    }
    
    m_pendingUpdate = true; // Mark for update but don't set dirty yet
}

void TerrainGrid::checkDirty() {
    if (m_pendingUpdate) {
        double now = glfwGetTime();
        // Only update terrain mesh every 0.5 seconds to reduce flickering
        if (now - m_lastUpdateTime > 0.5) {
            m_dirty = true;
            m_lastUpdateTime = now;
        }
    }
}

void TerrainGrid::clear() {
    m_cells.clear();
    m_minHeight = 0.0f;
    m_maxHeight = 1.0f;
    m_dirty = true;
    m_pendingUpdate = false;
}

// DataManager implementation
DataManager::DataManager()
    : m_rovers{{RoverData(1), RoverData(2), RoverData(3), RoverData(4), RoverData(5)}},
      m_terrain(2.0f) // 2 meter cell size
{
}

void DataManager::updateRoverPose(int roverId, const PosePacket& pose) {
    if (roverId < 1 || roverId > NUM_ROVERS) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_rovers[roverId - 1].updatePose(pose);
}

void DataManager::updateRoverTelemetry(int roverId, const VehicleTelem& telem) {
    if (roverId < 1 || roverId > NUM_ROVERS) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_rovers[roverId - 1].updateTelemetry(telem);
}

void DataManager::update(float deltaTime) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& rover : m_rovers) {
        rover.interpolate(deltaTime);
    }
    // Point clouds handle their own sync in the renderer
    m_terrain.checkDirty();
}

void DataManager::addPointCloud(int roverId, const std::vector<LidarPoint>& points) {
    if (roverId < 1 || roverId > NUM_ROVERS) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pointClouds[roverId - 1].addPoints(points);
    
    // Also add to terrain grid
    for (const auto& p : points) {
        m_terrain.addPoint(glm::vec3(p.x, p.y, p.z));
    }
}

RoverState& DataManager::getRover(int index) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_rovers[index].getState();
}

PointCloud& DataManager::getPointCloud(int index) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_pointClouds[index];
}

TerrainGrid& DataManager::getTerrainGrid() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_terrain;
}

size_t DataManager::getTotalPointCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t total = 0;
    for (const auto& pc : m_pointClouds) {
        total += pc.getPointCount();
    }
    return total;
}

} // namespace terrafirma

