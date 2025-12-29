#pragma once

#include "GameObject.h"
#include "Portal.h"
#include "Model.h"
#include "Skybox.h"
#include "PortalGun.h"
#include "PhysicsSystem.h"
#include "Player.h"
#include "Trigger.h"

#include <vector>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>


struct Scene {
    // Resource Management
    std::unordered_map<std::string, std::unique_ptr<Model>> modelResources;

    // Scene Graph
    std::unordered_map<std::string, std::unique_ptr<GameObject>> objects;
    std::unordered_map<std::string, std::unique_ptr<Trigger>> triggers;

    // Special Objects
    std::unique_ptr<Portal> portalA;
    std::unique_ptr<Portal> portalB;
    std::unique_ptr<Skybox> skybox;
    std::unique_ptr<PortalGun> portalGun;
    std::unique_ptr<Player> player;

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
        obj->name = name;
        objects[name] = std::move(obj);
    }

    void addPhysics(GameObject *obj, bool isStatic, uint32_t collisionMask = COLLISION_MASK_DEFAULT, float mass = 1.0f, float restitution = 0.2f, float friction = 0.5f) {
        obj->rigidBody = std::make_unique<RigidBody>();
        obj->rigidBody->isStatic = isStatic;
        obj->rigidBody->mass = mass;
        obj->rigidBody->restitution = restitution;
        obj->rigidBody->friction = friction;
        obj->rigidBody->collisionMask = collisionMask;
        if (!obj->collider) {
            obj->collider = std::make_unique<AABB>(obj->model->minBound, obj->model->maxBound);
        }
        physicsSystem->addObject(obj, obj->rigidBody.get(), obj->collider.get());
    }

    void addTrigger(std::string name, std::unique_ptr<Trigger> trigger) {
        if (triggers.count(name)) {
            printf("Trigger %s already exists!\n", name.c_str());
            return;
        }
        triggers[name] = std::move(trigger);
    }

    void update(float dt, const Camera &camera) {
        // Update Physics
        if (physicsSystem) {
            physicsSystem->update(dt);
        }

        if (player) {
            player->update(dt, physicsSystem.get());
        }

        for (auto &pair : objects) {
            pair.second->update(dt, camera);
        }

        if (portalGun) {
            portalGun->update(dt, camera);
        }

        for (auto &pair : triggers) {
            if (player) {
                pair.second->check(player.get());
            }
            for (auto &objPair : objects) {
                pair.second->check(objPair.second.get());
            }
        }
    }
};
