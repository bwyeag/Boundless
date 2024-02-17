/*
 * BOUNDLESS Engine File
 * Create: 2023/12/2/18:15
 * OpenGL Render C++ Header
 *
 */
#ifndef _BOUNDLESS_GL_UTILITY_HPP_FILE_
#define _BOUNDLESS_GL_UTILITY_HPP_FILE_
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "error_handle.hpp"

#include <cstdint>
#include <cstddef>

#include <utility>
#include <map>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <list>
#include <forward_list>
#include <algorithm>

namespace Boundless
{
    using namespace Log;
    //////////////////////////////////////////////////////////////////
    //  OpenGL 辅助函数
    //  OpenGL Helper Functions
    //
    constexpr inline bool openglIsInteger(GLenum type)
    {
        switch (type)
        {
        case GL_BYTE:
            return true;
        case GL_UNSIGNED_BYTE:
            return true;
        case GL_SHORT:
            return true;
        case GL_UNSIGNED_SHORT:
            return true;
        case GL_INT:
            return true;
        case GL_UNSIGNED_INT:
            return true;
        default:
            return false;
        /*
        case GL_FIXED:
            return false;
        case GL_FLOAT:
            return false;
        case GL_HALF_FLOAT:
            return false;
        case GL_DOUBLE:
            return false;
        case GL_INT_2_10_10_10_REV:
            return false;
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            return false;
        default:
            throw;*/
        }
    }
    constexpr inline size_t openglTypeSize(GLenum type)
    {
        switch (type)
        {
        case GL_BYTE:
            return sizeof(GLbyte);
        case GL_BOOL:
            return sizeof(GLbyte);
        case GL_UNSIGNED_BYTE:
            return sizeof(GLubyte);
        case GL_SHORT:
            return sizeof(GLshort);
        case GL_UNSIGNED_SHORT:
            return sizeof(GLushort);
        case GL_INT:
            return sizeof(GLint);
        case GL_UNSIGNED_INT:
            return sizeof(GLuint);
        case GL_INT_VEC2:
            return 2*sizeof(GLint);
        case GL_UNSIGNED_INT_VEC2:
            return 2*sizeof(GLuint);
        case GL_INT_VEC3:
            return 3*sizeof(GLint);
        case GL_UNSIGNED_INT_VEC3:
            return 3*sizeof(GLuint);
        case GL_INT_VEC4:
            return 4*sizeof(GLint);
        case GL_UNSIGNED_INT_VEC4:
            return 4*sizeof(GLuint);
        case GL_FIXED:
            return sizeof(GLfixed);
        case GL_FLOAT:
            return sizeof(GLfloat);
        case GL_HALF_FLOAT:
            return sizeof(GLhalf);
        case GL_DOUBLE:
            return sizeof(GLdouble);
        case GL_INT_2_10_10_10_REV:
            return sizeof(GLuint);
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            return sizeof(GLuint);
        case GL_FLOAT_VEC2:
            return 2*sizeof(GLfloat);
        case GL_FLOAT_VEC3:
            return 3*sizeof(GLfloat);
        case GL_FLOAT_VEC4:
            return 4*sizeof(GLfloat);
        case GL_FLOAT_MAT2:
            return 4*sizeof(GLfloat);
        case GL_FLOAT_MAT2x3:
            return 6*sizeof(GLfloat);
        case GL_FLOAT_MAT2x4:
            return 8*sizeof(GLfloat);
        case GL_FLOAT_MAT3:
            return 9*sizeof(GLfloat);
        case GL_FLOAT_MAT3x2:
            return 6*sizeof(GLfloat);
        case GL_FLOAT_MAT3x4:
            return 12*sizeof(GLfloat);
        case GL_FLOAT_MAT4:
            return 16*sizeof(GLfloat);
        case GL_FLOAT_MAT4x2:
            return 8*sizeof(GLfloat);
        case GL_FLOAT_MAT4x3:
            return 12*sizeof(GLfloat);
        case GL_BOOL_VEC2:
            return 2*sizeof(GLbyte);
        case GL_BOOL_VEC3:
            return 3*sizeof(GLbyte);
        case GL_BOOL_VEC4:
            return 4*sizeof(GLbyte);
        default:
            ERROR("OpenGL","无法识别的类型");
            error_handle();
        }
    }
    //////////////////////////////////////////////////////////////////
    //  OpenGL 类
    //  OpenGL Classes
    //

    /// @brief 存储数据范围的传递结构
    struct data_range
    {
        GLintptr offset;
        GLsizeiptr length;
    };

    /// @brief 所有*Buffer的基类
    class Buffer
    {
    protected:
        GLuint buffer_id;
        GLenum buffer_target;
        GLsizeiptr buffer_size;
        GLbitfield buffer_storeflags;
        union
        {
            GLenum map_access;
            GLbitfield map_bitfield;
        };
        data_range map_range;

    public:
        Buffer();
        Buffer(GLenum);
        Buffer(GLenum target, GLsizeiptr size, GLbitfield flags, const void *data = nullptr);
        Buffer(Buffer &&) = default;
        Buffer(const Buffer &) = delete;
        Buffer &operator=(Buffer &&) = default;
        Buffer &operator=(const Buffer &) = delete;
        ~Buffer();

        void Bind();
        void Unbind();
        void Bind(GLenum tar);
        void Unbind(GLenum tar);
        void SetTarget(GLenum tar);

        void StoreData(GLsizeiptr size, GLbitfield flags, const void *data = nullptr);
        void StoreSubData(const data_range &range, const void *data);
        void FillData(GLenum internal_format, GLenum format, GLenum type, const void *data);
        void FillSubData(const data_range &range, GLenum internal_format, GLenum format, GLenum type, const void *data);

        static void CopyData(Buffer &read, Buffer &write, const data_range &read_range, GLintptr write_offset);

        void GetData(const data_range &range, void *write_to);

        void *Map(GLenum access);
        void Unmap();
        void *MapSub(const data_range &range, GLbitfield flags);
        void FlushMap(const data_range &range);

        void Invalidate();
        void InvalidateSub(const data_range &range);
        operator GLuint();
    };
    /// @brief 用于存储顶点数组属性的类
    class layout_format
    {
    public:
        /// @brief 属性数据类型
        GLenum type;
        /// @brief 数据长度
        GLsizei size;
        /// @brief 复合对象包含多个信息
        /// @details 字节排序（高位）-8a-8b-8c-8d-（低位）a:是否归一化,
        ///          b:使用的index[1-255],c:是否启用,d:数量(1,2,3,4)
        mutable GLuint info;
        /// @brief 无参构造，不进行任何初始化
        layout_format() = default;
        /// @brief 构造函数，初始化数据
        /// @param type 属性数据类型
        /// @param count 数量
        /// @param normalize 是否归一化到[-1,+1]或[0,1]（强制转换为float）
        layout_format(GLenum type, GLuint count, GLboolean normalize);
        /// @brief 移动构造函数（实际只是复制）
        /// @param target 移动目标
        layout_format(layout_format &&target) noexcept = default;
        /// @brief 复制构造函数
        /// @param target 复制目标
        layout_format(const layout_format &target) = default;
        layout_format &operator=(layout_format &target);
        layout_format &operator=(layout_format &&target);
        void set_count(GLuint count) const;
        GLuint get_count() const;
        GLboolean is_normalised() const;
        void set_normalised(bool enable) const;

        GLboolean is_enable() const;
        void set_enable(bool enable) const;
        void set_index(GLuint index) const;
        GLuint get_index() const;
    };
    /// @brief 顶点索引类
    class IndexBuffer : public Buffer
    {
    private:
        /// @brief 绘制时索引排布方法
        GLenum draw_type;
        /// @brief 索引数据类型
        GLenum index_type;
        /// @brief 索引数量
        GLsizei index_count;

    public:
        IndexBuffer();
        IndexBuffer(GLenum draw_type, GLenum index_type, const void *data, GLsizeiptr size, GLbitfield flags);
        IndexBuffer(GLsizeiptr size, GLbitfield flags);
        IndexBuffer(const IndexBuffer &target) = delete;
        IndexBuffer(IndexBuffer &&target) = default;
        IndexBuffer &operator=(IndexBuffer &&target) noexcept = default;
        IndexBuffer &operator=(const IndexBuffer &target) = delete;
        void SetData(GLenum draw_type, GLenum index_type);
        void SetDrawType(GLenum draw_type);
        void SetDataType(GLenum index_type);
        GLenum GetDrawType() const;
        GLenum GetIndexType() const;
        GLsizei GetIndexCount() const;
        /// @brief 获取id
        operator GLuint() const;
    };
    class UniformBuffer : public Buffer
    {
    private:
        GLint binding;

    public:
        UniformBuffer();
        UniformBuffer(const void *data, GLsizeiptr size, GLbitfield flags);
        UniformBuffer(GLsizeiptr size, GLbitfield flags);
        UniformBuffer(const UniformBuffer &target) = delete;
        UniformBuffer(UniformBuffer &&target) = default;
        UniformBuffer &operator=(UniformBuffer &&target) noexcept = default;
        UniformBuffer &operator=(const UniformBuffer &target) = delete;
        void BindTo(GLuint index);
    };
    class ShaderStorageBuffer : public Buffer
    {
    private:
        GLint binding;

    public:
        ShaderStorageBuffer();
        ShaderStorageBuffer(const void *data, GLsizeiptr size, GLbitfield flags);
        ShaderStorageBuffer(GLsizeiptr size, GLbitfield flags);
        ShaderStorageBuffer(const ShaderStorageBuffer &target) = delete;
        ShaderStorageBuffer(ShaderStorageBuffer &&target) = default;
        ShaderStorageBuffer &operator=(ShaderStorageBuffer &&target) noexcept = default;
        ShaderStorageBuffer &operator=(const ShaderStorageBuffer &target) = delete;
        void BindTo(GLuint index);
    };
#define DEAUFT_VERTEX_ELEMENT_RESERVE 8

    /// @brief 顶点数据类
    class VertexBuffer : public Buffer
    {
    private:
        GLsizei vertex_size = 0;
        std::vector<layout_format> vertex_layout;

    public:
        VertexBuffer(std::uint32_t reserve = DEAUFT_VERTEX_ELEMENT_RESERVE);
        VertexBuffer(GLsizeiptr size,
                     GLbitfield flags,
                     std::uint32_t reserve = DEAUFT_VERTEX_ELEMENT_RESERVE,
                     const void *data = nullptr);
        VertexBuffer(const VertexBuffer &target) = delete;
        VertexBuffer(VertexBuffer &&target) noexcept = default;
        VertexBuffer &operator=(const VertexBuffer &target) = delete;
        VertexBuffer &operator=(VertexBuffer &&target) noexcept = default;
        VertexBuffer &operator<<(const layout_format &data);
        operator GLuint() const;

        GLsizei GetSize() const;
        const std::vector<layout_format> &GetLayout() const;
    };

    /// @brief 顶点数组类
    class VertexArray
    {
    protected:
        /// @brief 在OpenGL中的id
        GLuint array_id;
        /// @brief 当前最后一个未使用的顶点属性索引
        GLuint layout_index = 0;

    public:
        /// @brief 仅创建VertexArray
        /// @param n 对顶点属性存储的容量（可扩容）
        VertexArray();
        VertexArray(const VertexArray &target) = delete;
        VertexArray(VertexArray &&target) noexcept = default;
        VertexArray &operator=(const VertexArray &target) = delete;
        VertexArray &operator=(VertexArray &&target) noexcept = default;

        /// @brief 设置与VAO关联的VBO（当前绑定的VertexBuffer）的顶点属性（先绑定自身和VBO）
        /// @param target 设置顶点属性的目标
        /// @param range 顶点属性在VBO中的范围
        void Use(VertexBuffer &target, const data_range &range, bool enable = false, bool set = false);
        /// @brief 使用全部属性
        void UseAll(VertexBuffer &target, bool enable = false, bool set = false);
        void Enable(GLuint index);
        void Disable(GLuint index);
        /// @brief 绑定VAO为当前VAO
        void Bind() const;
        /// @brief 解绑VAO
        void Unbind() const;
        /// @brief 设置静态顶点属性
        /// @param index 属性索引
        /// @param v 设置的值
        /// @param val 属性信息
        void SetStatic(GLuint index, const layout_format &val, const void *v);
    };
    //////////////////////////////////////////////////////////////////
    //  OpenGL 着色器类
    //  OpenGL Shader Classes
    //
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
        // to do:buffer块范围方法
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
    //////////////////////////////////////////////////////////////////
    //  OpenGL 绘制类
    //  OpenGL Draw Classes
    //

    template<std::size_t count>
    struct array_render_pack
    {
        VertexArray vao;
        Program* shader;
        VertexBuffer vbos[count];
    };
    
    template<std::size_t count>
    struct index_render_pack
    {
        VertexArray vao;
        Program* shader;
        IndexBuffer ibo;
        VertexBuffer vbos[count];
    };
    
    //////////////////////////////////////////////////////////////////
    //  OpenGL 纹理类
    //  OpenGL Texture Classes
    //
    class Texture
    {
    protected:
        GLuint texture_id;
        GLenum texture_type;

    public:
        Texture(GLenum texture_type)
        {
            glCreateTextures(texture_type, 1, &texture_id);
        }
        ~Texture()
        {
            glDeleteTextures(1, &texture_id);
        }
        void BindTo(GLuint unit)
        {
            glBindTextureUnit(unit, this->texture_id);
        }
        static void UnBindAt(GLuint unit)
        {
            glBindTextureUnit(unit, 0);
        }
    };
    
} // namespace Boundless

#endif //!_BOUNDLESS_GL_UTILITY_HPP_FILE_