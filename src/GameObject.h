#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Model.h"
#include "Shader.h"

class GameObject {
public:
    // Transform attributes
    glm::vec3 position;
    glm::vec3 rotation; // Euler angles in degrees
    glm::vec3 scale;

    // Model reference (does not own the model)
    Model *model;

    GameObject(Model *model, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 rot = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f))
        : model(model), position(pos), rotation(rot), scale(scale) {
    }

    virtual ~GameObject() = default;

    // Core logic update
    virtual void update(float dt) {
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
};
