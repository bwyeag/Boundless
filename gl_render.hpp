/*
 * BOUNDLESS Engine File
 * Create: 2023/12/2/18:15
 * OpenGL Render C++ Header
 *
 */
#ifndef _BOUNDLESS_GL_RENDER_HPP_FILE_
#define _BOUNDLESS_GL_RENDER_HPP_FILE_
#include "includes/glad/glad.h"
#include "includes/GLFW/glfw3.h"
#include "glm/glm.hpp"

#include <cstdint>
#include <cstddef>

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <algorithm>

#define ERROR(type, info) std::cerr << "[ERROR][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl;
#define ERRORINFO(info) std::cerr << "[ERROR]other:" << info << std::endl;
#define WARNING(type, info) std::cerr << "[WARNING][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl;
#define WARNINGINFO(info) std::cerr << "[WARNING]other:" << info << std::endl;
#define INFO(type, info) std::cout << "[INFO][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl;
#define INFODATA(info) std::cout << "[INFO]other:" << info << std::endl;

namespace Boundless
{
    //////////////////////////////////////////////////////////////////
    //  错误处理部分
    //  Error Handle
    //
    extern const char *string_BUFFER;
    extern const char *string_SHADER;
    extern const char *string_PROGRAM;
    extern const char *string_VERTEX_ARRAY;
    extern const char *string_QUERY;
    extern const char *string_PROGRAM_PIPELINE;
    extern const char *string_TRANSFORM_FEEDBACK;
    extern const char *string_SAMPLER;
    extern const char *string_TEXTURE;
    extern const char *string_RENDERBUFFER;
    extern const char *string_FRAMEBUFFER;
    extern const char *string_null;
    const char *GetObjectType(GLenum o);
    extern const char *string_DEBUG_SOURCE_API;
    extern const char *string_DEBUG_SOURCE_WINDOW_SYSTEM;
    extern const char *string_DEBUG_SOURCE_SHADER_COMPILER;
    extern const char *string_DEBUG_SOURCE_THIRD_PARTY;
    extern const char *string_DEBUG_SOURCE_APPLICATION;
    extern const char *string_DEBUG_SOURCE_OTHER;
    const char *GetSourceFrom(GLenum s);
    extern const char *string_GL_DEBUG_TYPE_ERROR;
    extern const char *string_GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR;
    extern const char *string_GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR;
    extern const char *string_GL_DEBUG_TYPE_PORTABILITY;
    extern const char *string_GL_DEBUG_TYPE_PERFORMANCE;
    extern const char *string_GL_DEBUG_TYPE_MARKER;
    extern const char *string_GL_DEBUG_TYPE_PUSH_GROUP;
    extern const char *string_GL_DEBUG_TYPE_POP_GROUP;
    extern const char *string_GL_DEBUG_TYPE_OTHER;
    const char *GetSourceType(GLenum s);
    extern const char *string_GL_DEBUG_SEVERITY_HIGH;
    extern const char *string_GL_DEBUG_SEVERITY_MEDIUM;
    extern const char *string_GL_DEBUG_SEVERITY_LOW;
    extern const char *string_GL_DEBUG_SEVERITY_NOTIFICATION;
    const char *GetSeverity(GLenum s);
    void opengl_error_callback(GLenum source, GLenum type, GLuint id,
                               GLenum severity, GLsizei length, const GLchar *message, const void *userParam);

    //////////////////////////////////////////////////////////////////
    //  GLFW处理部分
    //  GLFW Handle
    //
    extern GLFWwindow *window_ptr;

    void utility_errorfun(int error_code, const char *description);
    void utility_windowposfun(GLFWwindow *window, int xpos, int ypos);
    void utility_windowsizefun(GLFWwindow *window, int width, int height);
    void utility_windowclosefun(GLFWwindow *window);
    void utility_windowrefreshfun(GLFWwindow *window);
    void utility_windowfocusfun(GLFWwindow *window, int focused);
    void utility_windowiconifyfun(GLFWwindow *window, int iconified);
    void utility_windowmaximizefun(GLFWwindow *window, int maximized);
    void utility_framebuffersizefun(GLFWwindow *window, int width, int height);
    void utility_windowcontentscalefun(GLFWwindow *window, float xscale, float yscale);
    void utility_mousebuttonfun(GLFWwindow *window, int button, int action, int mods);
    void utility_cursorposfun(GLFWwindow *window, double xpos, double ypos);
    void utility_cursorenterfun(GLFWwindow *window, int entered);
    void utility_scrollfun(GLFWwindow *window, double xoffset, double yoffset);
    void utility_keyfun(GLFWwindow *window, int key, int scancode, int action, int mods);
    void utility_charfun(GLFWwindow *window, unsigned int codepoint);
    void utility_charmodsfun(GLFWwindow *window, unsigned int codepoint, int mods);
    void utility_dropfun(GLFWwindow *window, int path_count, const char *paths[]);
    void utility_monitorfun(GLFWmonitor *monitor, int event);
    void utility_joystickfun(int jid, int event);

    void AddErrorCallback(GLFWerrorfun callback);
    void AddMonitorCallback(GLFWmonitorfun callback);
    void AddWindowPosCallback(GLFWwindowposfun callback);
    void AddWindowSizeCallback(GLFWwindowsizefun callback);
    void AddWindowCloseCallback(GLFWwindowclosefun callback);
    void AddWindowRefreshCallback(GLFWwindowrefreshfun callback);
    void AddWindowFocusCallback(GLFWwindowfocusfun callback);
    void AddWindowIconifyCallback(GLFWwindowiconifyfun callback);
    void AddWindowMaximizeCallback(GLFWwindowmaximizefun callback);
    void AddFramebufferSizeCallback(GLFWframebuffersizefun callback);
    void AddWindowContentScaleCallback(GLFWwindowcontentscalefun callback);
    void AddKeyCallback(GLFWkeyfun callback);
    void AddCharCallback(GLFWcharfun callback);
    void AddCharModsCallback(GLFWcharmodsfun callback);
    void AddMouseButtonCallback(GLFWmousebuttonfun callback);
    void AddCursorPosCallback(GLFWcursorposfun callback);
    void AddCursorEnterCallback(GLFWcursorenterfun callback);
    void AddScrollCallback(GLFWscrollfun callback);
    void AddDropCallback(GLFWdropfun callback);
    void AddJoystickCallback(GLFWjoystickfun callback);
//////////////////////////////////////////////////////////////////
//  OpenGL加载部分
//  OpenGL Loading Handle
//
#define SCREEN_INIT_WIDTH 800
#define SCREEN_INIT_HEIGHT 600
#define WINDOW_NAME "Boundless"

    // 窗口大小
    extern unsigned int screen_width;
    extern unsigned int screen_height;

    // 时间处理
    extern double delta_tim;
    extern double current_time;
    extern double last_time;

    // 鼠标信息追踪
    extern double last_mouse_x;
    extern double last_mouse_y;
    extern double delta_mouse_x;
    extern double delta_mouse_y;

    void process_input(GLFWwindow *windowptr);
    void framebuffer_size_callback(GLFWwindow *windowptr, int width, int height);
    void mouse_callback(GLFWwindow *windowptr, double xposIn, double yposIn);

    /// @brief 初始化OpenGL函数
    /// @return 初始化是否成功
    bool opengl_init();
    //////////////////////////////////////////////////////////////////
    //  OpenGL 辅助函数
    //  OpenGL Helper Functions
    //
    constexpr inline bool opengl_is_integer(GLenum type)
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
            throw;
        }
    }
    constexpr inline size_t opengl_type_size(GLenum type)
    {
        switch (type)
        {
        case GL_BYTE:
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
        default:
            throw;
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

        void bind();
        void unbind();
        void bind(GLenum tar);
        void unbind(GLenum tar);
        void set_target(GLenum tar);

        void store_data(GLsizeiptr size, GLbitfield flags, const void *data = nullptr);
        void store_sub_data(const data_range &range, const void *data);
        void fill_data(GLenum internal_format, GLenum format, GLenum type, const void *data);
        void fill_sub_data(const data_range &range, GLenum internal_format, GLenum format, GLenum type, const void *data);

        static void copy_data(Buffer &read, Buffer &write, const data_range &read_range, GLintptr write_offset);

        void get_data(const data_range &range, void *write_to);

        void *map(GLenum access);
        void unmap();
        void *map_sub(const data_range &range, GLbitfield flags);
        void flush_map(const data_range &range);

        void invalidate();
        void invalidate_sub(const data_range &range);
    };
    /// @brief 用于存储顶点数组属性的类
    class layout_element
    {
    public:
        /// @brief 属性数据类型
        GLenum type;
        /// @brief 数据长度
        GLsizei size;
        /// @brief 复合对象包含多个信息
        /// @details 字节排序（高位）-8a-8-8-8d-（低位）a:是否归一化,
        ///          d:数量(1,2,3,4)
        mutable GLuint info;
        /// @brief 无参构造，不进行任何初始化
        layout_element() = default;
        /// @brief 构造函数，初始化数据
        /// @param type 属性数据类型
        /// @param count 数量
        /// @param normalize 是否归一化到[-1,+1]或[0,1]（强制转换为float）
        layout_element(GLenum type, GLuint count, GLboolean normalize);
        /// @brief 移动构造函数（实际只是复制）
        /// @param target 移动目标
        layout_element(layout_element &&target) noexcept = default;
        /// @brief 复制构造函数
        /// @param target 复制目标
        layout_element(const layout_element &target) = default;
        layout_element &operator=(layout_element &target);
        layout_element &operator=(layout_element &&target);
        void set_count(GLuint count) const;
        GLuint get_count() const;
        GLboolean is_normalised() const;
        void set_normalised(bool enable) const;
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
        void set_data(GLenum draw_type, GLenum index_type);
        void set_draw_type(GLenum draw_type);
        void set_data_type(GLenum index_type);
        GLenum get_draw_type() const;
        GLenum get_index_type() const;
        GLsizei get_index_count() const;
        /// @brief 获取id
        operator GLuint() const;
    };

#define DEAUFT_VERTEX_ELEMENT_RESERVE 8

    /// @brief 顶点数据类
    class VertexBuffer : public Buffer
    {
    private:
        GLsizei vertex_size = 0;
        std::vector<layout_element> vertex_layout;

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
        VertexBuffer &operator<<(const layout_element &data);
        operator GLuint() const;

        GLsizei get_size() const;
        const std::vector<layout_element> &get_layout() const;
    };

    /// @brief 顶点数组类
    class VertexArray
    {
    protected:
        /// @brief 在OpenGL中的id
        GLuint array_id;
        /// @brief 当前最后一个未使用的顶点属性索引
        GLuint layout_index;

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
        void use(VertexBuffer &target, const data_range &range, bool enable = false);
        /// @brief 使用全部属性
        void use_all(VertexBuffer &target, bool enable = false);
        void enable(GLuint index);
        void disable(GLuint index);
        /// @brief 绑定VAO为当前VAO
        void bind() const;
        /// @brief 解绑VAO
        void unbind() const;
        /// @brief 设置静态顶点属性
        /// @param index 属性索引
        /// @param v 设置的值
        /// @param val 属性信息
        void set_static(GLuint index, const layout_element &val, const void *v);
    };
} // namespace Boundless

#endif //!_BOUNDLESS_GL_RENDER_HPP_FILE_