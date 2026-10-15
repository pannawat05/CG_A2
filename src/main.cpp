#include "Libs/Mesh.h"
#include "Libs/Shader.h"
#include "Libs/Window.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

struct CameraState {
    glm::vec3 target = glm::vec3(-0.2f, 0.35f, 0.0f);
    float yaw = glm::radians(8.5f);
    float pitch = glm::radians(16.5f);
    float distance = 9.0f;
    double lastX = 0.0;
    double lastY = 0.0;
    bool dragging = false;
};

static glm::vec3 cameraPosition(const CameraState& camera)
{
    const float horizontal = std::cos(camera.pitch);
    return camera.target + camera.distance * glm::vec3(
        horizontal * std::sin(camera.yaw),
        std::sin(camera.pitch),
        horizontal * std::cos(camera.yaw)
    );
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    auto* camera = static_cast<CameraState*>(glfwGetWindowUserPointer(window));
    camera->dragging = action == GLFW_PRESS;
    if (camera->dragging) {
        glfwGetCursorPos(window, &camera->lastX, &camera->lastY);
    }
}

static void cursorPositionCallback(GLFWwindow* window, double x, double y)
{
    auto* camera = static_cast<CameraState*>(glfwGetWindowUserPointer(window));
    if (!camera->dragging) return;

    constexpr float sensitivity = 0.006f;
    camera->yaw -= static_cast<float>(x - camera->lastX) * sensitivity;
    camera->pitch += static_cast<float>(y - camera->lastY) * sensitivity;
    camera->pitch = std::clamp(camera->pitch, glm::radians(-5.0f), glm::radians(75.0f));
    camera->lastX = x;
    camera->lastY = y;
}

static void scrollCallback(GLFWwindow* window, double, double yOffset)
{
    auto* camera = static_cast<CameraState*>(glfwGetWindowUserPointer(window));
    camera->distance = std::clamp(
        camera->distance * std::exp(static_cast<float>(-yOffset) * 0.12f),
        3.2f,
        14.0f
    );
}

static void drawBox(const Mesh& cube,
                    const Shader& shader,
                    const glm::vec3& position,
                    const glm::vec3& scale,
                    const glm::vec3& color,
                    float rotationY = 0.0f,
                    float rotationZ = 0.0f)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, rotationY, glm::vec3(0, 1, 0));
    model = glm::rotate(model, rotationZ, glm::vec3(0, 0, 1));
    model = glm::scale(model, scale);

    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    cube.draw();
}

    static void drawSphere(const Mesh& sphere,
                   const Shader& shader,
                   const glm::vec3& position,
                   const glm::vec3& scale,
                   const glm::vec3& color,
                   float rotationZ = 0.0f)
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, rotationZ, glm::vec3(0, 0, 1));
        model = glm::scale(model, scale);

        shader.setMat4("model", model);
        shader.setVec3("objectColor", color);
        sphere.draw();
    }

static void drawMug(const Mesh& cube, const Shader& shader,
                    const glm::vec3& base, const glm::vec3& color)
{
    drawBox(cube, shader, base + glm::vec3(0.0f, 0.085f, 0.0f),
            glm::vec3(0.07f, 0.085f, 0.07f), color);
    drawBox(cube, shader, base + glm::vec3(0.0f, 0.165f, 0.0f),
            glm::vec3(0.056f, 0.008f, 0.056f), glm::vec3(0.20f, 0.10f, 0.05f));
    drawBox(cube, shader, base + glm::vec3(0.09f, 0.085f, 0.0f),
            glm::vec3(0.022f, 0.045f, 0.012f), color);
}

static void drawSteam(const Mesh& sphere, const Shader& shader,
                      const glm::vec3& top, float time, float seed)
{
    for (int i = 0; i < 3; ++i) {
        const float phase = std::fmod(time * 0.45f + seed + i / 3.0f, 1.0f);
        const float size = 0.034f * (1.0f - 0.55f * phase);
        drawSphere(sphere, shader,
                   top + glm::vec3(0.025f * std::sin(phase * 9.0f + i),
                                   0.04f + phase * 0.38f, 0.0f),
                   glm::vec3(size), glm::vec3(0.82f, 0.84f, 0.88f));
    }
}

static void drawPaperStack(const Mesh& cube, const Shader& shader,
                           const glm::vec3& center, int sheets, float lean)
{
    const glm::vec3 paper[] = {
        glm::vec3(0.93f, 0.93f, 0.90f),
        glm::vec3(0.85f, 0.90f, 0.95f),
        glm::vec3(0.95f, 0.88f, 0.80f)
    };
    for (int i = 0; i < sheets; ++i) {
        const float x = center.x + lean * i + 0.012f * std::sin(i * 1.7f);
        const float z = center.z + 0.010f * std::cos(i * 2.1f);
        drawBox(cube, shader,
                glm::vec3(x, center.y + 0.011f + i * 0.024f, z),
                glm::vec3(0.19f, 0.011f, 0.14f),
                paper[i % 3], 0.12f * std::sin(i * 2.3f));
    }
}

static void drawFunnyProps(const Mesh& cube, const Mesh& sphere,
                           const Shader& shader, float time)
{
    const float deskTop = 0.14f;
    const glm::vec3 white(0.90f, 0.90f, 0.88f);
    const glm::vec3 paperWhite(0.93f, 0.93f, 0.90f);
    const glm::vec3 black(0.03f, 0.03f, 0.04f);
    const glm::vec3 hair(0.045f, 0.075f, 0.095f);

    drawPaperStack(cube, shader, glm::vec3(-1.52f, deskTop, -0.25f), 18, 0.004f);
    drawPaperStack(cube, shader, glm::vec3(1.45f, deskTop, -0.30f), 8, -0.006f);

    const float floorY = -1.146f;
    const float sheets[][3] = {
        {-1.20f, 1.20f, 0.6f}, {-0.10f, 1.55f, -0.4f}, {0.50f, 1.10f, 1.1f},
        {-1.70f, 0.80f, 0.3f}, {0.15f, 2.00f, 0.9f}, {-0.80f, 1.85f, -0.7f}
    };
    for (const auto& s : sheets) {
        drawBox(cube, shader, glm::vec3(s[0], floorY, s[1]),
                glm::vec3(0.14f, 0.002f, 0.10f), paperWhite, s[2]);
    }

    drawBox(cube, shader, glm::vec3(0.75f, deskTop + 0.035f, 0.10f),
            glm::vec3(0.30f, 0.035f, 0.28f), glm::vec3(0.62f, 0.45f, 0.28f));
    drawSphere(sphere, shader, glm::vec3(0.88f, deskTop + 0.072f, 0.20f),
               glm::vec3(0.07f, 0.003f, 0.055f), glm::vec3(0.40f, 0.22f, 0.10f));
    drawSphere(sphere, shader, glm::vec3(0.72f, deskTop + 0.072f, 0.30f),
               glm::vec3(0.06f, 0.003f, 0.05f), glm::vec3(0.40f, 0.22f, 0.10f));

    drawMug(cube, shader, glm::vec3(-1.45f, deskTop, 0.60f), white);
    drawMug(cube, shader, glm::vec3(-1.28f, deskTop, 0.12f), glm::vec3(0.95f, 0.80f, 0.25f));
    drawMug(cube, shader, glm::vec3(1.35f, deskTop, 0.50f), glm::vec3(0.90f, 0.45f, 0.60f));
    drawMug(cube, shader, glm::vec3(1.55f, deskTop, 0.18f), glm::vec3(0.30f, 0.50f, 0.85f));
    drawMug(cube, shader, glm::vec3(0.62f, deskTop + 0.07f, 0.05f), glm::vec3(0.80f, 0.25f, 0.25f));

    drawSteam(sphere, shader, glm::vec3(-1.45f, deskTop + 0.17f, 0.60f), time, 0.0f);
    drawSteam(sphere, shader, glm::vec3(0.62f, deskTop + 0.24f, 0.05f), time, 0.5f);

    drawBox(cube, shader, glm::vec3(0.36f, deskTop + 0.004f, 0.62f),
            glm::vec3(0.15f, 0.004f, 0.11f), glm::vec3(0.20f, 0.10f, 0.05f));
    drawBox(cube, shader, glm::vec3(0.55f, deskTop + 0.07f, 0.62f),
            glm::vec3(0.07f, 0.085f, 0.07f), glm::vec3(0.30f, 0.75f, 0.45f),
            0.0f, 1.5708f);
    drawBox(cube, shader, glm::vec3(0.468f, deskTop + 0.07f, 0.62f),
            glm::vec3(0.008f, 0.056f, 0.056f), glm::vec3(0.20f, 0.10f, 0.05f));
    drawBox(cube, shader, glm::vec3(0.55f, deskTop + 0.16f, 0.62f),
            glm::vec3(0.022f, 0.045f, 0.012f), glm::vec3(0.30f, 0.75f, 0.45f),
            0.0f, 1.5708f);

    const float duckX = 0.75f, duckZ = -0.30f;
    const float monitorTop = 1.64f;
    const glm::vec3 yellow(1.0f, 0.85f, 0.15f);
    drawSphere(sphere, shader, glm::vec3(duckX, monitorTop + 0.09f, duckZ),
               glm::vec3(0.12f, 0.09f, 0.13f), yellow);
    drawSphere(sphere, shader, glm::vec3(duckX, monitorTop + 0.21f, duckZ + 0.07f),
               glm::vec3(0.07f), yellow);
    drawBox(cube, shader, glm::vec3(duckX, monitorTop + 0.20f, duckZ + 0.15f),
            glm::vec3(0.03f, 0.012f, 0.04f), glm::vec3(1.0f, 0.50f, 0.10f));
    for (float side : {-0.03f, 0.03f}) {
        drawSphere(sphere, shader, glm::vec3(duckX + side, monitorTop + 0.235f, duckZ + 0.125f),
                   glm::vec3(0.012f), black);
        drawSphere(sphere, shader, glm::vec3(duckX + side * 3.8f, monitorTop + 0.095f, duckZ),
                   glm::vec3(0.03f, 0.05f, 0.09f), glm::vec3(0.95f, 0.75f, 0.10f));
    }

    struct Note { float x, y, tilt; glm::vec3 color; };
    const glm::vec3 noteYellow(1.0f, 0.90f, 0.30f);
    const glm::vec3 notePink(1.0f, 0.50f, 0.70f);
    const glm::vec3 noteGreen(0.50f, 0.90f, 0.50f);
    const glm::vec3 noteOrange(1.0f, 0.65f, 0.25f);
    const Note notes[] = {
        {-1.065f, 1.30f,  0.10f, noteYellow}, {-1.065f, 0.95f, -0.12f, notePink},
        {-1.065f, 0.62f,  0.08f, noteGreen},  { 1.065f, 1.22f, -0.09f, noteOrange},
        { 1.065f, 0.80f,  0.14f, noteYellow}, {-0.70f, 1.585f,  0.06f, notePink},
        { 0.35f, 1.585f, -0.10f, noteGreen}
    };
    for (const Note& n : notes) {
        drawBox(cube, shader, glm::vec3(n.x, n.y, -0.206f),
                glm::vec3(0.045f, 0.045f, 0.004f), n.color, 0.0f, n.tilt);
    }

    if (std::sin(time * 5.0f) > 0.0f) {
        drawBox(cube, shader, glm::vec3(0.20f, 0.50f, -0.168f),
                glm::vec3(0.62f, 0.05f, 0.006f), glm::vec3(0.90f, 0.15f, 0.15f));
        for (int k = 0; k < 5; ++k) {
            drawBox(cube, shader, glm::vec3(-0.20f + k * 0.18f, 0.50f, -0.165f),
                    glm::vec3(0.06f, 0.012f, 0.007f), glm::vec3(1.0f));
        }
    }

    const float bugPhase = time * 0.6f;
    const float bugX = -0.45f + 0.65f * std::sin(bugPhase);
    const float bugDir = std::cos(bugPhase) >= 0.0f ? 1.0f : -1.0f;
    const glm::vec3 bugPos(bugX, deskTop + 0.035f, 0.30f);
    drawSphere(sphere, shader, bugPos, glm::vec3(0.055f, 0.035f, 0.055f),
               glm::vec3(0.85f, 0.10f, 0.10f));
    drawSphere(sphere, shader, bugPos + glm::vec3(bugDir * 0.055f, 0.0f, 0.0f),
               glm::vec3(0.03f, 0.025f, 0.03f), black);
    for (float side : {-0.02f, 0.02f}) {
        drawSphere(sphere, shader, bugPos + glm::vec3(-bugDir * 0.015f, 0.03f, side),
                   glm::vec3(0.012f), black);
    }
    for (int k = -1; k <= 1; ++k) {
        drawBox(cube, shader, glm::vec3(bugX + k * 0.032f, deskTop + 0.006f, 0.30f),
                glm::vec3(0.004f, 0.003f, 0.075f), black,
                0.5f * std::sin(time * 25.0f + k * 2.0f));
    }

    drawBox(cube, shader, glm::vec3(2.30f, -0.85f, 0.50f),
            glm::vec3(0.20f, 0.30f, 0.20f), glm::vec3(0.20f, 0.22f, 0.25f));
    drawBox(cube, shader, glm::vec3(2.30f, -0.54f, 0.50f),
            glm::vec3(0.22f, 0.015f, 0.22f), glm::vec3(0.14f, 0.16f, 0.18f));
    const float balls[][3] = {
        {2.22f, -0.46f, 0.44f}, {2.38f, -0.45f, 0.56f}, {2.30f, -0.36f, 0.50f},
        {2.20f, -0.50f, 0.58f}, {1.95f, -1.07f, 0.72f}, {2.05f, -1.07f, 0.15f},
        {2.62f, -1.07f, 0.95f}, {1.70f, -1.07f, 0.45f}
    };
    for (const auto& b : balls) {
        drawSphere(sphere, shader, glm::vec3(b[0], b[1], b[2]),
                   glm::vec3(0.09f), glm::vec3(0.90f, 0.90f, 0.86f));
    }

    const float spikes[][4] = {
        {-0.72f, 1.50f, 0.98f,  0.70f}, {-0.60f, 1.56f, 1.00f,  0.25f},
        {-0.48f, 1.58f, 1.00f, -0.15f}, {-0.38f, 1.54f, 0.98f, -0.50f},
        {-0.30f, 1.47f, 0.95f, -0.90f}
    };
    for (const auto& s : spikes) {
        drawSphere(sphere, shader, glm::vec3(s[0], s[1], s[2]),
                   glm::vec3(0.045f, 0.11f, 0.045f), hair, s[3]);
    }
    drawSphere(sphere, shader, glm::vec3(-0.55f, 1.66f, 0.97f),
               glm::vec3(0.022f, 0.13f, 0.022f), hair, -0.30f);

    const float dropPhase = std::fmod(time * 0.7f, 1.0f);
    drawSphere(sphere, shader, glm::vec3(-0.27f, 1.40f - dropPhase * 0.45f, 0.72f),
               glm::vec3(0.022f, 0.04f * (1.0f - 0.3f * dropPhase), 0.022f),
               glm::vec3(0.35f, 0.70f, 1.0f));
}

static void drawScene(const Mesh& cube,
                  const Mesh& sphere,
                  const Mesh& floor,
                  const Shader& shader,
                  const glm::vec3& lightPos,
                  float time,
                  bool showLight)
    {
        const glm::vec3 suit(0.055f, 0.16f, 0.22f);
        const glm::vec3 skin(0.70f, 0.43f, 0.30f);
        const glm::vec3 cyan(0.08f, 0.78f, 0.92f);

        drawBox(floor, shader, glm::vec3(0.0f, -1.15f, 0.0f),
            glm::vec3(1.0f), glm::vec3(0.11f, 0.16f, 0.21f));

            drawBox(cube, shader, glm::vec3(0.0f, 0.68f, -2.32f),
                glm::vec3(3.9f, 1.82f, 0.10f), glm::vec3(0.14f, 0.17f, 0.21f));
                drawBox(cube, shader, glm::vec3(-3.92f, 0.68f, 0.35f),
                    glm::vec3(0.10f, 1.82f, 2.75f), glm::vec3(0.16f, 0.19f, 0.22f));
                drawBox(cube, shader, glm::vec3(3.92f, 0.68f, 0.35f),
                    glm::vec3(0.10f, 1.82f, 2.75f), glm::vec3(0.18f, 0.20f, 0.22f));
                drawBox(cube, shader, glm::vec3(0.0f, 2.58f, 0.30f),
                    glm::vec3(4.0f, 0.10f, 2.85f), glm::vec3(0.16f, 0.18f, 0.20f));
            drawBox(cube, shader, glm::vec3(2.45f, 1.76f, -2.19f),
                glm::vec3(1.05f, 0.92f, 0.025f), glm::vec3(0.42f, 0.28f, 0.24f));
            drawBox(cube, shader, glm::vec3(2.45f, 2.35f, -2.14f),
                glm::vec3(0.98f, 0.29f, 0.018f), glm::vec3(0.24f, 0.29f, 0.40f));
            drawBox(cube, shader, glm::vec3(2.45f, 1.92f, -2.14f),
                glm::vec3(0.98f, 0.13f, 0.018f), glm::vec3(0.70f, 0.36f, 0.28f));
            drawBox(cube, shader, glm::vec3(2.45f, 1.60f, -2.14f),
                glm::vec3(0.98f, 0.17f, 0.018f), glm::vec3(0.88f, 0.48f, 0.28f));
            drawBox(cube, shader, glm::vec3(2.45f, 1.24f, -2.14f),
                glm::vec3(0.98f, 0.19f, 0.018f), glm::vec3(0.49f, 0.27f, 0.24f));
            for (float x : {1.37f, 3.53f}) {
            drawBox(cube, shader, glm::vec3(x, 1.76f, -2.08f),
                glm::vec3(0.065f, 1.02f, 0.075f), glm::vec3(0.16f, 0.20f, 0.23f));
            }
            for (float y : {0.73f, 2.79f}) {
            drawBox(cube, shader, glm::vec3(2.45f, y, -2.08f),
                glm::vec3(1.14f, 0.065f, 0.075f), glm::vec3(0.16f, 0.20f, 0.23f));
            }
            drawBox(cube, shader, glm::vec3(2.45f, 1.76f, -2.06f),
                glm::vec3(0.035f, 0.96f, 0.055f), glm::vec3(0.16f, 0.20f, 0.23f));
            drawSphere(sphere, shader, glm::vec3(2.78f, 1.54f, -2.03f),
                   glm::vec3(0.19f, 0.19f, 0.035f), glm::vec3(1.0f, 0.61f, 0.30f));
            if (showLight) {
            drawBox(cube, shader, glm::vec3(0.75f, -1.132f, 1.15f),
                glm::vec3(1.65f, 0.006f, 0.48f), glm::vec3(0.72f, 0.36f, 0.16f),
                0.0f, -0.07f);
            }

            const float shelfZ = -1.91f;
            const glm::vec3 woodColor(0.31f, 0.19f, 0.12f);
            drawBox(cube, shader, glm::vec3(-3.08f, 0.28f, shelfZ - 0.10f),
                glm::vec3(0.84f, 1.40f, 0.13f), glm::vec3(0.12f, 0.14f, 0.16f));
            for (float x : {-3.84f, -2.32f}) {
                drawBox(cube, shader, glm::vec3(x, 0.26f, shelfZ),
                    glm::vec3(0.07f, 1.43f, 0.25f), woodColor);
            }
            for (float y : {-0.55f, 0.12f, 0.79f, 1.46f}) {
                drawBox(cube, shader, glm::vec3(-3.08f, y, shelfZ),
                    glm::vec3(0.82f, 0.055f, 0.27f), woodColor);
            }
            const glm::vec3 bookColors[] = {
                glm::vec3(0.56f, 0.20f, 0.15f), glm::vec3(0.12f, 0.36f, 0.39f),
                glm::vec3(0.72f, 0.48f, 0.22f), glm::vec3(0.30f, 0.31f, 0.48f),
                glm::vec3(0.45f, 0.25f, 0.18f), glm::vec3(0.25f, 0.42f, 0.28f)
            };
            const float bookHeights[] = {0.39f, 0.48f, 0.34f, 0.44f, 0.37f, 0.50f};
            for (int row = 0; row < 3; ++row) {
                const float boardY = -0.55f + row * 0.67f;
                for (int book = 0; book < 6; ++book) {
                    const float x = -3.68f + book * 0.23f;
                    const float height = bookHeights[(book + row) % 6];
                    const float tilt = book % 3 == 0 ? 0.025f : 0.0f;
                    drawBox(cube, shader,
                        glm::vec3(x, boardY + 0.055f + height * 0.5f, shelfZ + 0.17f),
                        glm::vec3(0.085f, height * 0.5f, 0.065f),
                        bookColors[(book + row * 2) % 6], 0.0f, tilt);
                }
            }
            drawBox(cube, shader, glm::vec3(-3.15f, 1.64f, shelfZ + 0.14f),
                glm::vec3(0.17f, 0.13f, 0.13f), glm::vec3(0.69f, 0.32f, 0.17f));
            drawBox(cube, shader, glm::vec3(-3.15f, 1.84f, shelfZ + 0.14f),
                glm::vec3(0.13f, 0.09f, 0.10f), glm::vec3(0.20f, 0.44f, 0.25f));
            drawSphere(sphere, shader, glm::vec3(-3.15f, 2.02f, shelfZ + 0.14f),
                glm::vec3(0.13f, 0.19f, 0.12f), glm::vec3(0.20f, 0.48f, 0.28f));
            drawSphere(sphere, shader, glm::vec3(-3.32f, 1.96f, shelfZ + 0.14f),
                glm::vec3(0.13f, 0.10f, 0.11f), glm::vec3(0.26f, 0.55f, 0.30f));
            drawSphere(sphere, shader, glm::vec3(-2.61f, 1.66f, shelfZ + 0.14f),
                glm::vec3(0.17f, 0.17f, 0.13f), glm::vec3(0.16f, 0.43f, 0.60f));
            drawBox(cube, shader, glm::vec3(-2.61f, 1.49f, shelfZ + 0.14f),
                glm::vec3(0.12f, 0.035f, 0.10f), woodColor);

        drawBox(cube, shader, glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.75f, 0.14f, 0.72f), glm::vec3(0.12f, 0.19f, 0.25f));
        drawBox(cube, shader, glm::vec3(0.0f, -0.16f, 0.69f),
            glm::vec3(1.72f, 0.035f, 0.035f), glm::vec3(0.04f, 0.48f, 0.56f));
        for (float x : {-1.48f, 1.48f}) {
        drawBox(cube, shader, glm::vec3(x, -0.62f, -0.12f),
            glm::vec3(0.075f, 0.49f, 0.08f), glm::vec3(0.16f, 0.21f, 0.25f));
        }

        drawBox(cube, shader, glm::vec3(0.0f, 0.96f, -0.30f),
            glm::vec3(1.12f, 0.68f, 0.09f), glm::vec3(0.035f, 0.075f, 0.10f));
        drawBox(cube, shader, glm::vec3(0.0f, 0.96f, -0.202f),
            glm::vec3(1.01f, 0.57f, 0.012f), glm::vec3(0.025f, 0.055f, 0.075f));
        drawBox(cube, shader, glm::vec3(0.0f, 0.20f, -0.30f),
            glm::vec3(0.085f, 0.48f, 0.08f), glm::vec3(0.16f, 0.21f, 0.25f));
        drawBox(cube, shader, glm::vec3(0.0f, -0.055f, -0.18f),
            glm::vec3(0.38f, 0.045f, 0.32f), glm::vec3(0.13f, 0.18f, 0.22f));
        drawBox(cube, shader, glm::vec3(-0.73f, 0.96f, -0.185f),
            glm::vec3(0.012f, 0.48f, 0.008f), glm::vec3(0.08f, 0.28f, 0.34f));
        const float codeRows[] = {1.36f, 1.27f, 1.18f, 1.09f, 1.00f, 0.91f, 0.82f, 0.73f, 0.64f};
        const float codeWidths[] = {0.38f, 0.57f, 0.45f, 0.66f, 0.32f, 0.52f, 0.61f, 0.42f, 0.55f};
        const glm::vec3 codeColors[] = {
        glm::vec3(0.20f, 0.78f, 0.92f), glm::vec3(0.48f, 0.77f, 0.51f),
        glm::vec3(0.82f, 0.64f, 0.36f)
        };
        for (int row = 0; row < 9; ++row) {
        const float indent = row % 3 == 1 ? 0.12f : 0.0f;
        drawBox(cube, shader,
            glm::vec3(-0.55f + indent + codeWidths[row] * 0.5f, codeRows[row], -0.168f),
            glm::vec3(codeWidths[row] * 0.5f, 0.014f, 0.006f),
            codeColors[row % 3]);
        }

        drawBox(cube, shader, glm::vec3(-0.55f, -0.30f, 0.98f),
            glm::vec3(0.38f, 0.07f, 0.36f), glm::vec3(0.15f, 0.19f, 0.23f));
        drawBox(cube, shader, glm::vec3(-0.55f, 0.10f, 1.34f),
            glm::vec3(0.35f, 0.40f, 0.07f), glm::vec3(0.15f, 0.19f, 0.23f));
        for (float x : {-0.82f, -0.28f}) {
            for (float z : {0.76f, 1.20f}) {
                drawBox(cube, shader, glm::vec3(x, -0.77f, z),
                    glm::vec3(0.04f, 0.38f, 0.04f), glm::vec3(0.18f, 0.22f, 0.25f));
            }
        }
        drawBox(cube, shader, glm::vec3(-0.55f, -0.65f, 0.98f),
            glm::vec3(0.30f, 0.035f, 0.30f), glm::vec3(0.18f, 0.22f, 0.25f));

        drawSphere(sphere, shader, glm::vec3(-0.55f, 0.19f, 0.94f),
            glm::vec3(0.39f, 0.52f, 0.24f), suit, -0.06f);
        drawSphere(sphere, shader, glm::vec3(-0.55f, 0.80f, 0.91f),
            glm::vec3(0.12f, 0.17f, 0.13f), skin);
        drawSphere(sphere, shader, glm::vec3(-0.55f, 1.13f, 0.91f),
            glm::vec3(0.27f, 0.33f, 0.23f), skin, -0.04f);
        drawSphere(sphere, shader, glm::vec3(-0.55f, 1.38f, 1.02f),
            glm::vec3(0.28f, 0.12f, 0.16f), glm::vec3(0.045f, 0.075f, 0.095f));
        drawBox(cube, shader, glm::vec3(-0.55f, 1.15f, 0.675f),
            glm::vec3(0.235f, 0.052f, 0.025f), cyan);
        drawBox(cube, shader, glm::vec3(-0.55f, 0.50f, 0.695f),
            glm::vec3(0.095f, 0.09f, 0.025f), cyan);

        for (float x : {-0.82f, -0.28f}) {
            drawSphere(sphere, shader, glm::vec3(x, 0.31f, 0.82f),
                glm::vec3(0.115f, 0.22f, 0.13f), suit, -0.10f);

            drawSphere(sphere, shader, glm::vec3(x, 0.16f, 0.66f),
                glm::vec3(0.095f, 0.085f, 0.19f), suit, -0.06f);

            drawSphere(sphere, shader, glm::vec3(x, 0.285f, 0.57f),
                glm::vec3(0.095f, 0.055f, 0.09f), skin);
        }

        for (float x : {-0.68f, -0.42f}) {
            drawSphere(sphere, shader, glm::vec3(x, -0.31f, 0.68f),
                glm::vec3(0.14f, 0.13f, 0.30f), suit, -0.08f);

            drawSphere(sphere, shader, glm::vec3(x, -0.68f, 0.53f),
                glm::vec3(0.105f, 0.27f, 0.11f), suit, 0.02f);

            drawSphere(sphere, shader, glm::vec3(x, -1.035f, 0.34f),
                glm::vec3(0.14f, 0.075f, 0.20f),
                glm::vec3(0.035f, 0.06f, 0.075f));
        }

        drawBox(cube, shader, glm::vec3(-0.48f, 0.19f, 0.64f),
            glm::vec3(0.68f, 0.045f, 0.22f), glm::vec3(0.18f, 0.23f, 0.28f));
        drawBox(cube, shader, glm::vec3(-0.48f, 0.238f, 0.64f),
            glm::vec3(0.61f, 0.008f, 0.17f), glm::vec3(0.10f, 0.34f, 0.40f));
        for (int key = 0; key < 8; ++key) {
        drawBox(cube, shader,
            glm::vec3(-1.02f + key * 0.155f, 0.251f, 0.64f),
            glm::vec3(0.055f, 0.008f, 0.012f),
            key % 3 == 0 ? cyan : glm::vec3(0.25f, 0.43f, 0.48f));
        }

        if (showLight) {
        drawSphere(sphere, shader, lightPos, glm::vec3(0.075f),
               glm::vec3(1.0f, 0.78f, 0.45f));
        }

        drawFunnyProps(cube, sphere, shader, time);
    }

int main()
{
    try {
        Window window(800, 600, "Assignment 2 - FUTURE ME");
        CameraState camera;
        glfwSetWindowUserPointer(window.native(), &camera);
        glfwSetMouseButtonCallback(window.native(), mouseButtonCallback);
        glfwSetCursorPosCallback(window.native(), cursorPositionCallback);
        glfwSetScrollCallback(window.native(), scrollCallback);

        Shader shader("shader.vert", "shader.frag");
        Shader shadowShader("shadow.vert", "shadow.frag");
        Mesh cube = Mesh::cube();
        Mesh sphere = Mesh::sphere();
        Mesh floor = Mesh::plane(12.0f);

        const glm::vec3 lightPos(2.8f, 2.55f, -1.55f);
        const unsigned int shadowWidth = 2048;
        const unsigned int shadowHeight = 2048;
        unsigned int shadowFramebuffer = 0;
        unsigned int shadowTexture = 0;
        glGenFramebuffers(1, &shadowFramebuffer);
        glGenTextures(1, &shadowTexture);
        glBindTexture(GL_TEXTURE_2D, shadowTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT,
                 GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        const float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                       GL_TEXTURE_2D, shadowTexture, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Shadow framebuffer is incomplete.");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        const glm::mat4 lightProjection = glm::ortho(
            -7.0f, 7.0f, -5.0f, 5.0f, 1.0f, 18.0f
        );
        const glm::mat4 lightView = glm::lookAt(
            lightPos, glm::vec3(-0.6f, 0.0f, 0.0f), glm::vec3(0, 1, 0)
        );
        const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        shader.use();
        shader.setVec3("lightColor", glm::vec3(1.0f, 0.72f, 0.48f));
        shader.setVec3("lightPos", lightPos);
        shader.setInt("shadowMap", 0);

        while (window.isOpen()) {
            const float t = static_cast<float>(glfwGetTime());
            shadowShader.use();
            shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
            glViewport(0, 0, shadowWidth, shadowHeight);
            glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(2.0f, 4.0f);
            drawScene(cube, sphere, floor, shadowShader, lightPos, t, false);
            glDisable(GL_POLYGON_OFFSET_FILL);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            int framebufferWidth = 0;
            int framebufferHeight = 0;
            glfwGetFramebufferSize(window.native(), &framebufferWidth, &framebufferHeight);
            glViewport(0, 0, framebufferWidth, framebufferHeight);
            window.beginFrame(0.012f, 0.025f, 0.043f);

            const glm::vec3 viewPos = cameraPosition(camera);
            const glm::mat4 view = glm::lookAt(
                viewPos,
                camera.target,
                glm::vec3(0.0f, 1.0f, 0.0f)
            );

            const glm::mat4 projection = glm::perspective(
                glm::radians(45.0f),
                static_cast<float>(framebufferWidth) /
                    static_cast<float>(std::max(framebufferHeight, 1)),
                0.1f,
                100.0f
            );

            shader.use();
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);
            shader.setVec3("viewPos", viewPos);
            shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, shadowTexture);
            drawScene(cube, sphere, floor, shader, lightPos, t, true);

            window.endFrame();
        }

        glDeleteFramebuffers(1, &shadowFramebuffer);
        glDeleteTextures(1, &shadowTexture);
    }
    catch (const std::exception& e) {
        std::cerr << "\nFATAL ERROR\n" << e.what() << "\n";
        return 1;
    }

    return 0;
}
