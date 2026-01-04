#include "Door.h"
#include <algorithm>
#include <cmath>
static float moveTowards(float current, float target, float maxDelta) {
    float delta = target - current;
    if (std::fabs(delta) <= maxDelta) return target;
    return current + (delta > 0 ? 1.0f : -1.0f) * maxDelta;
}

void Door::toggle(const glm::vec3& playerPos) {
    if (!isOpen) {
        float yawRad = glm::radians(closedYaw);
        glm::vec3 forward = glm::normalize(glm::vec3(std::sin(yawRad), 0.0f, -std::cos(yawRad)));

        glm::vec3 toPlayer = playerPos - position;
        toPlayer.y = 0.0f;
        if (glm::length(toPlayer) > 1e-6f) toPlayer = glm::normalize(toPlayer);

        float s = glm::dot(forward, toPlayer);

        int side;
        if (std::fabs(s) < deadZone) {
            side = lastSide;
        } else {
            side = (s >= 0.0f) ? 1 : -1;
            lastSide = side;
        }
        chosenOpenYaw = closedYaw + openYawAbs;
    }

    isOpen = !isOpen;
}
void Door::update(float dt, const Camera& camera) {
    captureInitIfNeeded();
    float target = isOpen ? chosenOpenYaw : closedYaw;
    rotation.y = moveTowards(rotation.y, target, speedDegPerSec * dt);
}

void Door::captureInitIfNeeded() {
    if (initCaptured) return;
    initPos = position;
    initRot = rotation;
    closedYaw = rotation.y;              
    chosenOpenYaw = closedYaw + openYawAbs;
    isOpen = false;
    initCaptured = true;
}

void Door::reset() {
    captureInitIfNeeded();

    position = initPos;
    rotation = initRot;

    closedYaw = rotation.y;
    chosenOpenYaw = closedYaw + openYawAbs;
    isOpen = false;
    lastSide = 1;
}