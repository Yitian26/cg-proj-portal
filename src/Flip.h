#pragma once

#include "GameObject.h"
#include <glm/glm.hpp>

class Flip : public GameObject {
public:
    Flip(Model *model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);

    void flip();
    void reset();

    void update(float dt, const Camera &camera) override;
    void setPivot(glm::vec3 pivot) { pivotLocal = pivot; };
    void setPosition(glm::vec3 pos) { initialPosition = pos; position = pos; };
    void setFlipAngle(float angle);
    void setSpeed(float speed);
    bool getIsRotating() const { return isRotating; }

private:
    glm::vec3 initialPosition;
    glm::vec3 initialRotation;

    glm::vec3 pivotLocal;
    glm::vec3 rotationAxis;
    float maxAngle = 45.0f;
    float speed = 90.0f;

    float currentAngle = 0.0f;
    float targetAngle = 0.0f;
    bool isRotating = false;
};
