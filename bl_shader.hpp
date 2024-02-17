#ifndef _GL_SHADER_HPP_FILE_
#define _GL_SHADER_HPP_FILE_

#include "bl_error_handle.hpp"

#include <vector>
#include <map>

#include "glad/glad.h"

namespace Boundless::Render
{
    class Shader
    {
    private:
        GLuint shader_id;
        GLenum shader_type;

    public:
        Shader(const std::string &path, GLenum type);
        Shader(const char *data, GLenum type);
        // Warning:注意Shader的删除时机
        Shader(const Shader &) = default;
        Shader(Shader &&) = default;
        Shader &operator=(const Shader &) = default;
        Shader &operator=(Shader &&) = default;
        ~Shader();
        operator GLuint();
    };

#define DEAUFT_PROGRAM_SHADER_COUNT 2
#define PROGRAM_LOG_MAX_SIZE 4096
    class Program
    {
    private:
        GLuint program_id;
        std::vector<Shader> program_shader;
        mutable std::map<std::string, GLint> shader_uniformmap;
#ifdef _DEBUG
        void PrintLog() const;
#endif
        GLint GetUniformLocation(const std::string &target) const;
        GLint GetUniformBlockLocation(const std::string &target) const;

    public:
        Program(GLuint n = DEAUFT_PROGRAM_SHADER_COUNT);
        Program(const Program &target) = delete;
        Program(Program &&target) noexcept = default;
        Program &operator=(const Program &target) = delete;
        Program &operator=(Program &&target) noexcept = default;
        Shader &operator[](size_t index);
        ~Program();
        void UseShader(Shader &t);
        void MoveShader(Shader &&t);
        void Use() const;
        void UnUse() const;
        void Link() const;

        void SetUniformblockBinding(const std::string &name, GLuint index);
        GLint GetUniformblockSize(const std::string &name);
        void SetUniformblock(const std::string &name, UniformBuffer &ubo);
        void SetUniformblock(const std::string &name, UniformBuffer &ubo, const data_range &range);
        // 设置纹理，使value对应的纹理单元被使用（注：先glActiveTexture）
        void SetTexture(const std::string &name, GLint unit);
        // 设置布尔值
        void SetBool(const std::string &name, GLboolean value);
        // 设置int
        void SetInt(const std::string &name, GLint value);
        // 设置uint
        void SetUint(const std::string &name, GLuint value);
        // 设置浮点数
        void SetFloat(const std::string &name, GLfloat value);
        // 设置二维向量（float）
        void SetVec2(const std::string &name, const glm::vec2 &value);
        // 设置二维向量（float）
        void SetVec2(const std::string &name, GLfloat x, GLfloat y);

        // 设置三维向量（float）
        void SetVec3(const std::string &name, const glm::vec3 &value);
        // 设置三维向量（float）
        void SetVec3(const std::string &name, GLfloat x, GLfloat y, GLfloat z);

        // 设置四维向量（float）
        void SetVec4(const std::string &name, const glm::vec4 &value);
        // 设置四维向量（float）
        void SetVec4(const std::string &name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

        // 设置二维向量数组（float）
        void SetVec2Array(const std::string &name, GLsizei count, const glm::vec2 *value);
        // 设置三维向量数组（float）
        void SetVec3Array(const std::string &name, GLsizei count, const glm::vec3 *value);
        // 设置四维向量数组（float）
        void SetVec4Array(const std::string &name, GLsizei count, const glm::vec4 *value);

        // 设置2*2矩阵（float）
        void SetMat2(const std::string &name, const glm::mat2 &mat);
        // 设置3*3矩阵（float）
        void SetMat3(const std::string &name, const glm::mat3 &mat);
        // 设置4*4矩阵（float）
        void SetMat4(const std::string &name, const glm::mat4 &mat);
        // 设置2*2矩阵数组（float）
        void SetMat2Array(const std::string &name, GLsizei count, const glm::mat2 *mat);
        // 设置3*3矩阵数组（float）
        void SetMat3Array(const std::string &name, GLsizei count, const glm::mat3 *mat);
        // 设置4*4矩阵数组（float）
        void SetMat4Array(const std::string &name, GLsizei count, const glm::mat4 *mat);
        // 设置双精度浮点数
        void SetDouble(const std::string &name, GLdouble value);
        // 设置二维向量（double）
        void SetVec2(const std::string &name, const glm::dvec2 &value);
        // 设置二维向量数组（double）
        void SetVec2Array(const std::string &name, GLsizei count, const glm::dvec2 *value);
        // 设置二维向量（double）
        void SetVec2(const std::string &name, GLdouble x, GLdouble y);

        // 设置三维向量（double）
        void SetVec3(const std::string &name, const glm::dvec3 &value);
        // 设置三维向量数组（double）
        void SetVec3Array(const std::string &name, GLsizei count, const glm::dvec3 *value);
        // 设置三维向量（double）
        void SetVec3(const std::string &name, GLdouble x, GLdouble y, GLdouble z);

        // 设置四维向量（double）
        void SetVec4(const std::string &name, const glm::dvec4 &value);
        // 设置四维向量数组（double）
        void SetVec4Array(const std::string &name, GLsizei count, const glm::dvec4 *value);
        // 设置四维向量（double）
        void SetVec4(const std::string &name, GLdouble x, GLdouble y, GLdouble z, GLdouble w);

        // 设置2*2矩阵（double）
        void SetMat2(const std::string &name, const glm::dmat2 &mat);
        // 设置3*3矩阵（double）
        void SetMat3(const std::string &name, const glm::dmat3 &mat);
        // 设置4*4矩阵（double）
        void SetMat4(const std::string &name, const glm::dmat4 &mat);
        // 设置2*2矩阵数组（double）
        void SetMat2Array(const std::string &name, GLsizei count, const glm::dmat2 *mat);
        // 设置3*3矩阵数组（double）
        void SetMat3Array(const std::string &name, GLsizei count, const glm::dmat3 *mat);
        // 设置4*4矩阵数组（double）
        void SetMat4Array(const std::string &name, GLsizei count, const glm::dmat4 *mat);
        // 设置二维向量（int
        void SetVec2(const std::string &name, const glm::ivec2 &value);
        // 设置二维向量（int）
        void SetVec2(const std::string &name, GLint x, GLint y);

        // 设置三维向量（int）
        void SetVec3(const std::string &name, const glm::ivec3 &value);
        // 设置三维向量（int）
        void SetVec3(const std::string &name, GLint x, GLint y, GLint z);

        // 设置四维向量（int）
        void SetVec4(const std::string &name, const glm::ivec4 &value);
        // 设置四维向量（int）
        void SetVec4(const std::string &name, GLint x, GLint y, GLint z, GLint w);
        // 设置二维向量（uint32_t）
        void SetVec2(const std::string &name, const glm::uvec2 &value);
        // 设置二维向量（uint32_t）
        void SetVec2(const std::string &name, GLuint x, GLuint y);

        // 设置三维向量（uint32_t）
        void SetVec3(const std::string &name, const glm::uvec3 &value);
        // 设置三维向量（uint32_t）
        void SetVec3(const std::string &name, GLuint x, GLuint y, GLuint z);

        // 设置四维向量（uint32_t）
        void SetVec4(const std::string &name, const glm::uvec4 &value);
        // 设置四维向量（uint32_t）
        void SetVec4(const std::string &name, GLuint x, GLuint y, GLuint z, GLuint w);
    };
} // namespace Boundless::Render
#endif //!_GL_SHADER_HPP_FILE_