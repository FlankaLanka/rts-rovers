#include "pathfinding/AStar.h"
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <cmath>
#include <random>
#include <algorithm>

namespace terrafirma {

// Hash function for grid coordinates
struct PairHash {
    size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 16);
    }
};

std::pair<int, int> AStar::worldToGrid(const TerrainGrid& terrain, float worldX, float worldZ) {
    float cellSize = terrain.getCellSize();
    return {
        static_cast<int>(std::floor(worldX / cellSize)),
        static_cast<int>(std::floor(worldZ / cellSize))
    };
}

glm::vec3 AStar::gridToWorld(const TerrainGrid& terrain, int gridX, int gridZ) {
    float cellSize = terrain.getCellSize();
    float worldX = gridX * cellSize + cellSize * 0.5f;
    float worldZ = gridZ * cellSize + cellSize * 0.5f;
    
    // Get height at this cell
    float height = 0.0f;
    getGridHeight(terrain, gridX, gridZ, height);
    
    return glm::vec3(worldX, height, worldZ);
}

bool AStar::getGridHeight(const TerrainGrid& terrain, int x, int z, float& height) {
    const auto& cells = terrain.getCells();
    auto it = cells.find({x, z});
    if (it != cells.end()) {
        height = it->second;
        return true;
    }
    return false;
}

bool AStar::isValidMove(const TerrainGrid& terrain, int fromX, int fromZ, int toX, int toZ, float maxSlope) {
    float fromHeight, toHeight;
    
    // Both cells must exist
    if (!getGridHeight(terrain, fromX, fromZ, fromHeight)) return false;
    if (!getGridHeight(terrain, toX, toZ, toHeight)) return false;
    
    // Calculate horizontal distance
    float cellSize = terrain.getCellSize();
    float dx = (toX - fromX) * cellSize;
    float dz = (toZ - fromZ) * cellSize;
    float horizontalDist = std::sqrt(dx * dx + dz * dz);
    
    // Calculate slope angle
    float heightDiff = std::abs(toHeight - fromHeight);
    float slopeAngle = std::atan2(heightDiff, horizontalDist) * 180.0f / 3.14159f;
    
    return slopeAngle <= maxSlope;
}

float AStar::heuristic(int x1, int z1, int x2, int z2) {
    // Euclidean distance
    float dx = static_cast<float>(x2 - x1);
    float dz = static_cast<float>(z2 - z1);
    return std::sqrt(dx * dx + dz * dz);
}

std::vector<glm::vec3> AStar::findPath(
    const TerrainGrid& terrain,
    const glm::vec3& start,
    const glm::vec3& goal,
    float maxSlopeDegrees
) {
    auto [startX, startZ] = worldToGrid(terrain, start.x, start.z);
    auto [goalX, goalZ] = worldToGrid(terrain, goal.x, goal.z);
    
    // Check if start and goal cells exist
    float startHeight, goalHeight;
    if (!getGridHeight(terrain, startX, startZ, startHeight)) {
        // Start cell doesn't exist, find nearest valid cell
        const auto& cells = terrain.getCells();
        float minDist = FLT_MAX;
        for (const auto& [coord, height] : cells) {
            float dist = heuristic(startX, startZ, coord.first, coord.second);
            if (dist < minDist) {
                minDist = dist;
                startX = coord.first;
                startZ = coord.second;
            }
        }
    }
    
    if (!getGridHeight(terrain, goalX, goalZ, goalHeight)) {
        // Goal cell doesn't exist, find nearest valid cell
        const auto& cells = terrain.getCells();
        float minDist = FLT_MAX;
        for (const auto& [coord, height] : cells) {
            float dist = heuristic(goalX, goalZ, coord.first, coord.second);
            if (dist < minDist) {
                minDist = dist;
                goalX = coord.first;
                goalZ = coord.second;
            }
        }
    }
    
    // A* algorithm
    auto cmp = [](const Node* a, const Node* b) { return a->f > b->f; };
    std::priority_queue<Node*, std::vector<Node*>, decltype(cmp)> openSet(cmp);
    std::unordered_map<std::pair<int, int>, Node, PairHash> allNodes;
    std::unordered_set<std::pair<int, int>, PairHash> closedSet;
    
    // Initialize start node
    Node* startNode = &allNodes[{startX, startZ}];
    startNode->x = startX;
    startNode->z = startZ;
    startNode->g = 0;
    startNode->h = heuristic(startX, startZ, goalX, goalZ);
    startNode->f = startNode->h;
    startNode->parent = nullptr;
    
    openSet.push(startNode);
    
    // 8-directional movement
    const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    const int dz[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    const float costs[] = {1.414f, 1.0f, 1.414f, 1.0f, 1.0f, 1.414f, 1.0f, 1.414f};
    
    int iterations = 0;
    const int maxIterations = 10000;  // Prevent infinite loops
    
    while (!openSet.empty() && iterations < maxIterations) {
        iterations++;
        
        Node* current = openSet.top();
        openSet.pop();
        
        // Check if we reached the goal
        if (current->x == goalX && current->z == goalZ) {
            // Reconstruct path
            std::vector<glm::vec3> path;
            Node* node = current;
            while (node != nullptr) {
                path.push_back(gridToWorld(terrain, node->x, node->z));
                node = node->parent;
            }
            std::reverse(path.begin(), path.end());
            return simplifyPath(path);
        }
        
        auto currentCoord = std::make_pair(current->x, current->z);
        if (closedSet.count(currentCoord)) continue;
        closedSet.insert(currentCoord);
        
        // Explore neighbors
        for (int i = 0; i < 8; i++) {
            int nx = current->x + dx[i];
            int nz = current->z + dz[i];
            
            auto neighborCoord = std::make_pair(nx, nz);
            if (closedSet.count(neighborCoord)) continue;
            
            // Check if move is valid
            if (!isValidMove(terrain, current->x, current->z, nx, nz, maxSlopeDegrees)) {
                continue;
            }
            
            float tentativeG = current->g + costs[i];
            
            // Add height difference as additional cost
            float currentHeight, neighborHeight;
            if (getGridHeight(terrain, current->x, current->z, currentHeight) &&
                getGridHeight(terrain, nx, nz, neighborHeight)) {
                tentativeG += std::abs(neighborHeight - currentHeight) * 0.5f;
            }
            
            Node* neighbor = &allNodes[neighborCoord];
            
            // If this is a new node or we found a better path
            if (neighbor->g == 0 || tentativeG < neighbor->g) {
                neighbor->x = nx;
                neighbor->z = nz;
                neighbor->g = tentativeG;
                neighbor->h = heuristic(nx, nz, goalX, goalZ);
                neighbor->f = neighbor->g + neighbor->h;
                neighbor->parent = current;
                
                openSet.push(neighbor);
            }
        }
    }
    
    // No path found
    return {};
}

bool AStar::isReachable(const TerrainGrid& terrain, const glm::vec3& from, const glm::vec3& to) {
    auto path = findPath(terrain, from, to);
    return !path.empty();
}

glm::vec3 AStar::findRandomReachable(
    const TerrainGrid& terrain,
    const glm::vec3& from,
    float minDist,
    float maxDist,
    int maxAttempts
) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    const auto& cells = terrain.getCells();
    if (cells.empty()) return from;
    
    // Collect cells within distance range
    std::vector<std::pair<int, int>> candidates;
    float cellSize = terrain.getCellSize();
    
    for (const auto& [coord, height] : cells) {
        float worldX = coord.first * cellSize + cellSize * 0.5f;
        float worldZ = coord.second * cellSize + cellSize * 0.5f;
        
        float dist = std::sqrt(
            (worldX - from.x) * (worldX - from.x) +
            (worldZ - from.z) * (worldZ - from.z)
        );
        
        if (dist >= minDist && dist <= maxDist) {
            candidates.push_back(coord);
        }
    }
    
    if (candidates.empty()) {
        // No cells in range, try any cell
        for (const auto& [coord, height] : cells) {
            candidates.push_back(coord);
        }
    }
    
    if (candidates.empty()) return from;
    
    // Try random candidates until we find a reachable one
    std::shuffle(candidates.begin(), candidates.end(), gen);
    
    for (int i = 0; i < std::min(maxAttempts, static_cast<int>(candidates.size())); i++) {
        auto& coord = candidates[i];
        glm::vec3 target = gridToWorld(terrain, coord.first, coord.second);
        
        if (isReachable(terrain, from, target)) {
            return target;
        }
    }
    
    // No reachable position found
    return from;
}

std::vector<glm::vec3> AStar::simplifyPath(const std::vector<glm::vec3>& path) {
    if (path.size() <= 2) return path;
    
    std::vector<glm::vec3> simplified;
    simplified.push_back(path[0]);
    
    glm::vec3 lastDir(0.0f);
    
    for (size_t i = 1; i < path.size() - 1; i++) {
        glm::vec3 dir = glm::normalize(path[i + 1] - path[i - 1]);
        
        // Check if direction changed significantly
        if (i == 1 || glm::dot(dir, lastDir) < 0.98f) {
            simplified.push_back(path[i]);
            lastDir = dir;
        }
    }
    
    simplified.push_back(path.back());
    return simplified;
}

} // namespace terrafirma

