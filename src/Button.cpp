#include "Button.h"

#include <iostream>

Button::Button(Model *model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
    : GameObject(model, pos, rot, scale), initialPosition(pos) {
    // Button presses down 0.05 units
    pressedPosition = pos - glm::vec3(0.0f, 0.05f, 0.0f);
}

std::unique_ptr<Trigger> Button::createTrigger() {
    glm::vec3 min = model->minBound * scale;
    glm::vec3 max = model->maxBound * scale;

    glm::vec3 center = initialPosition + (min + max) * 0.5f;
    glm::vec3 size = max - min;

    glm::vec3 triggerHalfExtents = size * 0.5f;
    triggerHalfExtents.y = 1.0f; // Height of trigger area

    float topSurfaceY = initialPosition.y + max.y;
    glm::vec3 triggerCenter = center;
    triggerCenter.y = topSurfaceY + triggerHalfExtents.y * 0.5f;

    glm::vec3 axes[3] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };

    auto trigger = std::make_unique<Trigger>(
        triggerCenter - triggerHalfExtents,
        triggerCenter + triggerHalfExtents
    );

    trigger->onEnter = [this](GameObject *obj) {
        this->objectsOnButton++;
        };

    trigger->onExit = [this](GameObject *obj) {
        this->objectsOnButton--;
        if (this->objectsOnButton < 0) this->objectsOnButton = 0;
    };

    return trigger;
}

void Button::update(float dt, const Camera &camera) {
    lastPressedState = isPressed;
    isPressed = (objectsOnButton > 0);
    glm::vec3 target = isPressed ? pressedPosition : initialPosition;

    glm::vec3 diff = target - position;
    float dist = glm::length(diff);

    if (dist > 0.0001f) {
        float moveStep = pressSpeed * dt;
        if (moveStep >= dist) {
            position = target;
        } else {
            position += glm::normalize(diff) * moveStep;
        }
    }
}

bool Button::getIsPressed() {
    return (lastPressedState != isPressed && isPressed);
}

bool Button::getIsReleased() {
    return (lastPressedState != isPressed && !isPressed);
}
