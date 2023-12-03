/*
 * BOUNDLESS Engine File
 * Create: 2023/12/2/18:15
 * OpenGL Render C++ Header
 *
 */
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
    //  OpenGL 类
    //  OpenGL Classes
    //
    struct data_range
    {
        GLintptr offset;
        GLsizeiptr length;
    };

    class Buffer
    {
    private:
        GLuint buffer_id;
        GLenum buffer_target;
        GLsizeiptr buffer_size;
        GLbitfield buffer_storeflags ;
        union
        {
            GLenum map_access;
            GLbitfield map_bitfield;
        };
        data_range map_range;

    public:
        Buffer();
        Buffer(GLenum);
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
        void *map_range(const data_range &range, GLbitfield flags);
        void flush_map(const data_range &range);

        void invalidate();
        void invalidate_sub(const data_range &range);
    };

} // namespace Boundless
