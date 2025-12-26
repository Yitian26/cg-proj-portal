#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <functional>

constexpr uint32_t COLLISION_MASK_DEFAULT = 0xF;
constexpr uint32_t COLLISION_MASK_PORTALFRAME = 0xF0;
constexpr uint32_t COLLISION_MASK_PORTALON = 0x1;
constexpr uint32_t COLLISION_MASK_NEARPORTAL = 0xF2;

// Forward declaration
class GameObject;

// Basic AABB Collider
struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB(const glm::vec3 &min = glm::vec3(0.0f), const glm::vec3 &max = glm::vec3(0.0f))
        : min(min), max(max) {
    }
};

// Oriented Bounding Box
struct OBB {
    glm::vec3 center;
    glm::vec3 axes[3]; // Local x, y, z axes in world space (normalized)
    glm::vec3 halfExtents;

    OBB() {}
    OBB(const glm::vec3 &center, const glm::vec3 axes[3], const glm::vec3 &halfExtents)
        : center(center), halfExtents(halfExtents) {
        this->axes[0] = axes[0];
        this->axes[1] = axes[1];
        this->axes[2] = axes[2];
    }
};

// RigidBody Component
struct RigidBody {
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f);
    glm::vec3 force = glm::vec3(0.0f);
    float mass = 1.0f;
    bool useGravity = true;
    bool isStatic = false;
    bool isCollisionEnabled = true; // Enable/Disable collision response
    float friction = 0.5f;
    float restitution = 0.2f; // Bounciness
    uint32_t collisionMask = 0xFFFFFFFF; // Bitmask for collision filtering

    void addForce(const glm::vec3 &f) {
        force += f;
    }

    void clearForces() {
        force = glm::vec3(0.0f);
    }
};

// Raycast Result
struct RaycastHit {
    bool hit = false;
    float distance = 0.0f;
    glm::vec3 point = glm::vec3(0.0f);
    glm::vec3 normal = glm::vec3(0.0f);
    GameObject *object = nullptr;
};

class PhysicsSystem {
public:
    PhysicsSystem();
    ~PhysicsSystem();

    void setGravity(const glm::vec3 &g);

    struct PhysicsObject {
        GameObject *gameObject;
        RigidBody *rigidBody;
        AABB *collider; // Local space AABB

        // World space OBB cache
        OBB worldOBB;
    };

    void addObject(GameObject *obj, RigidBody *rb, AABB *col);
    void removeObject(GameObject *obj);

    // Character Controller Helper
    // Checks if the given AABB overlaps with any static physics object.
    // Returns true if collision found, and sets correction vector to resolve it.
    bool checkPlayerCollision(const AABB &playerAABB, glm::vec3 &outCorrection, uint32_t playerMask = COLLISION_MASK_DEFAULT);

    void update(float dt);

    // Raycasting
    RaycastHit raycast(const glm::vec3 &origin, const glm::vec3 &direction, float maxDistance);

private:
    glm::vec3 gravity;
    std::vector<PhysicsObject> physicsObjects;

    void integrate(PhysicsObject &obj, float dt);
    void checkCollisions();
    void resolveCollision(PhysicsObject &a, PhysicsObject &b, const glm::vec3 &normal, float penetration);

    // SAT Helper
    bool checkCollisionSAT(const OBB &a, const OBB &b, glm::vec3 &outNormal, float &outPenetration);
};
