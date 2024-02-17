#include "bl_shader.hpp"

namespace Boundless::Render
{
    Shader::Shader(const std::string &path, GLenum type)
        : shader_type(type)
    {

        std::ifstream reader(path, std::ios::in);
        if (!reader.is_open())
        {
            this->shader_id = 0;
            ERROR("Resource", "无法打开文件:", path);
            return;
        }
        std::string shader_code((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());
        reader.close();

        this->shader_id = glCreateShader(type);
        const char *res = shader_code.c_str();
        const GLint length = static_cast<GLint>(shader_code.size());
        glShaderSource(this->shader_id, 1, &res, &length);
        glCompileShader(this->shader_id);

        GLint success;
        glGetShaderiv(this->shader_id, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE)
        {
            int length;
            glGetShaderiv(this->shader_id, GL_INFO_LOG_LENGTH, &length);
            char *log = (char *)malloc(sizeof(char) * length);
            if (log == nullptr)
            {
                ERROR("Memory/OpenGL", "内存耗尽/着色器编译错误:", shader_code, "\n--------------------------------");
            }
            else
            {
                glGetShaderInfoLog(this->shader_id, length, &length, log);
                ERROR("OpenGL", "着色器编译错误:", shader_code, "\n--------------------------------", log, "\n--------------------------------");
                free(log);
            }
            return;
        }
    }
    Shader::Shader(const char *data, GLenum type)
        : shader_type(type)
    {
        this->shader_id = glCreateShader(type);
        const GLint length = static_cast<GLint>(std::strlen(data));
        glShaderSource(this->shader_id, 1, &data, &length);
        glCompileShader(this->shader_id);

        GLint success;
        glGetShaderiv(this->shader_id, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE)
        {
            int length;
            glGetShaderiv(this->shader_id, GL_INFO_LOG_LENGTH, &length);
            char *log = (char *)malloc(sizeof(char) * length);
            if (log == nullptr)
            {
                ERROR("Memory/OpenGL", "内存耗尽/着色器编译错误:", data, "\n--------------------------------");
            }
            else
            {
                glGetShaderInfoLog(this->shader_id, length, &length, log);
                ERROR("OpenGL", "着色器编译错误", data, "\n--------------------------------", log, "\n--------------------------------");
                free(log);
            }
        }
    }

    Shader::~Shader()
    {
        glDeleteShader(this->shader_id);
    }
    Shader::operator GLuint()
    {
        return this->shader_id;
    }
    Program::Program(GLuint n)
    {
        if (n != 0)
        {
            program_shader.reserve(n);
        }
        this->program_id = glCreateProgram();
    }
    Shader &Program::operator[](size_t index)
    {
        return this->program_shader[index];
    }
    Program::~Program()
    {
        glDeleteProgram(this->program_id);
    }
    void Program::UseShader(Shader &t)
    {
        this->program_shader.push_back(t);
        glAttachShader(this->program_id, GLuint(this->program_shader.back()));
    }
    void Program::MoveShader(Shader &&t)
    {
        this->program_shader.push_back(std::forward<Shader>(t));
        glAttachShader(this->program_id, GLuint(this->program_shader.back()));
    }
    void Program::Use() const
    {
        glUseProgram(this->program_id);
    }
    void Program::UnUse() const
    {
        glUseProgram(0);
    }

    void Program::Link() const
    {
        glLinkProgram(this->program_id);
#ifdef _DEBUG
        this->PrintLog();
#endif
    }
    GLint Program::GetUniformLocation(const std::string &target) const
    {
        if (shader_uniformmap.find(target) != shader_uniformmap.end())
            return shader_uniformmap[target];
        GLint location = glGetUniformLocation(this->program_id, target.c_str());
        if (location != -1)
            shader_uniformmap[target] = location;
        return location;
    }
    GLint Program::GetUniformBlockLocation(const std::string &target) const
    {
        if (shader_uniformmap.find(target) != shader_uniformmap.end())
            return shader_uniformmap[target];
        GLint location = glGetUniformBlockIndex(this->program_id, target.c_str());
        if (location != -1)
            shader_uniformmap[target] = location;
        return location;
    }
#ifdef _DEBUG
    void Program::PrintLog() const
    {
        GLint success;
        glGetProgramiv(this->program_id, GL_LINK_STATUS, &success);
        if (success == GL_FALSE)
        {
            GLsizei length;
            glGetProgramiv(this->program_id, GL_INFO_LOG_LENGTH, &length);
            char *log = (char *)malloc(length);
            if (log == nullptr)
            {
                ERROR("Memory/OpenGL", "内存耗尽/着色器链接错误");
            }
            else
            {
                glGetProgramInfoLog(this->program_id, PROGRAM_LOG_MAX_SIZE, nullptr, log);
                ERROR("OpenGL","着色器链接错误:", log, "\n--------------------------------");
                free(log);
            }
        }
    }
#endif

    // 设置纹理，使value对应的纹理单元被使用（注：先glActiveTexture）
    void Program::SetTexture(const std::string &name, GLint value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform1i(location, value);
    }
    // 设置布尔值
    void Program::SetBool(const std::string &name, GLboolean value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform1i(location, (int)value);
    }
    // 设置int
    void Program::SetInt(const std::string &name, GLint value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform1i(location, value);
    }
    // 设置uint
    void Program::SetUint(const std::string &name, GLuint value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform1ui(location, value);
    }
    // 设置浮点数
    void Program::SetFloat(const std::string &name, GLfloat value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform1f(location, value);
    }
    // 设置二维向量（float）
    void Program::SetVec2(const std::string &name, const glm::vec2 &value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform2fv(location, 1, &value[0]);
    }
    // 设置二维向量（float）
    void Program::SetVec2(const std::string &name, GLfloat x, GLfloat y)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform2f(location, x, y);
    }

    // 设置三维向量（float）
    void Program::SetVec3(const std::string &name, const glm::vec3 &value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform3fv(location, 1, &value[0]);
    }
    // 设置三维向量（float）
    void Program::SetVec3(const std::string &name, GLfloat x, GLfloat y, GLfloat z)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform3f(location, x, y, z);
    }

    // 设置四维向量（float）
    void Program::SetVec4(const std::string &name, const glm::vec4 &value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform4fv(location, 1, &value[0]);
    }
    // 设置四维向量（float）
    void Program::SetVec4(const std::string &name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform4f(location, x, y, z, w);
    }
    // 设置二维向量数组（float）
    void Program::SetVec2Array(const std::string &name, GLsizei count, const glm::vec2 *value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform2fv(location, count, (GLfloat *)value);
    }
    // 设置三维向量数组（float）
    void Program::SetVec3Array(const std::string &name, GLsizei count, const glm::vec3 *value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform3fv(location, count, (GLfloat *)value);
    }
    // 设置四维向量数组（float）
    void Program::SetVec4Array(const std::string &name, GLsizei count, const glm::vec4 *value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform4fv(location, count, (GLfloat *)value);
    }
    // 设置2*2矩阵（float）
    void Program::SetMat2(const std::string &name, const glm::mat2 &mat)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix2fv(location, 1, GL_FALSE, &mat[0][0]);
    }
    // 设置3*3矩阵（float）
    void Program::SetMat3(const std::string &name, const glm::mat3 &mat)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
    }
    // 设置4*4矩阵（float）
    void Program::SetMat4(const std::string &name, const glm::mat4 &mat)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
    }
    // 设置2*2矩阵数组（float）
    void Program::SetMat2Array(const std::string &name, GLsizei count, const glm::mat2 *mat)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix2fv(location, count, GL_FALSE, (GLfloat *)mat);
    }
    // 设置3*3矩阵数组（float）
    void Program::SetMat3Array(const std::string &name, GLsizei count, const glm::mat3 *mat)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix3fv(location, count, GL_FALSE, (GLfloat *)mat);
    }
    // 设置4*4矩阵数组（float）
    void Program::SetMat4Array(const std::string &name, GLsizei count, const glm::mat4 *mat)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix4fv(location, count, GL_FALSE, (GLfloat *)mat);
    }
    // 设置双精度浮点数
    void Program::SetDouble(const std::string &name, GLdouble value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform1d(location, value);
    }
    // 设置二维向量（double）
    void Program::SetVec2(const std::string &name, const glm::dvec2 &value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform2dv(location, 1, &value[0]);
    }
    // 设置二维向量数组（double）
    void Program::SetVec2Array(const std::string &name, GLsizei count, const glm::dvec2 *value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform2dv(location, count, (GLdouble *)value);
    }
    // 设置二维向量（double）
    void Program::SetVec2(const std::string &name, GLdouble x, GLdouble y)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform2d(location, x, y);
    }

    // 设置三维向量（double）
    void Program::SetVec3(const std::string &name, const glm::dvec3 &value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform3dv(location, 1, &value[0]);
    }
    // 设置三维向量数组（double）
    void Program::SetVec3Array(const std::string &name, GLsizei count, const glm::dvec3 *value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform3dv(location, count, (GLdouble *)value);
    }
    // 设置三维向量（double）
    void Program::SetVec3(const std::string &name, GLdouble x, GLdouble y, GLdouble z)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform3d(location, x, y, z);
    }

    // 设置四维向量（double）
    void Program::SetVec4(const std::string &name, const glm::dvec4 &value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform4dv(location, 1, &value[0]);
    }
    // 设置四维向量数组（double）
    void Program::SetVec4Array(const std::string &name, GLsizei count, const glm::dvec4 *value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform4dv(location, count, (GLdouble *)value);
    }
    // 设置四维向量（double）
    void Program::SetVec4(const std::string &name, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform4d(location, x, y, z, w);
    }

    // 设置2*2矩阵（double）
    void Program::SetMat2(const std::string &name, const glm::dmat2 &mat)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix2dv(location, 1, GL_FALSE, &mat[0][0]);
    }
    // 设置3*3矩阵（double）
    void Program::SetMat3(const std::string &name, const glm::dmat3 &mat)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix3dv(location, 1, GL_FALSE, &mat[0][0]);
    }
    // 设置4*4矩阵（double）
    void Program::SetMat4(const std::string &name, const glm::dmat4 &mat)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix4dv(location, 1, GL_FALSE, &mat[0][0]);
    }
    // 设置2*2矩阵数组（double）
    void Program::SetMat2Array(const std::string &name, GLsizei count, const glm::dmat2 *mat)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix2dv(location, count, GL_FALSE, (GLdouble *)mat);
    }
    // 设置3*3矩阵数组（double）
    void Program::SetMat3Array(const std::string &name, GLsizei count, const glm::dmat3 *mat)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix3dv(location, count, GL_FALSE, (GLdouble *)mat);
    }
    // 设置4*4矩阵数组（double）
    void Program::SetMat4Array(const std::string &name, GLsizei count, const glm::dmat4 *mat)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix4dv(location, count, GL_FALSE, (GLdouble *)mat);
    }
    // 设置二维向量（int）
    void Program::SetVec2(const std::string &name, const glm::ivec2 &value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform2iv(location, 1, &value[0]);
    }
    // 设置二维向量（int）
    void Program::SetVec2(const std::string &name, GLint x, GLint y)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform2i(location, x, y);
    }

    // 设置三维向量（int）
    void Program::SetVec3(const std::string &name, const glm::ivec3 &value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform3iv(location, 1, &value[0]);
    }
    // 设置三维向量（int）
    void Program::SetVec3(const std::string &name, GLint x, GLint y, GLint z)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform3i(location, x, y, z);
    }

    // 设置四维向量（int）
    void Program::SetVec4(const std::string &name, const glm::ivec4 &value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform4iv(location, 1, &value[0]);
    }
    // 设置四维向量（int）
    void Program::SetVec4(const std::string &name, GLint x, GLint y, GLint z, GLint w)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform4i(location, x, y, z, w);
    }
    // 设置二维向量（uint32_t）
    void Program::SetVec2(const std::string &name, const glm::uvec2 &value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform2uiv(location, 1, &value[0]);
    }
    // 设置二维向量（uint32_t）
    void Program::SetVec2(const std::string &name, GLuint x, GLuint y)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform2ui(location, x, y);
    }

    // 设置三维向量（uint32_t）
    void Program::SetVec3(const std::string &name, const glm::uvec3 &value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform3uiv(location, 1, &value[0]);
    }
    // 设置三维向量（uint32_t）
    void Program::SetVec3(const std::string &name, GLuint x, GLuint y, GLuint z)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform3ui(location, x, y, z);
    }

    // 设置四维向量（uint32_t）
    void Program::SetVec4(const std::string &name, const glm::uvec4 &value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform4uiv(location, 1, &value[0]);
    }
    // 设置四维向量（uint32_t）
    void Program::SetVec4(const std::string &name, GLuint x, GLuint y, GLuint z, GLuint w)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform4ui(location, x, y, z, w);
    }
} // namespace Boundless::Render
