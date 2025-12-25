#include "GameObject.h"
#include "PhysicsSystem.h"

GameObject::GameObject(Model *model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
    : model(model), position(pos), rotation(rot), scale(scale) {
}

GameObject::~GameObject() = default;
