#include "terrain/TerrainRaycast.h"
#include <cmath>
#include <limits>

namespace terrafirma {

void screenToWorldRay(
    float mouseX, float mouseY,
    int screenWidth, int screenHeight,
    const glm::mat4& view, const glm::mat4& projection,
    glm::vec3& outRayOrigin, glm::vec3& outRayDir)
{
    // Convert mouse coordinates to normalized device coordinates (NDC)
    // Screen coords: (0,0) top-left, (width, height) bottom-right
    // NDC: (-1,-1) bottom-left, (1,1) top-right
    float ndcX = (2.0f * mouseX) / screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY) / screenHeight;  // Flip Y
    
    // Create ray in clip space
    glm::vec4 rayClipNear(ndcX, ndcY, -1.0f, 1.0f);  // Near plane
    glm::vec4 rayClipFar(ndcX, ndcY, 1.0f, 1.0f);    // Far plane
    
    // Transform to world space
    glm::mat4 invProjView = glm::inverse(projection * view);
    
    glm::vec4 rayWorldNear = invProjView * rayClipNear;
    glm::vec4 rayWorldFar = invProjView * rayClipFar;
    
    // Perspective divide
    rayWorldNear /= rayWorldNear.w;
    rayWorldFar /= rayWorldFar.w;
    
    outRayOrigin = glm::vec3(rayWorldNear);
    outRayDir = glm::normalize(glm::vec3(rayWorldFar - rayWorldNear));
}

bool getTerrainHeightAt(const TerrainGrid& terrain, float x, float z, float& outHeight) {
    const auto& cells = terrain.getCells();
    float cellSize = terrain.getCellSize();
    
    // Get the cell index
    int cx = static_cast<int>(std::floor(x / cellSize));
    int cz = static_cast<int>(std::floor(z / cellSize));
    
    // Check if we have this cell
    auto key = std::make_pair(cx, cz);
    auto it = cells.find(key);
    
    if (it != cells.end()) {
        outHeight = it->second;
        return true;
    }
    
    // Try to interpolate from nearby cells
    // Check the 4 surrounding cells
    float totalHeight = 0.0f;
    int count = 0;
    
    for (int dx = -1; dx <= 1; dx++) {
        for (int dz = -1; dz <= 1; dz++) {
            auto nearKey = std::make_pair(cx + dx, cz + dz);
            auto nearIt = cells.find(nearKey);
            if (nearIt != cells.end()) {
                totalHeight += nearIt->second;
                count++;
            }
        }
    }
    
    if (count > 0) {
        outHeight = totalHeight / count;
        return true;
    }
    
    return false;
}

RaycastResult raycastTerrain(
    float mouseX, float mouseY,
    int screenWidth, int screenHeight,
    const glm::mat4& view, const glm::mat4& projection,
    const TerrainGrid& terrain)
{
    RaycastResult result;
    result.hit = false;
    
    // Get ray from mouse position
    glm::vec3 rayOrigin, rayDir;
    screenToWorldRay(mouseX, mouseY, screenWidth, screenHeight, view, projection, rayOrigin, rayDir);
    
    const auto& cells = terrain.getCells();
    if (cells.empty()) {
        return result;
    }
    
    float cellSize = terrain.getCellSize();
    
    // March along the ray and check for terrain intersection
    // We'll use a simple step-based approach
    const float maxDistance = 2000.0f;
    const float stepSize = cellSize * 0.5f;  // Half cell size for accuracy
    
    float closestHit = std::numeric_limits<float>::max();
    
    for (float t = 0.0f; t < maxDistance; t += stepSize) {
        glm::vec3 point = rayOrigin + rayDir * t;
        
        // Get terrain height at this XZ position
        float terrainHeight;
        if (getTerrainHeightAt(terrain, point.x, point.z, terrainHeight)) {
            // Check if ray is at or below terrain
            if (point.y <= terrainHeight) {
                // Found intersection - back up slightly for better accuracy
                float refinedT = t;
                for (int i = 0; i < 5; i++) {  // Binary search refinement
                    refinedT -= stepSize / (2 << i);
                    glm::vec3 testPoint = rayOrigin + rayDir * refinedT;
                    float testHeight;
                    if (getTerrainHeightAt(terrain, testPoint.x, testPoint.z, testHeight)) {
                        if (testPoint.y > testHeight) {
                            refinedT += stepSize / (2 << i);
                        }
                    }
                }
                
                glm::vec3 hitPoint = rayOrigin + rayDir * refinedT;
                float hitHeight;
                if (getTerrainHeightAt(terrain, hitPoint.x, hitPoint.z, hitHeight)) {
                    hitPoint.y = hitHeight;  // Snap to terrain surface
                }
                
                if (refinedT < closestHit) {
                    closestHit = refinedT;
                    result.hit = true;
                    result.position = hitPoint;
                    result.distance = refinedT;
                }
                break;
            }
        }
    }
    
    return result;
}

} // namespace terrafirma

