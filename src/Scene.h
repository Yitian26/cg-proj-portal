#pragma once

#include "GameObject.h"
#include "Portal.h"
#include "Model.h"
#include "Skybox.h"
#include "PortalGun.h"
#include "PhysicsSystem.h"
#include "Player.h"
#include "Trigger.h"
#include "Button.h"
#include "Flip.h"

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

        auto button_flip = objects.find("button_flip");
        if (button_flip != objects.end()) {
            Button *btn = dynamic_cast<Button *>(button_flip->second.get());
            if (btn && btn->getIsPressed()) {
                auto flipWall = objects.find("flip_wall");
                if (flipWall != objects.end()) {
                    Flip *flip = dynamic_cast<Flip *>(flipWall->second.get());
                    if (flip) {
                        flip->flip();
                    }
                }
            } else if (btn && btn->getIsReleased()) {
                auto flipWall = objects.find("flip_wall");
                if (flipWall != objects.end()) {
                    Flip *flip = dynamic_cast<Flip *>(flipWall->second.get());
                    if (flip) {
                        flip->reset();
                    }
                }
            }
        }

        auto flip_wall = objects.find("flip_wall");
        if (flip_wall != objects.end()) {
            Flip *flip = dynamic_cast<Flip *>(flip_wall->second.get());
            if (flip && flip->getIsRotating()) {
                if (portalA->getOnObject() == flip) {
                    portalA->setOnObject(nullptr);
                    portalA->isActive = false;
                    portalA->position = glm::vec3(100.0f, 0.0f, 100.0f);
                    portalA->getNearTrigger()->isActive = false;
                    portalA->getTeleportTrigger()->isActive = false;
                    portalB->getNearTrigger()->isActive = false;
                    portalB->getTeleportTrigger()->isActive = false;
                }
                if (portalB->getOnObject() == flip) {
                    portalB->setOnObject(nullptr);
                    portalB->isActive = false;
                    portalB->position = glm::vec3(100.0f, 0.0f, 100.0f);
                    portalA->getNearTrigger()->isActive = false;
                    portalA->getTeleportTrigger()->isActive = false;
                    portalB->getNearTrigger()->isActive = false;
                    portalB->getTeleportTrigger()->isActive = false;
                }
            }
        }
    }
};
