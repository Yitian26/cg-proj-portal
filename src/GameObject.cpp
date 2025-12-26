#include "GameObject.h"
#include "PhysicsSystem.h"

GameObject::GameObject(Model *model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
    : model(model), position(pos), rotation(rot), scale(scale) {
}

GameObject::~GameObject() = default;

void GameObject::setScaleToSizeX(float sizeX) {
    if (!model) return;
    float originalWidth = model->maxBound.x - model->minBound.x;
    if (originalWidth < 1e-6f) return;
    float factor = sizeX / originalWidth;
    scale = glm::vec3(factor);
}

void GameObject::setCollisionMask(uint32_t mask) {
    if (rigidBody) {
        rigidBody->collisionMask = mask;
    }
}
