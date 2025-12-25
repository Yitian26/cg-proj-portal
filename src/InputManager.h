#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include <functional>

class InputManager {
public:
    InputManager();
    ~InputManager();

    void initialize(GLFWwindow *window);
    void update();

    // Keyboard
    bool isKeyDown(int key) const;
    bool isKeyPressed(int key) const;
    bool isKeyReleased(int key) const;

    // Mouse buttons
    bool isMouseDown(int button) const;
    bool isMousePressed(int button) const;
    bool isMouseReleased(int button) const;

    // Mouse movement / scroll
    void getMouseDelta(double &dx, double &dy) const;
    double getScrollY() const;

    // Optional: register a callback for a key press edge
    void setKeyCallback(int key, std::function<void()> cb);

private:
    GLFWwindow *window = nullptr;
    std::vector<char> currKeys;
    std::vector<char> prevKeys;

    std::vector<char> currMouse;
    std::vector<char> prevMouse;

    double mouseLastX = 0.0;
    double mouseLastY = 0.0;
    double mouseDX = 0.0;
    double mouseDY = 0.0;
    double scrollY = 0.0;

    std::vector<std::function<void()>> keyCallbacks; // indexed by key (size = GLFW_KEY_LAST+1)
};
