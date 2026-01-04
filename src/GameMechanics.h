#pragma once
#include "GameObject.h"
#include "Scene.h"
#include "Trigger.h"
#include <iostream>

// 1. 按钮
class Button : public GameObject {
public:
    bool isPressed = false;
    int pressCount = 0;
    glm::vec3 upPos;          
    glm::vec3 downPos;       
    float speed = 5.0f;

    Button(Model* model, glm::vec3 pos, glm::vec3 scale) 
        : GameObject(model, pos, glm::vec3(0.0f), scale) {
        this->upPos = pos;
        this->downPos = pos - glm::vec3(0.0f, 0.10f, 0.0f); 
    }

    void initTrigger(Scene* scene) {
        glm::vec3 modelCenter = this->model->getCenter();
        glm::vec3 worldCenter = position + (modelCenter * scale);
        worldCenter.y += 1.0f; 

        glm::vec3 size = (this->model->maxBound - this->model->minBound) * scale;
        glm::vec3 halfExtents = size * 0.5f;
        halfExtents.x *= 1.2f;
        halfExtents.z *= 1.2f;
        halfExtents.y = 1.0f; 

        glm::vec3 min = worldCenter - halfExtents;
        glm::vec3 max = worldCenter + halfExtents;
        
        auto trigger = std::make_unique<Trigger>(min, max);

        trigger->onEnter = [this](GameObject* obj) {
            if (obj->name == "player" || obj->name == "companion_cube" || obj->name == "fallingCube") {
                this->pressCount++;
                if (this->pressCount == 1) std::cout << "[Button] Pressed!" << std::endl;
            }
        };

        trigger->onExit = [this](GameObject* obj) {
            if (obj->name == "player" || obj->name == "companion_cube" || obj->name == "fallingCube") {
                this->pressCount--;
                if (this->pressCount < 0) this->pressCount = 0;
                if (this->pressCount == 0) std::cout << "[Button] Released!" << std::endl;
            }
        };

        scene->addTrigger(name + "_trigger", std::move(trigger));
    }

    void update(float dt, const Camera& camera) override {
        isPressed = (pressCount > 0);
        glm::vec3 target = isPressed ? downPos : upPos;
        position = glm::mix(position, target, dt * speed);
    }

    void reset() {
        pressCount = 0;
        isPressed = false;
        position = upPos;
    }
};

// 2. 障碍墙

class Barrier : public GameObject {
public:
    Button* linkedButton = nullptr;
    glm::vec3 closedPos; 
    glm::vec3 openPos;   
    float speed = 2.0f;

    Barrier(Model* model, glm::vec3 pos, glm::vec3 scale, float liftHeight = 10.0f) 
        : GameObject(model, pos, glm::vec3(0.0f), scale) {
        this->closedPos = pos;
        this->openPos = pos + glm::vec3(0.0f, liftHeight, 0.0f); 
    }

    void setLinkedButton(Button* btn) {
        linkedButton = btn;
    }

    void update(float dt, const Camera& camera) override {
        glm::vec3 target = closedPos;
        if (linkedButton && linkedButton->isPressed) {
            target = openPos;
        }
        position = glm::mix(position, target, dt * speed);
    }
};

class Goal : public GameObject {
public:
    bool isWon = false; 

    Goal(Model* model, glm::vec3 pos, glm::vec3 scale) 
        : GameObject(model, pos, glm::vec3(0.0f), scale) {
    }

    void initTrigger(Scene* scene) {
        glm::vec3 modelCenter = this->model->getCenter();
        glm::vec3 worldCenter = position + (modelCenter * scale);
        glm::vec3 size = (this->model->maxBound - this->model->minBound) * scale;
        glm::vec3 halfExtents = size * 0.5f;
        halfExtents.y = 50.0f; 
        glm::vec3 min = worldCenter - halfExtents;
        glm::vec3 max = worldCenter + halfExtents;
        
        auto trigger = std::make_unique<Trigger>(min, max);

        trigger->onEnter = [this](GameObject* obj) {
        //std::cout << "Collision detected with: [" << obj->name << "]" << std::endl;
            if (obj->name == "player") {
                this->isWon = true;
                std::cout << "\n=======================" << std::endl;
                std::cout << "       PASS! WIN!      " << std::endl;
                std::cout << "=======================\n" << std::endl;
            }
        };

        scene->addTrigger(name + "_trigger", std::move(trigger));
    }
};