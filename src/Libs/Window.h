#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window {
public:
    Window(int width, int height, const char* title);
    ~Window();

    bool isOpen() const;
    void beginFrame(float r, float g, float b, float a = 1.0f) const;
    void endFrame() const;

    GLFWwindow* native() const;
    int width() const;
    int height() const;

private:
    GLFWwindow* m_window = nullptr;
    int m_width = 0;
    int m_height = 0;
};
