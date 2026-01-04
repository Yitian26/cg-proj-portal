#pragma once
#include "GameObject.h"
class Door : public GameObject {
public:
    glm::vec3 initPos{};
    glm::vec3 initRot{};
    bool initCaptured = false;

    void captureInitIfNeeded();
    void reset();
    
    bool isOpen = false;
    float closedYaw = 0.0f;
    float openYawAbs = 90.0f;     // 永远为正：开门幅度
    float speedDegPerSec = 180.0f;

    bool autoChooseDirection = true;

    float chosenOpenYaw = 90.0f;
    float deadZone = 0.05f;
    int lastSide = 1;
    Door(Model* model,
         glm::vec3 pos = glm::vec3(0),
         glm::vec3 rot = glm::vec3(0),
         glm::vec3 scl = glm::vec3(1))
        : GameObject(model, pos, rot, scl) {}

    void toggle(const glm::vec3& playerPos);

    void update(float dt, const Camera& camera) override;
};
