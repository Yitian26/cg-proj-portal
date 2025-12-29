#pragma once

#include <glad/gl.h>
#include "GameObject.h"
#include "Camera.h"

class PortalGun : public GameObject {
private:
    glm::mat4 gunModelMatrix;

    // Recoil animation state
    bool isRecoiling = false;
    float recoilTimer = 0.0f;
    const float recoilDuration = 0.1f; // Duration of the recoil back
    const float recoverDuration = 0.2f; // Duration of the recovery forward
    const float maxRecoilDistance = 0.5f; // How far back it goes

public:
    PortalGun(Model *model) : GameObject(model) {
        gunModelMatrix = glm::mat4(1.0f);
    }

    void fire() {
        if (!isRecoiling) {
            isRecoiling = true;
            recoilTimer = 0.0f;
        }
    }

    void update(float dt, const Camera &camera) override {
        float currentRecoil = 0.0f;
        if (isRecoiling) {
            recoilTimer += dt;
            if (recoilTimer < recoilDuration) {
                // Moving back
                float t = recoilTimer / recoilDuration;
                // Ease out cubic
                t = 1.0f - pow(1.0f - t, 3.0f);
                currentRecoil = t * maxRecoilDistance;
            } else if (recoilTimer < recoilDuration + recoverDuration) {
                // Recovering forward
                float t = (recoilTimer - recoilDuration) / recoverDuration;
                // Ease out sine
                currentRecoil = maxRecoilDistance * (1.0f - sin(t * 1.5707963f));
            } else {
                // Finished
                isRecoiling = false;
                recoilTimer = 0.0f;
                currentRecoil = 0.0f;
            }
        }

        glm::vec3 offset = glm::vec3(1.5f, -1.5f, -4.0f + currentRecoil);

        glm::mat4 viewMat = camera.GetViewMatrix();
        glm::mat4 invViewMat = glm::inverse(viewMat);
        glm::mat3 camRotation = glm::mat3(invViewMat);
        glm::vec3 worldOffset = camRotation * offset;
        this->position = camera.Position + worldOffset;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, this->position);
        model = model * glm::mat4(camRotation);
        model = glm::scale(model, this->scale);

        this->gunModelMatrix = model;
    }

    void draw(Shader &shader) override {
        glClear(GL_DEPTH_BUFFER_BIT);//render on top
        if (!model) return;
        shader.setMat4("model", gunModelMatrix);
        model->Draw(shader);
    }
};