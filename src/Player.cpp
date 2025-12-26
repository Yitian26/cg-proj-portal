#include "Player.h"
#include <iostream>
#include <algorithm>

Player::Player(glm::vec3 startPos)
    : GameObject(nullptr, startPos), // Initialize GameObject
    camera(startPos), // Default, will be updated
    isGrounded(false) {

    isTeleportable = true;
    // Initialize RigidBody
    rigidBody = std::make_unique<RigidBody>();
    rigidBody->velocity = glm::vec3(0.0f);
    rigidBody->useGravity = false; // We handle gravity manually in update()
    rigidBody->isStatic = false;
    rigidBody->collisionMask = COLLISION_MASK_DEFAULT;

    // Initialize collider (local space relative to position? No, let's keep it simple: World Space AABB updated every frame)
    // Center is position + height/2
    glm::vec3 min = position + glm::vec3(-radius, 0.0f, -radius);
    glm::vec3 max = position + glm::vec3(radius, height, radius);
    collider = std::make_unique<AABB>(min, max);

    // Set correct camera height based on member variable
    camera.Position = position + glm::vec3(0.0f, height * 0.9f, 0.0f);
}

void Player::processInput(const InputManager &input, float dt) {
    // Calculate target velocity based on input
    glm::vec3 targetVel(0.0f);

    // Get camera forward/right vectors but flatten them on Y axis
    glm::vec3 forward = camera.Front;
    forward.y = 0.0f;
    forward = glm::normalize(forward);

    glm::vec3 right = camera.Right;
    right.y = 0.0f;
    right = glm::normalize(right);

    if (input.isKeyDown(GLFW_KEY_W)) targetVel += forward;
    if (input.isKeyDown(GLFW_KEY_S)) targetVel -= forward;
    if (input.isKeyDown(GLFW_KEY_A)) targetVel -= right;
    if (input.isKeyDown(GLFW_KEY_D)) targetVel += right;

    if (glm::length(targetVel) > 0.0f) {
        targetVel = glm::normalize(targetVel) * moveSpeed;
    } else if (!isGrounded){
        // No input: maintain current horizontal velocity for momentum
        targetVel.x = rigidBody->velocity.x;
        targetVel.z = rigidBody->velocity.z;
    } else {
        targetVel = glm::vec3(0.0f);
    }

    // Smoothly interpolate horizontal velocity (simple acceleration)
    float accel = 10.0f;
    if (!isGrounded) accel = 2.0f; // Less control in air

    rigidBody->velocity.x = glm::mix(rigidBody->velocity.x, targetVel.x, accel * dt);
    rigidBody->velocity.z = glm::mix(rigidBody->velocity.z, targetVel.z, accel * dt);

    // Jump
    if (input.isKeyPressed(GLFW_KEY_SPACE) && isGrounded) {
        rigidBody->velocity.y = jumpForce;
        isGrounded = false;
    }
}

void Player::update(float dt, PhysicsSystem *physicsSystem) {
    // Apply Gravity
    rigidBody->velocity.y -= gravity * dt;

    // Terminal velocity
    rigidBody->velocity.y = std::max(rigidBody->velocity.y, -20.0f);

    // Proposed movement
    glm::vec3 displacement = rigidBody->velocity * dt;

    // Collision Detection & Resolution (Kinematic)
    // We will do a simple "collide and slide" approach
    // 1. Try moving X
    // 2. Try moving Z
    // 3. Try moving Y

    // Update collider to current position before checking
    // Actually, we need to check *from* current position *to* new position

    // Simple iterative solver:
    // Move Y first (handle gravity/ground)
    {
        position.y += displacement.y;
        // Update collider
        collider->min = position + glm::vec3(-radius, 0.0f, -radius);
        collider->max = position + glm::vec3(radius, height, radius);

        // Check collision
        // We need a way to query the physics system for overlaps with this AABB
        // Since PhysicsSystem doesn't expose a "checkOverlap(AABB)" directly, we might need to add it or iterate here.
        // For now, let's assume we can iterate physics objects.
        // But PhysicsSystem::physicsObjects is private. 
        // We should add a helper to PhysicsSystem: `bool checkOverlap(const AABB& aabb, glm::vec3& correction)`

        // WORKAROUND: Since we can't easily modify PhysicsSystem private access without editing it, 
        // let's assume we will add a public method `checkPlayerCollision` to PhysicsSystem.
        // Or we can use raycasts for ground detection and simple AABB checks if we had access.

        // Let's rely on a new method in PhysicsSystem we will add: `resolvePlayerCollision`
        // But for now, I'll implement a placeholder logic that assumes we will add that method.

        if (!rigidBody->isCollisionEnabled) {
            // Skip collision check
        } else {
            glm::vec3 correction;
            if (physicsSystem->checkPlayerCollision(*collider, correction, rigidBody->collisionMask)) {
                // If we hit something moving Y
                position += correction;

                if (rigidBody->velocity.y < 0 && correction.y > 0) {
                    isGrounded = true;
                    rigidBody->velocity.y = 0;
                } else if (rigidBody->velocity.y > 0 && correction.y < 0) {
                    // Hit ceiling
                    rigidBody->velocity.y = 0;
                }
            } else {
                isGrounded = false;
            }
        }
    }

    // Move X/Z
    {
        glm::vec3 horizontalDisp = glm::vec3(displacement.x, 0.0f, displacement.z);
        position += horizontalDisp;

        collider->min = position + glm::vec3(-radius, 0.0f, -radius);
        collider->max = position + glm::vec3(radius, height, radius);

        if (rigidBody->isCollisionEnabled) {
            glm::vec3 correction;
            if (physicsSystem->checkPlayerCollision(*collider, correction, rigidBody->collisionMask)) {
                position += correction;
                // No need to kill velocity, just slide (position is already corrected)
            }
        }
    }

    // Sync Camera
    camera.Position = position + glm::vec3(0.0f, height * 0.9f, 0.0f); // Eyes near top
}
