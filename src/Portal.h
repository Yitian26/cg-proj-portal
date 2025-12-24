#pragma once

#include "GameObject.h"
#include "FrameBuffer.h"
#include "Camera.h"
#include <memory>

class Portal : public GameObject {
public:
    Portal(Model *model, int width, int height, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 rot = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));

    void setLinkedPortal(Portal *portal);
    Portal *getLinkedPortal() const;

    // Prepares the framebuffer and returns the view matrix for the portal pass
    glm::mat4 beginRender(Camera &camera);
    // Finishes the framebuffer pass
    void endRender(int scrWidth, int scrHeight);

    // Returns the portal plane equation (Ax + By + Cz + D = 0) in World Space
    glm::vec4 getPlaneEquation();

    void draw(Shader &shader) override;

    unsigned int getTextureID() const;

private:
    std::unique_ptr<FrameBuffer> frameBuffer;
    Portal *linkedPortal;
};
