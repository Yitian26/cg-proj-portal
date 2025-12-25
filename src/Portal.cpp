#include "Portal.h"

Portal::Portal(Model *model, int width, int height, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
    : GameObject(model, pos, rot, scale), linkedPortal(nullptr) {
    frameBuffer[0] = std::make_unique<FrameBuffer>(width, height);
    frameBuffer[1] = std::make_unique<FrameBuffer>(width, height);
}

void Portal::setLinkedPortal(Portal *portal) {
    linkedPortal = portal;
}

Portal *Portal::getLinkedPortal() const {
    return linkedPortal;
}

void Portal::beginRender() {
    currentBuffer = (currentBuffer + 1) % 2;
    frameBuffer[currentBuffer]->Bind();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

glm::mat4 Portal::getTransformedView(glm::mat4 view) {
    if (!linkedPortal) return view;
    // 1. Get Model Matrices
    glm::mat4 myModel = glm::mat4(1.0f);
    myModel = glm::translate(myModel, position);
    myModel = glm::rotate(myModel, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    myModel = glm::rotate(myModel, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    myModel = glm::rotate(myModel, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 otherModel = glm::mat4(1.0f);
    otherModel = glm::translate(otherModel, linkedPortal->position);
    otherModel = glm::rotate(otherModel, glm::radians(linkedPortal->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    otherModel = glm::rotate(otherModel, glm::radians(linkedPortal->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    otherModel = glm::rotate(otherModel, glm::radians(linkedPortal->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // 2. Calculate Relative Transform (Camera -> Me)

    glm::mat4 camTransform = glm::inverse(view);

    // Transformation from this portal's local space to the other portal's local space

    // Step 1: Transform Camera to My Local Space
    glm::mat4 camInLocal = glm::inverse(myModel) * camTransform;

    // Step 2: Rotate 180 degrees around Y (to face out)

    glm::mat4 rotation180 = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 destView = otherModel * rotation180 * glm::inverse(myModel) * camTransform;

    return glm::inverse(destView);
}

void Portal::endRender(int scrWidth, int scrHeight) {
    frameBuffer[currentBuffer]->Unbind();
    glViewport(0, 0, scrWidth, scrHeight);
}

glm::vec4 Portal::getPlaneEquation() {
    if (!linkedPortal) return glm::vec4(0.0f);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, linkedPortal->position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(linkedPortal->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(linkedPortal->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(linkedPortal->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 normal = glm::vec3(modelMatrix * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
    normal = glm::normalize(normal);

    // Plane equation: Ax + By + Cz + D = 0
    // D = -dot(N, P)
    float d = -glm::dot(normal, linkedPortal->position);

    return glm::vec4(normal, d);
}

void Portal::draw(Shader &shader) {
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, frameBuffer[currentBuffer]->GetTextureID());
    shader.setInt("reflectionTexture", 10);

    GameObject::draw(shader);

    glActiveTexture(GL_TEXTURE0);
}

void Portal::drawPrev(Shader &shader) {
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, frameBuffer[(currentBuffer+1) % 2]->GetTextureID());
    shader.setInt("reflectionTexture", 10);

    GameObject::draw(shader);

    glActiveTexture(GL_TEXTURE0);
}

unsigned int Portal::getTextureID() const {
    return frameBuffer[(currentBuffer+1) % 2]->GetTextureID();
}
