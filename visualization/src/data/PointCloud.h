#pragma once

#include "common.h"
#include <vector>
#include <mutex>

namespace terrafirma {

class PointCloud {
public:
    PointCloud();

    // Called from network thread
    void addPoints(const std::vector<LidarPoint>& points);
    void clear();
    
    // Called from render thread - returns pointer to data for direct GPU upload
    // Returns number of NEW points since last call (for incremental upload)
    size_t getNewPointsForRendering(const glm::vec3** outData, size_t* outTotalCount, 
                                     float* outMinHeight, float* outMaxHeight);
    
    size_t getPointCount() const;
    float getMinHeight() const;
    float getMaxHeight() const;

private:
    std::vector<glm::vec3> m_points;
    mutable std::mutex m_mutex;
    
    float m_minHeight = 0.0f;
    float m_maxHeight = 100.0f;
    
    size_t m_lastRenderedCount = 0;
};

} // namespace terrafirma
