#include "PhysicsSystem.h"
#include "GameObject.h"
#include <algorithm>
#include <iostream>
#include <glm/gtx/norm.hpp> // For length2


PhysicsSystem::PhysicsSystem() : gravity(glm::vec3(0.0f, -9.81f, 0.0f)) {}

PhysicsSystem::~PhysicsSystem() {}

void PhysicsSystem::setGravity(const glm::vec3 &g) {
    gravity = g;
}

void PhysicsSystem::addObject(GameObject *obj, RigidBody *rb, AABB *col) {
    PhysicsObject physObj;
    physObj.gameObject = obj;
    physObj.rigidBody = rb;
    physObj.collider = col;
    physicsObjects.push_back(physObj);
}

void PhysicsSystem::removeObject(GameObject *obj) {
    physicsObjects.erase(std::remove_if(physicsObjects.begin(), physicsObjects.end(),
        [obj](const PhysicsObject &pObj) { return pObj.gameObject == obj; }), physicsObjects.end());
}

void PhysicsSystem::update(float dt) {
    // 1. Integration
    for (auto &obj : physicsObjects) {
        if (obj.rigidBody && !obj.rigidBody->isStatic) {
            integrate(obj, dt);
        }

        // Update World OBB
        if (obj.collider && obj.gameObject) {
            // Calculate rotation matrix from Euler angles
            glm::mat4 rotationMat = glm::mat4(1.0f);
            rotationMat = glm::rotate(rotationMat, glm::radians(obj.gameObject->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            rotationMat = glm::rotate(rotationMat, glm::radians(obj.gameObject->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            rotationMat = glm::rotate(rotationMat, glm::radians(obj.gameObject->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            // Extract axes
            glm::vec3 axes[3];
            axes[0] = glm::vec3(rotationMat[0]); // Right
            axes[1] = glm::vec3(rotationMat[1]); // Up
            axes[2] = glm::vec3(rotationMat[2]); // Forward

            // Calculate center and half extents
            glm::vec3 localCenter = (obj.collider->min + obj.collider->max) * 0.5f;
            glm::vec3 localExtent = (obj.collider->max - obj.collider->min) * 0.5f;

            // Apply scale
            glm::vec3 scaledExtent = localExtent * obj.gameObject->scale;

            // Transform center to world space
            // Note: We need to rotate the local center offset first, then add to position
            glm::vec3 rotatedCenterOffset = glm::vec3(rotationMat * glm::vec4(localCenter * obj.gameObject->scale, 1.0f));
            glm::vec3 worldCenter = obj.gameObject->position + rotatedCenterOffset;

            obj.worldOBB = OBB(worldCenter, axes, scaledExtent);
        }
    }

    // 2. Collision Detection & Resolution
    checkCollisions();
}

void PhysicsSystem::integrate(PhysicsObject &obj, float dt) {
    RigidBody *rb = obj.rigidBody;
    GameObject *go = obj.gameObject;

    // Apply Gravity
    if (rb->useGravity) {
        rb->addForce(gravity * rb->mass);
    }

    // F = ma -> a = F/m
    glm::vec3 acc = rb->acceleration + (rb->force / rb->mass);

    // v = v0 + at
    rb->velocity += acc * dt;

    // Apply Damping/Friction (Simple linear drag)
    rb->velocity *= (1.0f - dt * 0.5f);

    // x = x0 + vt
    go->position += rb->velocity * dt;

    // Clear forces for next frame
    rb->clearForces();
}

void PhysicsSystem::checkCollisions() {
    for (size_t i = 0; i < physicsObjects.size(); ++i) {
        for (size_t j = i + 1; j < physicsObjects.size(); ++j) {
            PhysicsObject &a = physicsObjects[i];
            PhysicsObject &b = physicsObjects[j];

            if (!a.rigidBody->isCollisionEnabled || !b.rigidBody->isCollisionEnabled) continue;
            if (a.rigidBody->isStatic && b.rigidBody->isStatic) continue; // Skip static-static

            // Check collision masks
            if ((a.rigidBody->collisionMask & b.rigidBody->collisionMask) == 0) continue;

            glm::vec3 normal;
            float penetration;
            if (checkCollisionSAT(a.worldOBB, b.worldOBB, normal, penetration)) {
                resolveCollision(a, b, normal, penetration);
            }
        }
    }
}

// Helper for SAT test on a single axis
bool TestAxis(const glm::vec3 &axis, const OBB &a, const OBB &b, float &minPenetration, glm::vec3 &bestAxis) {
    if (glm::length2(axis) < 1e-6f) return true; // Skip near-zero axis
    glm::vec3 nAxis = glm::normalize(axis);

    // Project A
    float rA = glm::abs(glm::dot(a.axes[0], nAxis) * a.halfExtents.x) +
        glm::abs(glm::dot(a.axes[1], nAxis) * a.halfExtents.y) +
        glm::abs(glm::dot(a.axes[2], nAxis) * a.halfExtents.z);

    // Project B
    float rB = glm::abs(glm::dot(b.axes[0], nAxis) * b.halfExtents.x) +
        glm::abs(glm::dot(b.axes[1], nAxis) * b.halfExtents.y) +
        glm::abs(glm::dot(b.axes[2], nAxis) * b.halfExtents.z);

    // Distance between centers projected
    float dist = glm::abs(glm::dot(b.center - a.center, nAxis));

    float penetration = rA + rB - dist;

    // If penetration is negative, there is a gap -> no collision
    if (penetration < 0) return false;

    if (penetration < minPenetration) {
        minPenetration = penetration;
        bestAxis = nAxis;
    }
    return true;
}

bool PhysicsSystem::checkCollisionSAT(const OBB &a, const OBB &b, glm::vec3 &outNormal, float &outPenetration) {
    float minPenetration = std::numeric_limits<float>::max();
    glm::vec3 bestAxis;

    // Test 3 axes of A
    for (int i = 0; i < 3; i++) {
        if (!TestAxis(a.axes[i], a, b, minPenetration, bestAxis)) return false;
    }

    // Test 3 axes of B
    for (int i = 0; i < 3; i++) {
        if (!TestAxis(b.axes[i], a, b, minPenetration, bestAxis)) return false;
    }

    // Test 9 cross products
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            glm::vec3 axis = glm::cross(a.axes[i], b.axes[j]);
            if (!TestAxis(axis, a, b, minPenetration, bestAxis)) return false;
        }
    }

    outPenetration = minPenetration;

    // Ensure normal points from B to A
    if (glm::dot(bestAxis, a.center - b.center) < 0) {
        outNormal = -bestAxis;
    } else {
        outNormal = bestAxis;
    }

    return true;
}

void PhysicsSystem::resolveCollision(PhysicsObject &a, PhysicsObject &b, const glm::vec3 &normal, float penetration) {
    // Helper for velocity reflection
    auto reflectVelocity = [](RigidBody *rb, float &velocityComponent, float restitution, float sign) {
        // Only reflect if moving towards the collision
        if (velocityComponent * sign < 0) {
            velocityComponent = -velocityComponent * restitution;

            // Stop micro-bouncing (resting contact threshold)
            if (std::abs(velocityComponent) < 0.5f) {
                velocityComponent = 0.0f;
            }
        }
        };

    if (!a.rigidBody->isStatic && !b.rigidBody->isStatic) {
        // Both dynamic: split penetration
        a.gameObject->position += normal * penetration * 0.5f;
        b.gameObject->position -= normal * penetration * 0.5f;

        // Simple velocity reflection along normal
        glm::vec3 vRel = a.rigidBody->velocity - b.rigidBody->velocity;
        float vRelNormal = glm::dot(vRel, normal);

        // Do not resolve if velocities are separating
        if (vRelNormal > 0) return;

        float e = std::min(a.rigidBody->restitution, b.rigidBody->restitution);
        float j = -(1 + e) * vRelNormal;
        j /= (1 / a.rigidBody->mass + 1 / b.rigidBody->mass);

        glm::vec3 impulse = j * normal;
        a.rigidBody->velocity += impulse / a.rigidBody->mass;
        b.rigidBody->velocity -= impulse / b.rigidBody->mass;

    } else if (!a.rigidBody->isStatic) {
        // A is dynamic
        a.gameObject->position += normal * penetration;

        glm::vec3 vRel = a.rigidBody->velocity; // B is static, vB = 0
        float vRelNormal = glm::dot(vRel, normal);

        if (vRelNormal > 0) return;

        float e = std::max(a.rigidBody->restitution, b.rigidBody->restitution);

        // Apply impulse
        glm::vec3 vn = vRelNormal * normal;
        glm::vec3 vt = vRel - vn; // Tangential velocity (friction could be applied here)

        // Debug print
        // std::cout << "Collision! Normal: " << normal.x << ", " << normal.y << ", " << normal.z 
        //           << " vRelNormal: " << vRelNormal 
        //           << " e: " << e << std::endl;

        a.rigidBody->velocity = vt - vn * e;

        // Threshold for resting
        // if (glm::length(a.rigidBody->velocity) < 0.1f) a.rigidBody->velocity = glm::vec3(0.0f);
        if (glm::length(a.rigidBody->velocity) < 0.5f) {
            // Only zero out if normal velocity is small (resting contact)
            if (std::abs(vRelNormal) < 1.0f) {
                a.rigidBody->velocity = glm::vec3(0.0f);
            }
        }

    } else if (!b.rigidBody->isStatic) {
        // B is dynamic
        b.gameObject->position -= normal * penetration;

        glm::vec3 vRel = -b.rigidBody->velocity; // A is static, vA = 0
        float vRelNormal = glm::dot(vRel, normal);

        if (vRelNormal > 0) return;

        float e = std::max(a.rigidBody->restitution, b.rigidBody->restitution);

        glm::vec3 vn = vRelNormal * normal;
        glm::vec3 vt = vRel - vn;

        // Note: vRel was A-B, so -vB. 
        // New vB should be reflected.
        // Let's simplify: just reflect B's velocity along normal
        glm::vec3 vB = b.rigidBody->velocity;
        float vBn = glm::dot(vB, normal);
        glm::vec3 vBnVec = vBn * normal;
        glm::vec3 vBtVec = vB - vBnVec;

        b.rigidBody->velocity = vBtVec - vBnVec * e;

        // if (glm::length(b.rigidBody->velocity) < 0.1f) b.rigidBody->velocity = glm::vec3(0.0f);
        if (glm::length(b.rigidBody->velocity) < 0.5f) {
            if (std::abs(vRelNormal) < 1.0f) {
                b.rigidBody->velocity = glm::vec3(0.0f);
            }
        }
    }
}

RaycastHit PhysicsSystem::raycast(const glm::vec3 &origin, const glm::vec3 &direction, float maxDistance) {
    RaycastHit hit;
    hit.distance = maxDistance;

    glm::vec3 invDir = 1.0f / direction;

    for (auto &obj : physicsObjects) {
        // Ray vs OBB
        // Transform ray to OBB local space
        OBB &obb = obj.worldOBB;

        glm::vec3 p = obb.center - origin;

        glm::vec3 f(
            glm::dot(obb.axes[0], direction),
            glm::dot(obb.axes[1], direction),
            glm::dot(obb.axes[2], direction)
        );

        glm::vec3 e(
            glm::dot(obb.axes[0], p),
            glm::dot(obb.axes[1], p),
            glm::dot(obb.axes[2], p)
        );

        float t[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        // Test against 3 pairs of planes
        float tmin = 0.0f;
        float tmax = maxDistance;

        for (int i = 0; i < 3; i++) {
            float r = obb.halfExtents[i];
            if (std::abs(f[i]) > 1e-6f) {
                float t1 = (e[i] + r) / f[i];
                float t2 = (e[i] - r) / f[i];
                if (t1 > t2) std::swap(t1, t2);
                if (t1 > tmin) tmin = t1;
                if (t2 < tmax) tmax = t2;
                if (tmin > tmax) goto next_obj;
                if (tmax < 0) goto next_obj;
            } else if (-e[i] - r > 0 || -e[i] + r < 0) {
                goto next_obj;
            }
        }

        if (tmin > 0 && tmin < hit.distance) {
            hit.hit = true;
            hit.distance = tmin;
            hit.point = origin + direction * tmin;
            hit.object = obj.gameObject;

            // Calculate normal
            glm::vec3 localPoint = hit.point - obb.center;
            glm::vec3 normal(0.0f);
            float minDepth = std::numeric_limits<float>::max();

            for (int i = 0; i < 3; ++i) {
                float dist = glm::dot(localPoint, obb.axes[i]);
                float depth = obb.halfExtents[i] - std::abs(dist);
                if (depth < minDepth) {
                    minDepth = depth;
                    normal = obb.axes[i] * (dist > 0 ? 1.0f : -1.0f);
                }
            }
            hit.normal = normal;
        }

    next_obj:;
    }

    return hit;
}

bool PhysicsSystem::checkPlayerCollision(const AABB &playerAABB, glm::vec3 &outCorrection, uint32_t playerMask) {
    outCorrection = glm::vec3(0.0f);
    bool collided = false;

    // Convert Player AABB to OBB for SAT check
    glm::vec3 center = (playerAABB.min + playerAABB.max) * 0.5f;
    glm::vec3 extents = (playerAABB.max - playerAABB.min) * 0.5f;
    glm::vec3 axes[3] = { glm::vec3(1,0,0), glm::vec3(0,1,0), glm::vec3(0,0,1) };
    OBB playerOBB(center, axes, extents);

    for (auto &obj : physicsObjects) {
        // Check collision mask
        if ((obj.rigidBody->collisionMask & playerMask) == 0) continue;

        // if (!obj.rigidBody->isStatic) continue; // Allow collision with dynamic objects

        glm::vec3 normal;
        float penetration;
        if (checkCollisionSAT(playerOBB, obj.worldOBB, normal, penetration)) {
            // Accumulate correction? Or just take the largest?
            // For simple character controller, resolving one by one is okay-ish, 
            // but taking the max penetration is safer to avoid jitter.
            // Here we just apply the first one found (simple) or sum them (can be unstable).
            // Let's try resolving immediately.

            // Important: SAT normal points from B to A. 
            // Here A is player, B is obj. So normal points towards player.
            // We want to push player OUT of obj.

            outCorrection += normal * penetration;
            collided = true;

            // Simple interaction: Push dynamic objects
            if (!obj.rigidBody->isStatic) {
                // Apply a small impulse to the object away from the player
                // Normal points from Obj to Player, so -normal is force direction
                glm::vec3 pushForce = -normal * 10.0f;
                pushForce.y = 0.0f; // Keep it horizontal for now to avoid stomping
                obj.rigidBody->addForce(pushForce);

                // Also wake it up if sleeping (not implemented yet, but good practice)
            }
        }
    }
    return collided;
}