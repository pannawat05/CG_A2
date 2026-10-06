#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const std::string& vertexFile, const std::string& fragmentFile);
    ~Shader();

    void use() const;
    unsigned int id() const;

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;

private:
    unsigned int m_program = 0;

    static std::string readFile(const std::string& path);
    static unsigned int compile(unsigned int type, const std::string& source,
                                 const std::string& path);
};
