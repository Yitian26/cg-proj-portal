#pragma once

#include "Camera.h"
#include "GameObject.h"
#include "PhysicsSystem.h"
#include "InputManager.h"

// Forward declaration
class Scene;

class Player : public GameObject {
public:
    Player(glm::vec3 position);
    ~Player() = default;

    void update(float dt, PhysicsSystem *physicsSystem);
    void processInput(const InputManager &input, Scene *scene, float dt);

    Camera camera;

    // Physics properties
    bool isGrounded;
    bool isGrabbing = false;
    GameObject *grabbedObject = nullptr;

    // Configuration
    float moveSpeed = 4.0f;
    float jumpForce = 4.0f;
    float gravity = 9.8f;
    float height = 2.0f;
    float radius = 0.25f;


    // Helper to get current position (feet)
    glm::vec3 getPosition() const { return position; }
    void setPosition(const glm::vec3 &pos) { position = pos; }

private:
    void checkGrounded(PhysicsSystem *physicsSystem);
    void resolveCollisions(PhysicsSystem *physicsSystem, glm::vec3 &proposedVelocity, float dt);
};
