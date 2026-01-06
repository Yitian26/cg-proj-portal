#pragma once

#include "GameObject.h"
#include "Trigger.h"

#include <memory>

class Button : public GameObject {
public:
    Button(Model *model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);

    // Creates and returns the trigger associated with this button.
    // The caller (Scene/Application) should take ownership of the trigger.
    std::unique_ptr<Trigger> createTrigger();

    void update(float dt, const Camera &camera) override;

    bool getIsPressed();

    bool getIsReleased();

private:
    glm::vec3 initialPosition;
    glm::vec3 pressedPosition;
    bool lastPressedState = false;
    bool isPressed = false;
    int objectsOnButton = 0;
    float pressSpeed = 1.0f; // Units per second
};
