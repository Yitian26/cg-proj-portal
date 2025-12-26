#pragma once

#include "Camera.h"
#include "GameObject.h"
#include "PhysicsSystem.h"
#include "InputManager.h"

class Player {
public:
    Player(glm::vec3 position);
    ~Player() = default;

    void update(float dt, PhysicsSystem *physicsSystem);
    void processInput(const InputManager &input, float dt);

    Camera camera;

    // Physics properties
    glm::vec3 velocity;
    bool isGrounded;
    uint32_t collisionMask = COLLISION_MASK_DEFAULT;

    // Configuration
    float moveSpeed = 5.0f;
    float jumpForce = 5.0f;
    float gravity = 9.8f;
    float height = 2.0f;
    float radius = 0.3f; // Approximate width

    // We use a simple AABB for the player for now
    std::unique_ptr<AABB> collider;

    // Helper to get current position (feet)
    glm::vec3 getPosition() const { return position; }
    void setPosition(const glm::vec3 &pos) { position = pos; }
    void setCollisionMask(uint32_t mask) { collisionMask = mask; }

private:
    glm::vec3 position; // Feet position

    void checkGrounded(PhysicsSystem *physicsSystem);
    void resolveCollisions(PhysicsSystem *physicsSystem, glm::vec3 &proposedVelocity, float dt);
};
