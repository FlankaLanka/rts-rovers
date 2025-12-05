#include "data/PointCloud.h"
#include <algorithm>

namespace terrafirma {

PointCloud::PointCloud() {
    m_points.reserve(2000000); // 2 million points
}

void PointCloud::addPoints(const std::vector<LidarPoint>& points) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (const auto& p : points) {
        glm::vec3 point(p.x, p.y, p.z);
        m_points.push_back(point);
        
        // Y is height
        m_minHeight = std::min(m_minHeight, point.y);
        m_maxHeight = std::max(m_maxHeight, point.y);
    }
}

void PointCloud::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_points.clear();
    m_minHeight = 0.0f;
    m_maxHeight = 100.0f;
    m_lastRenderedCount = 0;
}

size_t PointCloud::getNewPointsForRendering(const glm::vec3** outData, size_t* outTotalCount,
                                             float* outMinHeight, float* outMaxHeight) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    size_t currentCount = m_points.size();
    size_t newPoints = 0;
    
    if (currentCount > m_lastRenderedCount) {
        newPoints = currentCount - m_lastRenderedCount;
    }
    
    *outData = m_points.empty() ? nullptr : m_points.data();
    *outTotalCount = currentCount;
    *outMinHeight = m_minHeight;
    *outMaxHeight = m_maxHeight;
    
    m_lastRenderedCount = currentCount;
    
    return newPoints;
}

size_t PointCloud::getPointCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_points.size();
}

float PointCloud::getMinHeight() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_minHeight;
}

float PointCloud::getMaxHeight() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_maxHeight;
}

} // namespace terrafirma
