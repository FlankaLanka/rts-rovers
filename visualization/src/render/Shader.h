#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

namespace terrafirma {

class Shader {
public:
    Shader();
    ~Shader();

    bool loadFromSource(const std::string& vertexSrc, const std::string& fragmentSrc);
    bool loadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    
    void use() const;
    GLuint getProgram() const { return m_program; }
    
    void setMat4(const std::string& name, const glm::mat4& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setFloat(const std::string& name, float value) const;
    void setInt(const std::string& name, int value) const;

private:
    GLuint compileShader(GLenum type, const std::string& source);
    
    GLuint m_program = 0;
};

} // namespace terrafirma

