#pragma once

#include "common.h"
#include "data/DataManager.h"
#include <glm/glm.hpp>
#include <vector>

namespace terrafirma {

class AStar {
public:
    AStar() = default;
    
    // Find path on terrain grid, returns waypoints in world coords
    // Returns empty vector if no path found
    std::vector<glm::vec3> findPath(
        const TerrainGrid& terrain,
        const glm::vec3& start, 
        const glm::vec3& goal,
        float maxSlopeDegrees = 45.0f
    );
    
    // Check if a position is reachable from another using pathfinding
    bool isReachable(
        const TerrainGrid& terrain,
        const glm::vec3& from,
        const glm::vec3& to
    );
    
    // Find a random reachable position within radius
    // Returns start position if no valid position found
    glm::vec3 findRandomReachable(
        const TerrainGrid& terrain,
        const glm::vec3& from,
        float minDist = 20.0f,
        float maxDist = 50.0f,
        int maxAttempts = 20
    );
    
private:
    struct Node {
        int x, z;           // Grid cell coordinates
        float g = 0;        // Cost from start
        float h = 0;        // Heuristic (estimated cost to goal)
        float f = 0;        // Total cost (g + h)
        Node* parent = nullptr;
        
        bool operator>(const Node& other) const { return f > other.f; }
    };
    
    // Convert world position to grid cell
    std::pair<int, int> worldToGrid(const TerrainGrid& terrain, float worldX, float worldZ);
    
    // Convert grid cell to world position
    glm::vec3 gridToWorld(const TerrainGrid& terrain, int gridX, int gridZ);
    
    // Get height at grid cell (returns false if cell doesn't exist)
    bool getGridHeight(const TerrainGrid& terrain, int x, int z, float& height);
    
    // Check if movement from one cell to another is valid (slope check)
    bool isValidMove(const TerrainGrid& terrain, int fromX, int fromZ, int toX, int toZ, float maxSlope);
    
    // Heuristic: Euclidean distance
    float heuristic(int x1, int z1, int x2, int z2);
    
    // Simplify path by removing unnecessary waypoints
    std::vector<glm::vec3> simplifyPath(const std::vector<glm::vec3>& path);
};

} // namespace terrafirma

