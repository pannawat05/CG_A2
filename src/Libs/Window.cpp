#include "Window.h"

#include <iostream>
#include <stdexcept>

Window::Window(int width, int height, const char* title)
    : m_width(width), m_height(height)
{
    if (!glfwInit()) {
        throw std::runtime_error("GLFW initialization failed.");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    m_window = glfwCreateWindow(m_width, m_height, title, nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Could not create GLFW window.");
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    const GLenum glewResult = glewInit();
    glGetError(); // GLEW may generate a harmless GL_INVALID_ENUM on macOS.

    if (glewResult != GLEW_OK) {
        const char* message =
            reinterpret_cast<const char*>(glewGetErrorString(glewResult));
        glfwDestroyWindow(m_window);
        glfwTerminate();
        throw std::runtime_error(
            std::string("GLEW initialization failed: ") +
            (message ? message : "unknown error")
        );
    }

    glViewport(0, 0, m_width, m_height);
    glEnable(GL_DEPTH_TEST);

    glfwSetFramebufferSizeCallback(
        m_window,
        [](GLFWwindow*, int w, int h) {
            glViewport(0, 0, w, h);
        }
    );

    std::cout << "OpenGL version: "
              << reinterpret_cast<const char*>(glGetString(GL_VERSION))
              << "\n";
    std::cout << "GLSL version: "
              << reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))
              << "\n";
}

Window::~Window()
{
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

bool Window::isOpen() const
{
    return m_window && !glfwWindowShouldClose(m_window);
}

void Window::beginFrame(float r, float g, float b, float a) const
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::endFrame() const
{
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

GLFWwindow* Window::native() const
{
    return m_window;
}

int Window::width() const
{
    return m_width;
}

int Window::height() const
{
    return m_height;
}
