#pragma once

#include "GameObject.h"
#include "FrameBuffer.h"
#include "Camera.h"
#include "Trigger.h"
#include <memory>
#include <array>

class Portal : public GameObject {
public:
    Portal(Model *model, int width, int height, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 rot = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));

    void setLinkedPortal(Portal *portal) { linkedPortal = portal; };
    Portal *getLinkedPortal() { return linkedPortal; };

    void setNearTrigger(Trigger *trigger) { nearTrigger = trigger; }
    Trigger *getNearTrigger() { return nearTrigger; }

    void init(struct Scene *scene);
    void checkRaycast(RaycastHit result);

    // Prepares the framebuffer
    void beginRender();
    // Finishes the framebuffer pass
    void endRender(int scrWidth, int scrHeight);
    glm::mat4 getTransformedView(glm::mat4 view);

    // Returns the portal plane equation (Ax + By + Cz + D = 0) in World Space
    glm::vec4 getPlaneEquation();

    void draw(Shader &shader) override;

    void drawPrev(Shader &shader);

    unsigned int getTextureID() const;

    // Frame (door-frame) management. Portal owns these frames.
    // Call `createFrames` after portal construction to create the 4 frame pieces.
    // Then call `registerFramesPhysics(scene)` from Application to register them
    // with the physics system (they remain owned by the Portal).
    void createFrames(Model *cubeModel, float thickness = 0.05f, float depth = 0.1f);
    void registerFramesPhysics(struct Scene *scene, uint32_t collisionMask = COLLISION_MASK_DEFAULT);

    // Update frame transforms to follow portal position/rotation/scale.
    void updateFramesTransform();

private:
    std::unique_ptr<FrameBuffer> frameBuffer[2];
    int currentBuffer = 0;
    bool isActive = true;
    Portal *linkedPortal;
    Trigger *nearTrigger;
    Trigger *teleportTrigger;
    GameObject *onObject = nullptr;
    std::array<std::unique_ptr<GameObject>, 4> frames; // 0:top,1:bottom,2:left,3:right
};
