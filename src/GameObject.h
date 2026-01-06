#pragma once

#include "Model.h"
#include "Shader.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Forward declaration to avoid circular dependency if PhysicsSystem includes GameObject
struct RigidBody;
struct AABB;

class GameObject {
public:
    std::string name;
    bool isTeleportable = false;
    bool canOpenPortal = false;
    // Transform attributes
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f); // Euler angles in degrees
    glm::vec3 scale = glm::vec3(1.0f);

    // Model reference (does not own the model)
    Model *model;

    // Physics components (owned by GameObject for now, but managed by PhysicsSystem)
    std::unique_ptr<RigidBody> rigidBody;
    std::unique_ptr<AABB> collider;

    GameObject(Model *model, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 rot = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));

    virtual ~GameObject();

    // Core logic update
    virtual void update(float dt, const Camera &camera) {
        // Default object has no logic
    }

    // Core render logic
    virtual void draw(Shader &shader) {
        if (!model) return;

        // 1. Calculate Model Matrix
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, scale);

        // 2. Pass to Shader
        shader.setMat4("model", modelMatrix);

        // 3. Draw model
        model->Draw(shader);
    }

    // Helper to set uniform scale based on desired X-axis size
    void setScaleToSizeX(float sizeX);

    // Helper to set collision mask
    void setCollisionMask(uint32_t mask);

    // Helper to enable/disable collision
    void setCollisionEnabled(bool enabled);
};
