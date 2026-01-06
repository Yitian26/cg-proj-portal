#include "Flip.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

Flip::Flip(Model *model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
    : GameObject(model, pos, rot, scale) {
    initialPosition = pos;
    initialRotation = rot;

    glm::quat qx = glm::angleAxis(glm::radians(initialRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat qy = glm::angleAxis(glm::radians(initialRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat qz = glm::angleAxis(glm::radians(initialRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::quat qInit = qx * qy * qz;

    rotationAxis = qInit * glm::vec3(0.0f, 0.0f, 1.0f);
    maxAngle = 45.0f;

    glm::vec3 center = model->getCenter();
    pivotLocal = center;
}

void Flip::setFlipAngle(float angle) {
    maxAngle = angle;
}

void Flip::setSpeed(float s) {
    speed = s;
}

void Flip::flip() {
    targetAngle = maxAngle;
}

void Flip::reset() {
    targetAngle = 0.0f;
}

void Flip::update(float dt, const Camera &camera) {
    if (std::abs(currentAngle - targetAngle) < 0.01f) {
        currentAngle = targetAngle;
        canOpenPortal = true;
        isRotating = false;
    } else {
        canOpenPortal = false;
        isRotating = true;
        float step = speed * dt;
        if (currentAngle < targetAngle) {
            currentAngle += step;
            if (currentAngle > targetAngle) currentAngle = targetAngle;
        } else {
            currentAngle -= step;
            if (currentAngle < targetAngle) currentAngle = targetAngle;
        }
    }

    glm::mat4 initModel = glm::mat4(1.0f);
    initModel = glm::translate(initModel, initialPosition);
    initModel = glm::rotate(initModel, glm::radians(initialRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    initModel = glm::rotate(initModel, glm::radians(initialRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    initModel = glm::rotate(initModel, glm::radians(initialRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec3 pivotWorld = initialPosition +
        glm::quat(glm::radians(initialRotation)) * (pivotLocal * scale);

    // Rotation Quaternion
    glm::quat qRot = glm::angleAxis(glm::radians(currentAngle), rotationAxis);

    // New Rotation
    // R_new = qRot * R_init
    glm::quat qInit = glm::quat(glm::radians(initialRotation));
    glm::quat qFinal = qRot * qInit;

    glm::vec3 centerNew = pivotWorld + qRot * (initialPosition - pivotWorld);
    this->position = centerNew;
    this->rotation = initialRotation + rotationAxis * currentAngle;
}
