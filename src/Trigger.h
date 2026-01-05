#pragma once
#include "PhysicsSystem.h"
#include "Shader.h"

#include <unordered_set>
#include <functional>

#include <glm/glm.hpp>

class GameObject;

class Trigger {
public:
    Trigger(const OBB &obb);
    Trigger(const glm::vec3 &min, const glm::vec3 &max) {
        glm::vec3 center = (min + max) * 0.5f;
        glm::vec3 halfExtents = (max - min) * 0.5f;
        glm::vec3 axes[3] = {
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        };
        bounds = OBB(center, axes, halfExtents);
    }

    OBB bounds;
    bool isActive = true;

    // Callbacks
    // Pass the object pointer that triggered it.
    std::function<void(GameObject *obj)> onEnter;
    std::function<void(GameObject *obj)> onExit;
    std::function<void(GameObject *obj)> onInside;

    // Check a specific object
    void check(GameObject *obj);
    void drawOBBDebug(Shader &shader);

    // Reset the OBB bounds
    void setBounds(const OBB &obb) { bounds = obb; }

    // Helper to set OBB from center, axes and half extents
    void setFromCenterAxesExtents(const glm::vec3 &center, const glm::vec3 axes[3], const glm::vec3 &halfExtents) {
        bounds = OBB(center, axes, halfExtents);
    }

private:
    std::unordered_set<GameObject *> objectsInside;

    bool isPointInside(const glm::vec3 &point) const;
};
