#include "InputManager.h"
#include <algorithm>
#include <iostream>

InputManager::InputManager() {}
InputManager::~InputManager() {}

void InputManager::initialize(GLFWwindow *wnd) {
    window = wnd;
    int keyCount = GLFW_KEY_LAST + 1;
    currKeys.assign(keyCount, 0);
    prevKeys.assign(keyCount, 0);

    int mouseCount = GLFW_MOUSE_BUTTON_LAST + 1;
    currMouse.assign(mouseCount, 0);
    prevMouse.assign(mouseCount, 0);

    keyCallbacks.resize(keyCount);

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    mouseLastX = x; mouseLastY = y;
}

void InputManager::update() {
    if (!window) return;

    prevKeys = currKeys;
    prevMouse = currMouse;

    for (int k = 0; k <= GLFW_KEY_LAST; ++k) {
        int state = glfwGetKey(window, k);
        currKeys[k] = (state == GLFW_PRESS || state == GLFW_REPEAT) ? 1 : 0;
    }

    for (int b = 0; b <= GLFW_MOUSE_BUTTON_LAST; ++b) {
        int s = glfwGetMouseButton(window, b);
        currMouse[b] = (s == GLFW_PRESS) ? 1 : 0;
    }

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    mouseDX = x - mouseLastX;
    mouseDY = y - mouseLastY;
    mouseLastX = x; mouseLastY = y;

    // trigger callbacks for pressed edges
    for (int k = 0; k <= GLFW_KEY_LAST; ++k) {
        if (isKeyPressed(k) && keyCallbacks[k]) {
            keyCallbacks[k]();
        }
    }
}

bool InputManager::isKeyDown(int key) const {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return currKeys[key];
}
bool InputManager::isKeyPressed(int key) const {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return currKeys[key] && !prevKeys[key];
}
bool InputManager::isKeyReleased(int key) const {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return !currKeys[key] && prevKeys[key];
}

bool InputManager::isMouseDown(int button) const {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return currMouse[button];
}
bool InputManager::isMousePressed(int button) const {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return currMouse[button] && !prevMouse[button];
}
bool InputManager::isMouseReleased(int button) const {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return !currMouse[button] && prevMouse[button];
}

void InputManager::getMouseDelta(double &dx, double &dy) const {
    dx = mouseDX; dy = mouseDY;
}
double InputManager::getScrollY() const { return scrollY; }

void InputManager::setKeyCallback(int key, std::function<void()> cb) {
    if (key < 0 || key > GLFW_KEY_LAST) return;
    keyCallbacks[key] = std::move(cb);
}
