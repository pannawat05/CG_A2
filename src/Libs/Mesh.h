#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices,
         const std::vector<unsigned int>& indices);
    ~Mesh();

    void draw() const;

    static Mesh cube();
    static Mesh plane(float size = 10.0f);
    static Mesh sphere(unsigned int segments = 32, unsigned int rings = 20);

private:
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_ebo = 0;
    unsigned int m_indexCount = 0;
};
