#pragma once

#include "GameObject.h"
#include "FrameBuffer.h"
#include "Camera.h"
#include "Trigger.h"
#include "Texture.h"

#include <memory>
#include <array>

enum portalType {
    PORTAL_A,
    PORTAL_B
};

class Portal : public GameObject {
public:
    bool isActive = true;
    portalType type = PORTAL_A;

    Portal(int width, int height, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 rot = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
    ~Portal();

    void setLinkedPortal(Portal *portal) { linkedPortal = portal; };
    Portal *getLinkedPortal() { return linkedPortal; };

    void setNearTrigger(Trigger *trigger) { nearTrigger = trigger; }
    Trigger *getNearTrigger() { return nearTrigger; }

    void init(struct Scene *scene);
    void checkRaycast(RaycastHit result, glm::vec3 playerRight);

    // Prepares the framebuffer
    void beginRender();
    // Finishes the framebuffer pass
    void endRender(int scrWidth, int scrHeight);
    glm::mat4 getTransformedView(glm::mat4 view);

    // Returns the portal plane equation (Ax + By + Cz + D = 0) in World Space
    glm::vec4 getPlaneEquation();

    void draw(Shader &portalShader, Shader &shader);

    void drawPrev(Shader &portalShader, Shader &shader);

    void drawBuffer(int bufferIndex,Shader &portalShader);

    void DrawFrame(Shader &shader);

    void createFrames(Model *cubeModel, float thickness = 0.05f, float depth = 0.1f);
    void registerFramesPhysics(struct Scene *scene, uint32_t collisionMask = COLLISION_MASK_PORTALFRAME);

    // Update frame transforms to follow portal position/rotation/scale.
    void updateFramesTransform();

private:
    std::unique_ptr<FrameBuffer> frameBuffer[2];
    int currentBuffer = 0;
    Portal *linkedPortal;
    Trigger *nearTrigger;
    Trigger *teleportTrigger;
    GameObject *onObject = nullptr;
    std::array<std::unique_ptr<GameObject>, 4> frames; // 0:top,1:bottom,2:left,3:right
    unsigned int contentVAO, contentVBO;
    std::unique_ptr<Texture> frameTextureA, frameTextureB;
};
