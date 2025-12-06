#pragma once

#include "common.h"
#include "data/DataManager.h"
#include <glm/glm.hpp>

namespace terrafirma {

struct RaycastResult {
    bool hit = false;
    glm::vec3 position{0.0f};  // World position on terrain
    float distance = 0.0f;     // Distance from ray origin
};

// Convert mouse screen position to a world ray
// Returns ray origin and ray direction (normalized)
void screenToWorldRay(
    float mouseX, float mouseY,
    int screenWidth, int screenHeight,
    const glm::mat4& view, const glm::mat4& projection,
    glm::vec3& outRayOrigin, glm::vec3& outRayDir
);

// Raycast against the terrain grid
// Returns hit information including world position
RaycastResult raycastTerrain(
    float mouseX, float mouseY,
    int screenWidth, int screenHeight,
    const glm::mat4& view, const glm::mat4& projection,
    const TerrainGrid& terrain
);

// Get terrain height at a specific XZ position (interpolated)
// Returns false if no terrain data exists at that position
bool getTerrainHeightAt(const TerrainGrid& terrain, float x, float z, float& outHeight);

} // namespace terrafirma

