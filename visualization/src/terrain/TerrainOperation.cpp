#include "terrain/TerrainOperation.h"
#include "terrain/TerrainRaycast.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace terrafirma {

TerrainOperation::TerrainOperation() {}

void TerrainOperation::startDrawing(OperationType type) {
    m_state = OperationState::DRAWING;
    m_type = type;
    m_center = glm::vec2(0.0f);
    m_radius = 0.0f;
    m_currentDepth = 0.0f;
    m_appliedDepth = 0.0f;
    m_drawStart = glm::vec2(0.0f);
}

void TerrainOperation::updateDrawing(const glm::vec2& center, float radius) {
    if (m_state == OperationState::DRAWING) {
        m_center = center;
        m_radius = radius;
    }
}

void TerrainOperation::finishDrawing() {
    if (m_state == OperationState::DRAWING && m_radius > 0.5f) {
        m_state = OperationState::CONFIRMING;
    }
}

void TerrainOperation::confirm() {
    if (m_state == OperationState::CONFIRMING) {
        m_state = OperationState::MOVING;
        std::cout << "Operation confirmed - rover moving to (" << m_center.x << ", " << m_center.y << ")\n";
    }
}

void TerrainOperation::redo() {
    if (m_state == OperationState::CONFIRMING) {
        m_state = OperationState::DRAWING;
        m_radius = 0.0f;
    }
}

void TerrainOperation::cancel() {
    m_state = OperationState::IDLE;
    m_type = OperationType::NONE;
    m_center = glm::vec2(0.0f);
    m_radius = 0.0f;
    m_currentDepth = 0.0f;
    m_appliedDepth = 0.0f;
}

bool TerrainOperation::update(float deltaTime, glm::vec3& roverPosition, TerrainGrid& terrain) {
    switch (m_state) {
        case OperationState::MOVING:
            if (moveRover(deltaTime, roverPosition, terrain)) {
                m_state = OperationState::OPERATING;
            }
            return false;
            
        case OperationState::OPERATING:
            return performOperation(deltaTime, terrain);
            
        default:
            return false;
    }
}

bool TerrainOperation::moveRover(float deltaTime, glm::vec3& roverPosition, const TerrainGrid& terrain) {
    // Calculate direction to circle center
    glm::vec2 roverXZ(roverPosition.x, roverPosition.z);
    glm::vec2 toTarget = m_center - roverXZ;
    float distance = glm::length(toTarget);
    
    if (distance < ARRIVAL_THRESHOLD) {
        // Arrived at destination
        std::cout << "Rover arrived at dig site - starting operation\n";
        return true;
    }
    
    // Move toward target
    glm::vec2 direction = toTarget / distance;
    float moveDistance = std::min(MOVE_SPEED * deltaTime, distance);
    
    roverXZ += direction * moveDistance;
    roverPosition.x = roverXZ.x;
    roverPosition.z = roverXZ.y;
    
    // Maintain hover height above terrain
    float terrainHeight = terrain.getMinHeight();
    getTerrainHeightAt(terrain, roverPosition.x, roverPosition.z, terrainHeight);
    roverPosition.y = terrainHeight + HOVER_HEIGHT;
    
    return false;
}

bool TerrainOperation::performOperation(float deltaTime, TerrainGrid& terrain) {
    // Check if operation is complete
    float targetDepth = (m_type == OperationType::DIG) ? -MAX_DEPTH : MAX_DEPTH;
    
    if ((m_type == OperationType::DIG && m_currentDepth <= targetDepth) ||
        (m_type == OperationType::PILE && m_currentDepth >= targetDepth)) {
        // Operation complete - auto-stop
        std::cout << "Operation complete! Total depth: " << m_appliedDepth << "m\n";
        cancel();
        return false;
    }
    
    // Accumulate depth based on time (this is the "timer")
    float depthChange = DIG_RATE * deltaTime;
    if (m_type == OperationType::DIG) {
        depthChange = -depthChange;  // Negative for digging
    }
    
    m_currentDepth += depthChange;
    
    // Clamp to target
    if (m_type == OperationType::DIG) {
        m_currentDepth = std::max(m_currentDepth, targetDepth);
    } else {
        m_currentDepth = std::min(m_currentDepth, targetDepth);
    }
    
    // Check if we need to apply a new 5m step
    float nextStep;
    if (m_type == OperationType::DIG) {
        nextStep = m_appliedDepth - DEPTH_STEP;  // Next step is more negative
    } else {
        nextStep = m_appliedDepth + DEPTH_STEP;  // Next step is more positive
    }
    
    bool shouldApplyStep = false;
    if (m_type == OperationType::DIG && m_currentDepth <= nextStep) {
        shouldApplyStep = true;
    } else if (m_type == OperationType::PILE && m_currentDepth >= nextStep) {
        shouldApplyStep = true;
    }
    
    if (!shouldApplyStep) {
        return false;  // No terrain change yet
    }
    
    // Apply the 5m step to terrain
    float stepChange = (m_type == OperationType::DIG) ? -DEPTH_STEP : DEPTH_STEP;
    m_appliedDepth += stepChange;
    
    std::cout << "Applying " << (m_type == OperationType::DIG ? "dig" : "pile") 
              << " step: " << DEPTH_STEP << "m (total: " << std::abs(m_appliedDepth) << "m)\n";
    
    // Modify terrain cells within the circle
    auto& cells = const_cast<std::map<std::pair<int, int>, float>&>(terrain.getCells());
    float cellSize = terrain.getCellSize();
    
    // Calculate bounding box of circle in cell coordinates
    int minCX = static_cast<int>(std::floor((m_center.x - m_radius) / cellSize));
    int maxCX = static_cast<int>(std::ceil((m_center.x + m_radius) / cellSize));
    int minCZ = static_cast<int>(std::floor((m_center.y - m_radius) / cellSize));
    int maxCZ = static_cast<int>(std::ceil((m_center.y + m_radius) / cellSize));
    
    bool modified = false;
    
    for (int cx = minCX; cx <= maxCX; cx++) {
        for (int cz = minCZ; cz <= maxCZ; cz++) {
            // Check if cell center is within circle
            float cellX = cx * cellSize + cellSize * 0.5f;
            float cellZ = cz * cellSize + cellSize * 0.5f;
            
            float dx = cellX - m_center.x;
            float dz = cellZ - m_center.y;
            float dist = std::sqrt(dx * dx + dz * dz);
            
            if (dist <= m_radius) {
                auto key = std::make_pair(cx, cz);
                auto it = cells.find(key);
                
                if (it != cells.end()) {
                    // Apply 5m step change to this cell
                    it->second += stepChange;
                    modified = true;
                }
            }
        }
    }
    
    return modified;
}

// TerrainOperationManager implementation
TerrainOperationManager::TerrainOperationManager() {}

TerrainOperation& TerrainOperationManager::getOperation(int roverIndex) {
    return m_operations[roverIndex];
}

const TerrainOperation& TerrainOperationManager::getOperation(int roverIndex) const {
    return m_operations[roverIndex];
}

void TerrainOperationManager::update(float deltaTime, DataManager& dataManager) {
    for (int i = 0; i < NUM_ROVERS; i++) {
        auto& op = m_operations[i];
        bool isControlling = op.isControllingRover();
        
        // Update the controlled flag in DataManager (this blocks UDP pose updates)
        dataManager.setRoverControlled(i, isControlling);
        
        if (isControlling) {
            // Get rover position (need mutable reference)
            auto& rover = dataManager.getRover(i);
            glm::vec3 pos = rover.position;
            
            bool terrainModified = op.update(deltaTime, pos, dataManager.getTerrainGrid());
            
            // Update rover position if it moved
            rover.position = pos;
            
            // Mark terrain as needing update if modified
            if (terrainModified) {
                // The terrain will be marked dirty internally
                dataManager.getTerrainGrid().checkDirty();
            }
        }
    }
}

int TerrainOperationManager::getDrawingRover() const {
    for (int i = 0; i < NUM_ROVERS; i++) {
        auto state = m_operations[i].getState();
        if (state == OperationState::DRAWING || state == OperationState::CONFIRMING) {
            return i;
        }
    }
    return -1;
}

bool TerrainOperationManager::isRoverControlled(int roverIndex) const {
    if (roverIndex < 0 || roverIndex >= NUM_ROVERS) return false;
    return m_operations[roverIndex].isControllingRover();
}

} // namespace terrafirma

