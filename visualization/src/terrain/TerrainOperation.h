#pragma once

#include "common.h"
#include "data/DataManager.h"
#include <glm/glm.hpp>

namespace terrafirma {

enum class OperationState {
    IDLE,           // Normal operation - no dig/pile in progress
    DRAWING,        // User is drawing circle (click and drag)
    CONFIRMING,     // Circle drawn, waiting for confirm/redo
    MOVING,         // Rover moving to dig site
    OPERATING       // Digging or piling in progress
};

enum class OperationType {
    NONE,
    DIG,    // Lower terrain (button 1)
    PILE    // Raise terrain (button 2)
};

class TerrainOperation {
public:
    // Constants
    static constexpr float MAX_DEPTH = 25.0f;       // Maximum dig/pile depth in meters
    static constexpr float DIG_RATE = 1.0f;         // Meters per second (25m takes 25 seconds)
    static constexpr float HOVER_HEIGHT = 3.0f;     // Meters above terrain
    static constexpr float MOVE_SPEED = 5.0f;       // Meters per second (slower movement)
    static constexpr float ARRIVAL_THRESHOLD = 1.0f; // Distance to consider "arrived"
    
    // Returns true if rover should ignore UDP position updates
    bool isControllingRover() const { 
        return m_state == OperationState::MOVING || m_state == OperationState::OPERATING; 
    }
    
    TerrainOperation();
    
    // State accessors
    OperationState getState() const { return m_state; }
    OperationType getType() const { return m_type; }
    glm::vec2 getCenter() const { return m_center; }
    float getRadius() const { return m_radius; }
    float getCurrentDepth() const { return m_currentDepth; }
    bool isActive() const { return m_state != OperationState::IDLE; }
    
    // Start drawing a circle for dig or pile
    void startDrawing(OperationType type);
    
    // Update circle during drawing (mouse drag)
    void updateDrawing(const glm::vec2& center, float radius);
    
    // Finish drawing, wait for confirmation
    void finishDrawing();
    
    // Confirm the circle - start moving rover
    void confirm();
    
    // Redo - go back to drawing
    void redo();
    
    // Cancel the entire operation
    void cancel();
    
    // Update the operation each frame
    // Returns true if terrain was modified
    bool update(float deltaTime, glm::vec3& roverPosition, TerrainGrid& terrain);
    
private:
    // Move rover toward the dig site
    // Returns true when arrived
    bool moveRover(float deltaTime, glm::vec3& roverPosition, const TerrainGrid& terrain);
    
    // Perform the dig/pile operation
    // Returns true if terrain was modified
    bool performOperation(float deltaTime, TerrainGrid& terrain);
    
    OperationState m_state = OperationState::IDLE;
    OperationType m_type = OperationType::NONE;
    
    glm::vec2 m_center{0.0f};       // Circle center (XZ)
    float m_radius = 0.0f;          // Circle radius
    float m_currentDepth = 0.0f;    // Current dig/pile depth timer (accumulates continuously)
    float m_appliedDepth = 0.0f;    // Depth actually applied to terrain (in 5m steps)
    
    static constexpr float DEPTH_STEP = 5.0f;  // Terrain changes in 5m increments
    
    glm::vec2 m_drawStart{0.0f};    // Where drawing started (for click-drag)
};

// Manager for all rover operations (one per rover)
class TerrainOperationManager {
public:
    TerrainOperationManager();
    
    // Get operation for a specific rover (0-indexed)
    TerrainOperation& getOperation(int roverIndex);
    const TerrainOperation& getOperation(int roverIndex) const;
    
    // Update all operations
    void update(float deltaTime, DataManager& dataManager);
    
    // Get the rover index that has an active drawing/confirming operation
    // Returns -1 if none
    int getDrawingRover() const;
    
    // Check if a rover is being controlled by an operation (should ignore UDP position updates)
    bool isRoverControlled(int roverIndex) const;
    
private:
    std::array<TerrainOperation, NUM_ROVERS> m_operations;
};

} // namespace terrafirma

