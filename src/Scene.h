#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include "GameObject.h"
#include "Portal.h"
#include "Model.h"
#include "Skybox.h"
#include "PortalGun.h"
#include "PhysicsSystem.h"

struct Scene {
    // Resource Management
    std::unordered_map<std::string, std::unique_ptr<Model>> modelResources;

    // Scene Graph
    std::unordered_map<std::string, std::unique_ptr<GameObject>> objects;

    // Special Objects
    std::unique_ptr<Portal> portalA;
    std::unique_ptr<Portal> portalB;
    std::unique_ptr<Skybox> skybox;
    std::unique_ptr<PortalGun> portalGun;

    // Physics
    std::unique_ptr<PhysicsSystem> physicsSystem;

    Scene() {
        physicsSystem = std::make_unique<PhysicsSystem>();
    }

    // Lighting
    glm::vec3 lightPos;

    void addModelResource(const std::string &name, std::unique_ptr<Model> model) {
        if (modelResources.count(name)) {
            printf("Model resource %s already exists!\n", name.c_str());
            return;
        }
        modelResources[name] = std::move(model);
    }

    void addObject(std::string name, std::unique_ptr<GameObject> obj) {
        if (objects.count(name)) {
            printf("GameObject %s already exists!\n", name.c_str());
            return;
        }
        objects[name] = std::move(obj);
    }

    void addPhysics(GameObject *obj, bool isStatic, float mass = 1.0f, float restitution = 0.2f) {
        obj->rigidBody = std::make_unique<RigidBody>();
        obj->rigidBody->isStatic = isStatic;
        obj->rigidBody->mass = mass;
        obj->rigidBody->restitution = restitution;

        // Expand collider slightly to be larger than the visual model
        glm::vec3 padding(0.15f);
        obj->collider = std::make_unique<AABB>(obj->model->minBound - padding, obj->model->maxBound + padding);

        physicsSystem->addObject(obj, obj->rigidBody.get(), obj->collider.get());
    }

    void update(float dt, const Camera &camera) {
        // Update Physics
        if (physicsSystem) {
            physicsSystem->update(dt);
        }

        for (auto &pair : objects) {
            pair.second->update(dt, camera);
        }
        if (portalGun) {
            portalGun->update(dt, camera);
        }
    }
};
