#include "Libs/Mesh.h"
#include "Libs/Shader.h"
#include "Libs/Window.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <iostream>
#include <stdexcept>

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

int main()
{
    try {
        Window window(800, 600, "Assignment 2 - FUTURE ME");

        Shader shader("shader.vert", "shader.frag");
        Mesh cube = Mesh::cube();
        Mesh floor = Mesh::plane(12.0f);

        const glm::vec3 lightPos(2.5f, 4.0f, 2.5f);
        const glm::vec3 viewPos(0.0f, 2.2f, 7.0f);

        shader.use();
        shader.setVec3("lightColor", glm::vec3(1.0f, 0.92f, 0.80f));
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", viewPos);

        while (window.isOpen()) {
            window.beginFrame(0.015f, 0.025f, 0.055f);

            const float t =
                static_cast<float>(glfwGetTime());

            const glm::mat4 view = glm::lookAt(
                viewPos,
                glm::vec3(0.0f, 1.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );

            const glm::mat4 projection = glm::perspective(
                glm::radians(45.0f),
                800.0f / 600.0f,
                0.1f,
                100.0f
            );

            shader.use();
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);
            shader.setVec3("viewPos", viewPos);

            // Floor / workstation platform.
            drawBox(
                floor, shader,
                glm::vec3(0.0f, -1.15f, 0.0f),
                glm::vec3(1.0f),
                glm::vec3(0.16f, 0.20f, 0.28f)
            );

            // Main "Future Me" workstation.
            drawBox(
                cube, shader,
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(1.75f, 0.15f, 0.75f),
                glm::vec3(0.18f, 0.24f, 0.34f)
            );

            // Monitor.
            drawBox(
                cube, shader,
                glm::vec3(0.0f, 0.95f, -0.25f),
                glm::vec3(1.15f, 0.70f, 0.10f),
                glm::vec3(0.08f, 0.22f, 0.38f),
                0.0f
            );

            // Screen glow panel.
            drawBox(
                cube, shader,
                glm::vec3(0.0f, 0.95f, -0.12f),
                glm::vec3(0.92f, 0.48f, 0.035f),
                glm::vec3(0.12f, 0.55f, 0.95f)
            );

            // Monitor stand.
            drawBox(
                cube, shader,
                glm::vec3(0.0f, 0.20f, -0.25f),
                glm::vec3(0.10f, 0.55f, 0.10f),
                glm::vec3(0.22f, 0.25f, 0.30f)
            );

            // Future-you: a human computer scientist in a futuristic suit.
            drawBox(
                cube, shader,
                glm::vec3(-2.35f, 0.38f, 0.12f),
                glm::vec3(0.36f, 0.53f, 0.30f),
                glm::vec3(0.10f, 0.25f, 0.30f),
                0.06f
            );

            // Neck and head.
            drawBox(
                cube, shader,
                glm::vec3(-2.35f, 0.95f, 0.12f),
                glm::vec3(0.13f, 0.16f, 0.15f),
                glm::vec3(0.70f, 0.43f, 0.29f)
            );
            drawBox(
                cube, shader,
                glm::vec3(-2.35f, 1.30f, 0.12f),
                glm::vec3(0.30f, 0.35f, 0.25f),
                glm::vec3(0.72f, 0.48f, 0.34f),
                -0.04f
            );

            // Hair and luminous augmented-reality visor.
            drawBox(
                cube, shader,
                glm::vec3(-2.35f, 1.57f, 0.12f),
                glm::vec3(0.31f, 0.11f, 0.26f),
                glm::vec3(0.06f, 0.11f, 0.17f)
            );
            const float pulse = 0.55f + 0.35f * std::sin(t * 4.0f);
            drawBox(
                cube, shader,
                glm::vec3(-2.35f, 1.34f, 0.38f),
                glm::vec3(0.25f, 0.055f, 0.035f),
                glm::vec3(0.08f, pulse, 1.0f)
            );

            // Shoulders, arms, and legs give the avatar a clear human silhouette.
            drawBox(
                cube, shader,
                glm::vec3(-2.72f, 0.70f, 0.12f),
                glm::vec3(0.15f, 0.37f, 0.17f),
                glm::vec3(0.10f, 0.25f, 0.30f),
                0.0f, -0.28f
            );
            drawBox(
                cube, shader,
                glm::vec3(-1.99f, 0.70f, 0.12f),
                glm::vec3(0.15f, 0.37f, 0.17f),
                glm::vec3(0.10f, 0.25f, 0.30f),
                0.0f, 0.28f
            );
            drawBox(
                cube, shader,
                glm::vec3(-2.50f, -0.43f, 0.12f),
                glm::vec3(0.14f, 0.38f, 0.18f),
                glm::vec3(0.08f, 0.12f, 0.19f)
            );
            drawBox(
                cube, shader,
                glm::vec3(-2.20f, -0.43f, 0.12f),
                glm::vec3(0.14f, 0.38f, 0.18f),
                glm::vec3(0.08f, 0.12f, 0.19f)
            );

            // Suit badge / future-tech accent.
            drawBox(
                cube, shader,
                glm::vec3(-2.35f, 0.48f, 0.43f),
                glm::vec3(0.12f, 0.10f, 0.025f),
                glm::vec3(0.10f, 0.78f, 0.95f)
            );

            // Keyboard.
            drawBox(
                cube, shader,
                glm::vec3(0.0f, 0.22f, 0.80f),
                glm::vec3(0.90f, 0.045f, 0.30f),
                glm::vec3(0.25f, 0.30f, 0.38f),
                0.0f
            );

            // Small "AI core" hovering above the desk.
            const float hoverY = 1.15f + 0.10f * std::sin(t * 2.0f);
            drawBox(
                cube, shader,
                glm::vec3(1.20f, hoverY, 0.20f),
                glm::vec3(0.18f),
                glm::vec3(0.20f, 0.65f, 1.0f),
                t
            );

            // Light marker.
            drawBox(
                cube, shader,
                lightPos,
                glm::vec3(0.08f),
                glm::vec3(1.0f, 0.80f, 0.45f)
            );

            window.endFrame();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "\nFATAL ERROR\n" << e.what() << "\n";
        return 1;
    }

    return 0;
}
