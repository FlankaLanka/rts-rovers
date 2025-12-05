#pragma once

#include "common.h"
#include "data/RoverData.h"
#include "data/PointCloud.h"
#include <array>
#include <mutex>
#include <vector>
#include <map>

namespace terrafirma {

// Simple terrain grid
class TerrainGrid {
public:
    TerrainGrid(float cellSize = 1.0f);
    
    void addPoint(const glm::vec3& point);
    void clear();
    void checkDirty(); // Check if enough time has passed to mark as dirty
    
    const std::map<std::pair<int, int>, float>& getCells() const { return m_cells; }
    float getCellSize() const { return m_cellSize; }
    float getMinHeight() const { return m_minHeight; }
    float getMaxHeight() const { return m_maxHeight; }
    
    bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; m_pendingUpdate = false; }

private:
    std::map<std::pair<int, int>, float> m_cells; // (x, y) -> height
    float m_cellSize;
    float m_minHeight = 0.0f;
    float m_maxHeight = 1.0f;
    bool m_dirty = false;
    bool m_pendingUpdate = false;
    double m_lastUpdateTime = 0.0;
};

class DataManager {
public:
    DataManager();
    
    // Thread-safe updates (call from network thread)
    void updateRoverPose(int roverId, const PosePacket& pose);
    void updateRoverTelemetry(int roverId, const VehicleTelem& telem);
    void addPointCloud(int roverId, const std::vector<LidarPoint>& points);
    
    // Call from render thread each frame
    void update(float deltaTime);
    
    // Accessors (call from render thread)
    RoverState& getRover(int index);
    PointCloud& getPointCloud(int index);
    TerrainGrid& getTerrainGrid();
    
    size_t getTotalPointCount() const;

private:
    std::array<RoverData, NUM_ROVERS> m_rovers;
    std::array<PointCloud, NUM_ROVERS> m_pointClouds;
    TerrainGrid m_terrain;
    
    mutable std::mutex m_mutex;
};

} // namespace terrafirma

