#include "Mesh.h"

#include <GL/glew.h>

Mesh::Mesh(const std::vector<Vertex>& vertices,
           const std::vector<unsigned int>& indices)
    : m_indexCount(static_cast<unsigned int>(indices.size()))
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
        indices.data(),
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, position))
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, normal))
    );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, texCoord))
    );

    glBindVertexArray(0);
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

void Mesh::draw() const
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount),
                   GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

Mesh Mesh::cube()
{
    const glm::vec3 p[] = {
        {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1}, {-1, 1, 1},
        { 1,-1,-1}, {-1,-1,-1}, {-1, 1,-1}, { 1, 1,-1}
    };

    const glm::vec3 n[] = {
        { 0, 0, 1}, { 0, 0,-1}, {-1, 0, 0},
        { 1, 0, 0}, { 0, 1, 0}, { 0,-1, 0}
    };

    std::vector<Vertex> v;
    std::vector<unsigned int> i;

    auto addFace = [&](int a, int b, int c, int d, glm::vec3 normal) {
        const unsigned int base = static_cast<unsigned int>(v.size());
        v.push_back({p[a], normal, {0,0}});
        v.push_back({p[b], normal, {1,0}});
        v.push_back({p[c], normal, {1,1}});
        v.push_back({p[d], normal, {0,1}});
        i.insert(i.end(), {
            base, base+1, base+2,
            base, base+2, base+3
        });
    };

    addFace(0, 1, 2, 3, n[0]); // front
    addFace(4, 5, 6, 7, n[1]); // back
    addFace(5, 0, 3, 6, n[2]); // left
    addFace(1, 4, 7, 2, n[3]); // right
    addFace(3, 2, 7, 6, n[4]); // top
    addFace(5, 4, 1, 0, n[5]); // bottom

    return Mesh(v, i);
}

Mesh Mesh::plane(float size)
{
    const float h = size * 0.5f;

    std::vector<Vertex> v = {
        {{-h,0,-h}, {0,1,0}, {0,0}},
        {{ h,0,-h}, {0,1,0}, {1,0}},
        {{ h,0, h}, {0,1,0}, {1,1}},
        {{-h,0, h}, {0,1,0}, {0,1}}
    };

    std::vector<unsigned int> i = {0,1,2, 0,2,3};
    return Mesh(v, i);
}
