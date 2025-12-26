#pragma once

#include "Camera.h"
#include "GameObject.h"
#include "PhysicsSystem.h"
#include "InputManager.h"

class Player : public GameObject {
public:
    Player(glm::vec3 position);
    ~Player() = default;

    void update(float dt, PhysicsSystem *physicsSystem);
    void processInput(const InputManager &input, float dt);

    Camera camera;

    // Physics properties
    // velocity is now in rigidBody->velocity
    bool isGrounded;
    // collisionMask is now in rigidBody->collisionMask

    // Configuration
    float moveSpeed = 4.0f;
    float jumpForce = 4.0f;
    float gravity = 9.8f;
    float height = 2.0f;
    float radius = 0.25f; // Approximate width

    // collider is now in GameObject::collider

    // Helper to get current position (feet)
    glm::vec3 getPosition() const { return position; }
    void setPosition(const glm::vec3 &pos) { position = pos; }
    // setCollisionMask is inherited from GameObject
    // setCollisionEnabled is inherited from GameObject

private:
    // position is inherited from GameObject
    // isCollisionEnabled is in rigidBody

    void checkGrounded(PhysicsSystem *physicsSystem);
    void resolveCollisions(PhysicsSystem *physicsSystem, glm::vec3 &proposedVelocity, float dt);
};
