#include "Player.h"
#include "Scene.h"

#include <iostream>
#include <algorithm>

Player::Player(glm::vec3 startPos)
    : GameObject(nullptr, startPos),
    camera(startPos),
    isGrounded(false) {

    isTeleportable = true;
    // Initialize RigidBody
    rigidBody = std::make_unique<RigidBody>();
    rigidBody->velocity = glm::vec3(0.0f);
    rigidBody->useGravity = false; // We handle gravity manually in update()
    rigidBody->isStatic = false;
    rigidBody->collisionMask = COLLISION_MASK_DEFAULT;

    // Initialize collider (centered on position)
    glm::vec3 min = position + glm::vec3(-radius, -height * 0.5f, -radius);
    glm::vec3 max = position + glm::vec3(radius, height * 0.5f, radius);
    collider = std::make_unique<AABB>(min, max);

    // Set correct camera height
    camera.Position = position + glm::vec3(0.0f, height * 0.4f, 0.0f);
}

void Player::processInput(const InputManager &input, Scene *scene, float dt) {
    glm::vec3 targetVel(0.0f);

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
    } else if (!isGrounded) {
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

    //grabbing
    if (input.isKeyPressed(GLFW_KEY_E)) {
        if (isGrabbing) {
            isGrabbing = false;
        } else {
            auto result = scene->physicsSystem->raycast(camera.Position, camera.Front, 5.0f);
            if (result.hit && result.object && result.object->isTeleportable) {
                isGrabbing = true;
                grabbedObject = result.object;
            }
        }
    }

    //respawn portal
    if (input.isMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
        if (isGrabbing) {
            glm::vec3 pushForce = camera.Front * 10.0f;
            grabbedObject->rigidBody->addForce(pushForce);
            isGrabbing = false;
        } else {
            scene->portalGun->fire();
            auto result = scene->physicsSystem->raycast(camera.Position, camera.Front, 100.0f);
            scene->portalA->checkRaycast(result,camera.Right);
        }
    }
    if (input.isMousePressed(GLFW_MOUSE_BUTTON_RIGHT) && !isGrabbing) {
        scene->portalGun->fire();
        auto result = scene->physicsSystem->raycast(camera.Position, camera.Front, 100.0f);
        scene->portalB->checkRaycast(result,camera.Right);
    }

    // Roll recovery after teleport
    if (rollRecoveryTimer > 0) {
        rollRecoveryTimer -= dt;
        if (rollRecoveryTimer < 0) rollRecoveryTimer = 0;
        float t = rollRecoveryTimer / rollRecoveryDuration;
        camera.Roll = initialRoll * t;
        camera.updateCameraVectors();
    }
}

void Player::update(float dt, PhysicsSystem *physicsSystem) {
    // Apply Gravity
    rigidBody->velocity.y -= gravity * dt;

    // Terminal velocity
    rigidBody->velocity.y = std::max(rigidBody->velocity.y, -20.0f);

    // Proposed movement
    glm::vec3 displacement = rigidBody->velocity * dt;

    position.y += displacement.y;
    // Update collider (centered on position)
    collider->min = position + glm::vec3(-radius, -height * 0.5f, -radius);
    collider->max = position + glm::vec3(radius, height * 0.5f, radius);

    if (!rigidBody->isCollisionEnabled) {
        // Skip collision check
    } else {
        glm::vec3 correction;
        if (physicsSystem->checkPlayerCollision(*collider, correction, rigidBody->collisionMask)) {
            position += correction;
            if (rigidBody->velocity.y < 0 && correction.y > 0) {
                isGrounded = true;
                rigidBody->velocity.y = 0;
            } else if (rigidBody->velocity.y > 0 && correction.y < 0) {
                rigidBody->velocity.y = 0;
            }
        } else {
            isGrounded = false;
        }
    }


    // Move X/Z
    glm::vec3 horizontalDisp = glm::vec3(displacement.x, 0.0f, displacement.z);
    position += horizontalDisp;

    collider->min = position + glm::vec3(-radius, -height * 0.5f, -radius);
    collider->max = position + glm::vec3(radius, height * 0.5f, radius);

    if (rigidBody->isCollisionEnabled) {
        glm::vec3 correction;
        if (physicsSystem->checkPlayerCollision(*collider, correction, rigidBody->collisionMask)) {
            position += correction;
        }
    }

    if (isGrabbing && grabbedObject) {
        glm::vec3 targetPos = position + camera.Front * 2.0f + glm::vec3(0.0f, height * 0.5f, 0.0f);
        if (glm::length(targetPos - grabbedObject->position) > 3.0f) {
            // Too far, release
            isGrabbing = false;
        } else {
            glm::vec3 springForce = (targetPos - grabbedObject->position) * 30.0f;
            glm::vec3 dumplingForce = -grabbedObject->rigidBody->velocity * 5.0f;
            grabbedObject->rigidBody->addForce(springForce + dumplingForce);
        }
    }

    // Sync Camera
    camera.Position = position + glm::vec3(0.0f, height * 0.4f, 0.0f); // Eyes at center + 0.4*height
}
