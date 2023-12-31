/*
 * BOUNDLESS Engine File
 * Create: 2023/12/2/18:15
 * OpenGL Render C++ File
 *
 */

#include "gl_render.hpp"

namespace Boundless
{
    //////////////////////////////////////////////////////////////////
    //  错误处理部分
    //  Error Handle
    //
    const char *string_BUFFER = "缓存区对象";
    const char *string_SHADER = "着色器对象";
    const char *string_PROGRAM = "程序对象";
    const char *string_VERTEX_ARRAY = "顶点数组对象";
    const char *string_QUERY = "查询对象";
    const char *string_PROGRAM_PIPELINE = "程序管道对象";
    const char *string_TRANSFORM_FEEDBACK = "转换反馈对象";
    const char *string_SAMPLER = "采样器对象";
    const char *string_TEXTURE = "纹理对象";
    const char *string_RENDERBUFFER = "渲染缓存区对象";
    const char *string_FRAMEBUFFER = "帧缓存对象";
    const char *string_null = "未知";
    const char *string_DEBUG_SOURCE_API = "对 OpenGL API 的调用";
    const char *string_DEBUG_SOURCE_WINDOW_SYSTEM = "对窗口系统 API 的调用";
    const char *string_DEBUG_SOURCE_SHADER_COMPILER = "GLSL 编译器";
    const char *string_DEBUG_SOURCE_THIRD_PARTY = "与 OpenGL 关联的应用程序";
    const char *string_DEBUG_SOURCE_APPLICATION = "应用程序的用户";
    const char *string_DEBUG_SOURCE_OTHER = "一些不是其中之一的来源";
    const char *string_GL_DEBUG_TYPE_ERROR = "错误(来自 API)";
    const char *string_GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR = "使用某些标记为已弃用的行为";
    const char *string_GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR = "调用了未定义的行为";
    const char *string_GL_DEBUG_TYPE_PORTABILITY = "用户依赖的某些功能不可移植";
    const char *string_GL_DEBUG_TYPE_PERFORMANCE = "代码触发了可能的性能问题";
    const char *string_GL_DEBUG_TYPE_MARKER = " 命令流注释";
    const char *string_GL_DEBUG_TYPE_PUSH_GROUP = " 组推送";
    const char *string_GL_DEBUG_TYPE_POP_GROUP = "组弹出";
    const char *string_GL_DEBUG_TYPE_OTHER = "某种类型不属于这些类型之一";
    const char *string_GL_DEBUG_SEVERITY_HIGH = "高等级警报";
    const char *string_GL_DEBUG_SEVERITY_MEDIUM = "中等级错误";
    const char *string_GL_DEBUG_SEVERITY_LOW = "低等级错误";
    const char *string_GL_DEBUG_SEVERITY_NOTIFICATION = "消息";
    const char *GetObjectType(GLenum o)
    {
        switch (o)
        {
        case GL_BUFFER:
            return string_BUFFER;
        case GL_SHADER:
            return string_SHADER;
        case GL_PROGRAM:
            return string_PROGRAM;
        case GL_VERTEX_ARRAY:
            return string_VERTEX_ARRAY;
        case GL_QUERY:
            return string_QUERY;
        case GL_PROGRAM_PIPELINE:
            return string_PROGRAM_PIPELINE;
        case GL_TRANSFORM_FEEDBACK:
            return string_TRANSFORM_FEEDBACK;
        case GL_SAMPLER:
            return string_SAMPLER;
        case GL_TEXTURE:
            return string_TEXTURE;
        case GL_RENDERBUFFER:
            return string_RENDERBUFFER;
        case GL_FRAMEBUFFER:
            return string_FRAMEBUFFER;
        default:
            return string_null;
        }
    }
    const char *GetSourceFrom(GLenum s)
    {
        switch (s)
        {
        case GL_DEBUG_SOURCE_API:
            return string_DEBUG_SOURCE_API;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return string_DEBUG_SOURCE_WINDOW_SYSTEM;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return string_DEBUG_SOURCE_SHADER_COMPILER;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return string_DEBUG_SOURCE_THIRD_PARTY;
        case GL_DEBUG_SOURCE_APPLICATION:
            return string_DEBUG_SOURCE_APPLICATION;
        case GL_DEBUG_SOURCE_OTHER:
            return string_DEBUG_SOURCE_OTHER;
        default:
            return string_null;
        }
    }
    const char *GetSourceType(GLenum s)
    {
        switch (s)
        {
        case GL_DEBUG_TYPE_ERROR:
            return string_GL_DEBUG_TYPE_ERROR;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return string_GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return string_GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR;
        case GL_DEBUG_TYPE_PORTABILITY:
            return string_GL_DEBUG_TYPE_PORTABILITY;
        case GL_DEBUG_TYPE_PERFORMANCE:
            return string_GL_DEBUG_TYPE_PERFORMANCE;
        case GL_DEBUG_TYPE_MARKER:
            return string_GL_DEBUG_TYPE_MARKER;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return string_GL_DEBUG_TYPE_PUSH_GROUP;
        case GL_DEBUG_TYPE_POP_GROUP:
            return string_GL_DEBUG_TYPE_POP_GROUP;
        case GL_DEBUG_TYPE_OTHER:
            return string_GL_DEBUG_TYPE_OTHER;
        default:
            return string_null;
        }
    }
    const char *GetSeverity(GLenum s)
    {
        switch (s)
        {
        case GL_DEBUG_SEVERITY_HIGH:
            return string_GL_DEBUG_SEVERITY_HIGH;
        case GL_DEBUG_SEVERITY_MEDIUM:
            return string_GL_DEBUG_SEVERITY_MEDIUM;
        case GL_DEBUG_SEVERITY_LOW:
            return string_GL_DEBUG_SEVERITY_LOW;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            return string_GL_DEBUG_SEVERITY_NOTIFICATION;
        default:
            return string_null;
        }
    }

    void opengl_error_callback(GLenum source, GLenum type, GLuint id,
                               GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
    {
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
            return;
        const char *char_source = GetSourceFrom(source);
        const char *char_type = GetSourceType(type);
        const char *char_severity = GetSeverity(severity);
        ERROR("OpenGL", char_severity << ">from " << char_source << ";type " << char_type << ";id " << id << ";\n"
                                      << message)
    }

    //////////////////////////////////////////////////////////////////
    //  GLFW回调处理部分
    //  GLFW Callback Handle
    //
    GLFWwindow *window_ptr;

    static std::list<GLFWframebuffersizefun> list_callback_framebuffersize;

    static std::forward_list<GLFWerrorfun> list_errorfun;
    static std::forward_list<GLFWwindowposfun> list_windowposfun;
    static std::forward_list<GLFWwindowsizefun> list_windowsizefun;
    static std::forward_list<GLFWwindowclosefun> list_windowclosefun;
    static std::forward_list<GLFWwindowrefreshfun> list_windowrefreshfun;
    static std::forward_list<GLFWwindowfocusfun> list_windowfocusfun;
    static std::forward_list<GLFWwindowiconifyfun> list_windowiconifyfun;
    static std::forward_list<GLFWwindowmaximizefun> list_windowmaximizefun;
    static std::forward_list<GLFWframebuffersizefun> list_framebuffersizefun;
    static std::forward_list<GLFWwindowcontentscalefun> list_windowcontentscalefun;
    static std::forward_list<GLFWmousebuttonfun> list_mousebuttonfun;
    static std::forward_list<GLFWcursorposfun> list_cursorposfun;
    static std::forward_list<GLFWcursorenterfun> list_cursorenterfun;
    static std::forward_list<GLFWscrollfun> list_scrollfun;
    static std::forward_list<GLFWkeyfun> list_keyfun;
    static std::forward_list<GLFWcharfun> list_charfun;
    static std::forward_list<GLFWcharmodsfun> list_charmodsfun;
    static std::forward_list<GLFWdropfun> list_dropfun;
    static std::forward_list<GLFWmonitorfun> list_monitorfun;
    static std::forward_list<GLFWjoystickfun> list_joystickfun;

    void utility_errorfun(int error_code, const char *description)
    {
        for (auto val : list_errorfun)
        {
            val(error_code, description);
        }
    }

    void utility_windowposfun(GLFWwindow *window, int xpos, int ypos)
    {
        for (auto val : list_windowposfun)
        {
            val(window, xpos, ypos);
        }
    }

    void utility_windowsizefun(GLFWwindow *window, int width, int height)
    {
        for (auto val : list_windowsizefun)
        {
            val(window, width, height);
        }
    }

    void utility_windowclosefun(GLFWwindow *window)
    {
        for (auto val : list_windowclosefun)
        {
            val(window);
        }
    }

    void utility_windowrefreshfun(GLFWwindow *window)
    {
        for (auto val : list_windowrefreshfun)
        {
            val(window);
        }
    }

    void utility_windowfocusfun(GLFWwindow *window, int focused)
    {
        for (auto val : list_windowfocusfun)
        {
            val(window, focused);
        }
    }

    void utility_windowiconifyfun(GLFWwindow *window, int iconified)
    {
        for (auto val : list_windowiconifyfun)
        {
            val(window, iconified);
        }
    }

    void utility_windowmaximizefun(GLFWwindow *window, int maximized)
    {
        for (auto val : list_windowmaximizefun)
        {
            val(window, maximized);
        }
    }

    void utility_framebuffersizefun(GLFWwindow *window, int width, int height)
    {
        for (auto val : list_framebuffersizefun)
        {
            val(window, width, height);
        }
    }

    void utility_windowcontentscalefun(GLFWwindow *window, float xscale, float yscale)
    {
        for (auto val : list_windowcontentscalefun)
        {
            val(window, xscale, yscale);
        }
    }

    void utility_mousebuttonfun(GLFWwindow *window, int button, int action, int mods)
    {
        for (auto val : list_mousebuttonfun)
        {
            val(window, button, action, mods);
        }
    }

    void utility_cursorposfun(GLFWwindow *window, double xpos, double ypos)
    {
        for (auto val : list_cursorposfun)
        {
            val(window, xpos, ypos);
        }
    }

    void utility_cursorenterfun(GLFWwindow *window, int entered)
    {
        for (auto val : list_cursorenterfun)
        {
            val(window, entered);
        }
    }

    void utility_scrollfun(GLFWwindow *window, double xoffset, double yoffset)
    {
        for (auto val : list_scrollfun)
        {
            val(window, xoffset, yoffset);
        }
    }

    void utility_keyfun(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        for (auto val : list_keyfun)
        {
            val(window, key, scancode, action, mods);
        }
    }

    void utility_charfun(GLFWwindow *window, unsigned int codepoint)
    {
        for (auto val : list_charfun)
        {
            val(window, codepoint);
        }
    }

    void utility_charmodsfun(GLFWwindow *window, unsigned int codepoint, int mods)
    {
        for (auto val : list_charmodsfun)
        {
            val(window, codepoint, mods);
        }
    }

    void utility_dropfun(GLFWwindow *window, int path_count, const char *paths[])
    {
        for (auto val : list_dropfun)
        {
            val(window, path_count, paths);
        }
    }

    void utility_monitorfun(GLFWmonitor *monitor, int event)
    {
        for (auto val : list_monitorfun)
        {
            val(monitor, event);
        }
    }

    void utility_joystickfun(int jid, int event)
    {
        for (auto val : list_joystickfun)
        {
            val(jid, event);
        }
    }
    void AddErrorCallback(GLFWerrorfun callback)
    {
        if (list_errorfun.empty())
        {
            glfwSetErrorCallback(utility_errorfun);
        }
        list_errorfun.push_front(callback);
    }
    void AddMonitorCallback(GLFWmonitorfun callback)
    {
        if (list_monitorfun.empty())
        {
            glfwSetMonitorCallback(utility_monitorfun);
        }
        list_monitorfun.push_front(callback);
    }
    void AddWindowPosCallback(GLFWwindowposfun callback)
    {
        if (list_windowposfun.empty())
        {
            glfwSetWindowPosCallback(window_ptr, utility_windowposfun);
        }
        list_windowposfun.push_front(callback);
    }
    void AddWindowSizeCallback(GLFWwindowsizefun callback)
    {
        if (list_windowsizefun.empty())
        {
            glfwSetWindowSizeCallback(window_ptr, utility_windowsizefun);
        }
        list_windowsizefun.push_front(callback);
    }
    void AddWindowCloseCallback(GLFWwindowclosefun callback)
    {
        if (list_windowclosefun.empty())
        {
            glfwSetWindowCloseCallback(window_ptr, utility_windowclosefun);
        }
        list_windowclosefun.push_front(callback);
    }
    void AddWindowRefreshCallback(GLFWwindowrefreshfun callback)
    {
        if (list_windowrefreshfun.empty())
        {
            glfwSetWindowRefreshCallback(window_ptr, utility_windowrefreshfun);
        }
        list_windowrefreshfun.push_front(callback);
    }
    void AddWindowFocusCallback(GLFWwindowfocusfun callback)
    {
        if (list_windowfocusfun.empty())
        {
            glfwSetWindowFocusCallback(window_ptr, utility_windowfocusfun);
        }
        list_windowfocusfun.push_front(callback);
    }
    void AddWindowIconifyCallback(GLFWwindowiconifyfun callback)
    {
        if (list_windowiconifyfun.empty())
        {
            glfwSetWindowIconifyCallback(window_ptr, utility_windowiconifyfun);
        }
        list_windowiconifyfun.push_front(callback);
    }
    void AddWindowMaximizeCallback(GLFWwindowmaximizefun callback)
    {
        if (list_windowmaximizefun.empty())
        {
            glfwSetWindowMaximizeCallback(window_ptr, utility_windowmaximizefun);
        }
        list_windowmaximizefun.push_front(callback);
    }
    void AddFramebufferSizeCallback(GLFWframebuffersizefun callback)
    {
        if (list_framebuffersizefun.empty())
        {
            glfwSetFramebufferSizeCallback(window_ptr, utility_framebuffersizefun);
        }
        list_framebuffersizefun.push_front(callback);
    }
    void AddWindowContentScaleCallback(GLFWwindowcontentscalefun callback)
    {
        if (list_windowcontentscalefun.empty())
        {
            glfwSetWindowContentScaleCallback(window_ptr, utility_windowcontentscalefun);
        }
        list_windowcontentscalefun.push_front(callback);
    }
    void AddKeyCallback(GLFWkeyfun callback)
    {
        if (list_keyfun.empty())
        {
            glfwSetKeyCallback(window_ptr, utility_keyfun);
        }
        list_keyfun.push_front(callback);
    }
    void AddCharCallback(GLFWcharfun callback)
    {
        if (list_charfun.empty())
        {
            glfwSetCharCallback(window_ptr, utility_charfun);
        }
        list_charfun.push_front(callback);
    }
    void AddCharModsCallback(GLFWcharmodsfun callback)
    {
        if (list_charmodsfun.empty())
        {
            glfwSetCharModsCallback(window_ptr, utility_charmodsfun);
        }
        list_charmodsfun.push_front(callback);
    }
    void AddMouseButtonCallback(GLFWmousebuttonfun callback)
    {
        if (list_mousebuttonfun.empty())
        {
            glfwSetMouseButtonCallback(window_ptr, utility_mousebuttonfun);
        }
        list_mousebuttonfun.push_front(callback);
    }
    void AddCursorPosCallback(GLFWcursorposfun callback)
    {
        if (list_cursorposfun.empty())
        {
            glfwSetCursorPosCallback(window_ptr, utility_cursorposfun);
        }
        list_cursorposfun.push_front(callback);
    }
    void AddCursorEnterCallback(GLFWcursorenterfun callback)
    {
        if (list_cursorenterfun.empty())
        {
            glfwSetCursorEnterCallback(window_ptr, utility_cursorenterfun);
        }
        list_cursorenterfun.push_front(callback);
    }
    void AddScrollCallback(GLFWscrollfun callback)
    {
        if (list_scrollfun.empty())
        {
            glfwSetScrollCallback(window_ptr, utility_scrollfun);
        }
        list_scrollfun.push_front(callback);
    }
    void AddDropCallback(GLFWdropfun callback)
    {
        if (list_dropfun.empty())
        {
            glfwSetDropCallback(window_ptr, utility_dropfun);
        }
        list_dropfun.push_front(callback);
    }
    void AddJoystickCallback(GLFWjoystickfun callback)
    {
        if (list_joystickfun.empty())
        {
            glfwSetJoystickCallback(utility_joystickfun);
        }
        list_joystickfun.push_front(callback);
    }
    //////////////////////////////////////////////////////////////////
    //  OpenGL加载部分
    //  OpenGL Loading Handle
    //
    // 窗口大小
    unsigned int screen_width = SCREEN_INIT_WIDTH;
    unsigned int screen_height = SCREEN_INIT_HEIGHT;

    // 时间处理
    double delta_time = 0.0;
    double current_time = 0.0;
    double last_time = 0.0;

    // 鼠标信息追踪
    double last_mouse_x = screen_width / 2.0;
    double last_mouse_y = screen_height / 2.0;
    double delta_mouse_x = 0.0;
    double delta_mouse_y = 0.0;
    bool firstMouse = true;

    void process_input(GLFWwindow *windowptr)
    {
        if (glfwGetKey(windowptr, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(windowptr, true);
    }

    void framebuffer_size_callback(GLFWwindow *windowptr, int width, int height)
    {
        screen_height = height;
        screen_width = width;
        glViewport(0, 0, width, height);
    }

    void mouse_callback(GLFWwindow *windowptr, double xposIn, double yposIn)
    {
        if (firstMouse)
        {
            last_mouse_x = xposIn;
            last_mouse_y = yposIn;
            firstMouse = false;
        }
        else
        {
            delta_mouse_x = xposIn - last_mouse_x;
            delta_mouse_y = last_mouse_y - yposIn;
            last_mouse_x = xposIn;
            last_mouse_y = yposIn;
        }
    }

    /// @brief 初始化OpenGL函数
    /// @return 初始化是否成功
    bool opengl_init()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        window_ptr = glfwCreateWindow(SCREEN_INIT_WIDTH, SCREEN_INIT_HEIGHT, WINDOW_NAME, nullptr, nullptr);
        if (window_ptr == nullptr)
        {
            ERROR("GLFW", "创建窗口失败")
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window_ptr);
        // 添加回调函数
        AddFramebufferSizeCallback(framebuffer_size_callback);
        AddCursorPosCallback(mouse_callback);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            ERROR("GLAD", "创建失败")
            glfwTerminate();
            return false;
        }
#ifdef _DEBUG
        GLint flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) == GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(opengl_error_callback, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            INFO("OpenGL", "启用调试上下文成功")
        }
        else
        {
            INFO("OpenGL", "启用调试上下文失败")
        }
#endif
        return true;
    }

    //////////////////////////////////////////////////////////////////
    //  OpenGL 类
    //  OpenGL Classes
    //
    Buffer::Buffer()
    {
        glCreateBuffers(1, &(this->buffer_id));
    }
    Buffer::Buffer(GLenum target)
    {
        glCreateBuffers(1, &(this->buffer_id));
        this->buffer_target = target;
    }
    Buffer::Buffer(GLenum target, GLsizeiptr size, GLbitfield flags, const void *data)
        : Buffer(target)
    {
        StoreData(size, flags, data);
    }
    Buffer::~Buffer()
    {
        glDeleteBuffers(1, &(this->buffer_id));
        this->buffer_id = 0;
    }

    void Buffer::Bind()
    {
        glBindBuffer(this->buffer_id, this->buffer_target);
    }
    void Buffer::Unbind()
    {
        glBindBuffer(0, this->buffer_target);
    }
    void Buffer::Bind(GLenum tar)
    {
        glBindBuffer(this->buffer_id, tar);
    }
    void Buffer::Unbind(GLenum tar)
    {
        glBindBuffer(0, tar);
    }
    void Buffer::SetTarget(GLenum tar)
    {
        this->buffer_target = tar;
    }

    void Buffer::StoreData(GLsizeiptr size, GLbitfield flags, const void *data)
    {
        buffer_size = size;
        buffer_storeflags = flags;
        glNamedBufferStorage(buffer_id, size, data, flags);
    }
    void Buffer::StoreSubData(const data_range &range, const void *data)
    {
        glNamedBufferSubData(buffer_id, range.offset, range.length, data);
    }
    void Buffer::FillData(GLenum internal_format, GLenum format, GLenum type, const void *data)
    {
        glClearNamedBufferData(buffer_id, internal_format, format, type, data);
    }
    void Buffer::FillSubData(const data_range &range, GLenum internal_format, GLenum format, GLenum type, const void *data)
    {
        glClearNamedBufferSubData(buffer_id, internal_format, range.offset, range.length, format, type, data);
    }

    void Buffer::CopyData(Buffer &read, Buffer &write, const data_range &read_range, GLintptr write_offset)
    {
        glCopyNamedBufferSubData(read.buffer_id, write.buffer_id, read_range.offset, read_range.length, write_offset);
    }

    void Buffer::GetData(const data_range &range, void *write_to)

    {
        glGetNamedBufferSubData(buffer_id, range.offset, range.length, write_to);
    }

    void *Buffer::Map(GLenum access)
    {
        map_access = access;
        map_range = {0, buffer_size};
        return glMapNamedBuffer(buffer_id, access);
    }
    void Buffer::Unmap()
    {
        glUnmapNamedBuffer(buffer_id);
    }
    void *Buffer::MapSub(const data_range &range, GLbitfield flags)
    {
        map_bitfield = flags;
        map_range = range;
        return glMapNamedBufferRange(buffer_id, range.offset, range.length, flags);
    }
    void Buffer::FlushMap(const data_range &range)
    {
        glFlushMappedNamedBufferRange(buffer_id, range.offset, range.length);
    }

    void Buffer::Invalidate()
    {
        buffer_size = 0;
        buffer_storeflags = 0;
        glInvalidateBufferData(buffer_id);
    }
    void Buffer::InvalidateSub(const data_range &range)
    {
        glInvalidateBufferSubData(buffer_id, range.offset, range.length);
    }
    Buffer::operator GLuint()
    {
        return buffer_id;
    }
    layout_format::layout_format(GLenum type, GLuint count, GLboolean normalize)
        : type(type), size(count * static_cast<GLuint>(openglTypeSize(type)))
    {
        this->info = 0x00000000;
        if (normalize == GL_TRUE)
        {
            this->info |= 0xFF000000;
        }
        this->info |= static_cast<GLubyte>(count);
    }
    layout_format &layout_format::operator=(layout_format &target)
    {
        if (&target == this)
        {
            return *this;
        }
        type = target.type;
        size = target.size;
        info = target.info;
        return *this;
    }
    layout_format &layout_format::operator=(layout_format &&target)
    {
        if (&target == this)
        {
            return *this;
        }
        type = target.type;
        size = target.size;
        info = target.info;
        return *this;
    }
    void layout_format::set_count(GLuint count) const
    {
#ifdef _DEBUG
        if (count > 0xFF)
        {
            WARNING("INPUT", "count过大")
        }
#endif
        this->info &= 0xFFFFFF00;
        this->info |= static_cast<GLubyte>(count);
    }

    GLuint layout_format::get_count() const
    {
        return this->info & 0x000000FF;
    }
    GLboolean layout_format::is_normalised() const
    {
        return (this->info & 0xFF000000) != 0;
    }
    void layout_format::set_normalised(bool enable) const
    {
        if (enable)
        {
            this->info |= 0xFF000000;
        }
        else
        {
            this->info &= 0x00FFFFFF;
        }
    }
    GLboolean layout_format::is_enable() const
    {
        return (this->info & 0x0000FF00) != 0;
    }
    void layout_format::set_enable(bool enable) const
    {
        if (enable)
        {
            this->info |= 0x0000FF00;
        }
        else
        {
            this->info &= 0xFFFF00FF;
        }
    }
    void layout_format::set_index(GLuint index) const
    {
#ifdef _DEBUG
        if (index > 0xFF)
        {
            WARNING("INPUT","index过大")
        } 
#endif      
        this->info |= (index << 16);
    }
    GLuint layout_format::get_index() const
    {
        return (this->info & 0x00FF0000) >> 16;
    }
    IndexBuffer::IndexBuffer()
        : Buffer(GL_ELEMENT_ARRAY_BUFFER)
    {
    }
    IndexBuffer::IndexBuffer(GLenum draw_type, GLenum index_type, const void *data, GLsizeiptr size, GLbitfield flags)
        : Buffer(GL_ELEMENT_ARRAY_BUFFER, size, flags, data), draw_type(draw_type), index_type(index_type),
          index_count(size / openglTypeSize(index_type))
    {
    }
    IndexBuffer::IndexBuffer(GLsizeiptr size, GLbitfield flags)
        : Buffer(GL_ELEMENT_ARRAY_BUFFER, size, flags)
    {
    }
    void IndexBuffer::SetData(GLenum draw_type, GLenum index_type)
    {
        this->SetDrawType(draw_type);
        this->SetDrawType(index_type);
    }
    void IndexBuffer::SetDrawType(GLenum draw_type)
    {
        this->draw_type = draw_type;
    }
    void IndexBuffer::SetDataType(GLenum index_type)
    {
        this->index_type = index_type;
        this->index_count = this->buffer_size / openglTypeSize(index_type);
    }
    GLenum IndexBuffer::GetDrawType() const
    {
        return this->draw_type;
    }
    GLenum IndexBuffer::GetIndexType() const
    {
        return this->index_type;
    }
    GLsizei IndexBuffer::GetIndexCount() const
    {
        return this->index_count;
    }
    IndexBuffer::operator GLuint() const
    {
        return this->buffer_id;
    }
    VertexBuffer::VertexBuffer(uint32_t reserve)
        : Buffer(GL_ARRAY_BUFFER)
    {
        this->vertex_layout.reserve(reserve);
    }
    VertexBuffer::VertexBuffer(GLsizeiptr size,
                               GLbitfield flags,
                               uint32_t reserve,
                               const void *data)
        : Buffer(GL_ARRAY_BUFFER, size, flags, data)
    {
        vertex_layout.reserve(reserve);
    }

    VertexBuffer &VertexBuffer::operator<<(const layout_format &data)
    {
        this->vertex_size += data.size;
        this->vertex_layout.push_back(data);
        return *this;
    }
    VertexBuffer::operator GLuint() const
    {
        return this->buffer_id;
    }
    GLsizei VertexBuffer::GetSize() const
    {
        return this->vertex_size;
    }
    const std::vector<layout_format> &VertexBuffer::GetLayout() const
    {
        return this->vertex_layout;
    }

    UniformBuffer::UniformBuffer()
        : Buffer(GL_UNIFORM_BUFFER)
    {
    }
    UniformBuffer::UniformBuffer(const void *data, GLsizeiptr size, GLbitfield flags)
        : Buffer(GL_UNIFORM_BUFFER, size, flags, data)
    {
    }
    UniformBuffer::UniformBuffer(GLsizeiptr size, GLbitfield flags)
        : Buffer(GL_UNIFORM_BUFFER, size, flags)
    {
    }
    void UniformBuffer::BindTo(GLuint index)
    {
        binding = index;
        glBindBufferBase(GL_UNIFORM_BUFFER, index, buffer_id);
    }

    ShaderStorageBuffer::ShaderStorageBuffer()
        : Buffer(GL_SHADER_STORAGE_BUFFER)
    {
    }
    ShaderStorageBuffer::ShaderStorageBuffer(const void *data, GLsizeiptr size, GLbitfield flags)
        : Buffer(GL_SHADER_STORAGE_BUFFER, size, flags, data)
    {
    }
    ShaderStorageBuffer::ShaderStorageBuffer(GLsizeiptr size, GLbitfield flags)
        : Buffer(GL_SHADER_STORAGE_BUFFER, size, flags)
    {
    }
    void ShaderStorageBuffer::BindTo(GLuint index)
    {
        binding = index;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffer_id);
    }
    VertexArray::VertexArray()
    {
        glCreateVertexArrays(1, &this->array_id);
    }

    void VertexArray::Use(VertexBuffer &target, const data_range &range, bool enable, bool set)
    {
        size_t offset = 0;
        for (GLuint i = range.offset; i < range.offset + range.length; i++)
        {
            const layout_format &val = target.GetLayout()[i];
            if ((val.is_normalised() == GL_FALSE) && openglIsInteger(val.type))
            {
                glVertexAttribIPointer(i, val.get_count(), val.type, target.GetSize(),
                                       (const void *)offset);
            }
            else if (val.type == GL_DOUBLE)
            {
                glVertexAttribLPointer(i, val.get_count(), val.type, target.GetSize(),
                                       (const void *)offset);
            }
            else
            {
                glVertexAttribPointer(i, val.get_count(), val.type, val.is_normalised(),
                                      target.GetSize(), (const void *)offset);
            }
            if (set)
            {
                val.set_index(layout_index);
                val.set_enable(enable);
            }
            if (enable)
            {
                glEnableVertexAttribArray(this->layout_index);
            }
            this->layout_index++;
            offset += val.size;
        }
    }
    inline void VertexArray::UseAll(VertexBuffer &target, bool enable, bool set)
    {
        this->Use(target, {0, GLsizei(target.GetLayout().size())}, enable,set);
    }
    inline void VertexArray::Enable(GLuint index)
    {
#ifdef _DEBUG
        assert(index <= layout_index);
#endif
        glEnableVertexAttribArray(index);
    }
    inline void VertexArray::Disable(GLuint index)
    {
#ifdef _DEBUG
        assert(index <= layout_index);
#endif
        glDisableVertexAttribArray(index);
    }
    inline void VertexArray::Bind() const
    {
        glBindVertexArray(this->array_id);
    }
    inline void VertexArray::Unbind() const
    {
        glBindVertexArray(0);
    }
    void VertexArray::SetStatic(GLuint index, const layout_format &val, const void *v)
    {
        if (openglIsInteger(val.type))
        {
            if ((val.is_normalised() == GL_FALSE))
            {
                if (val.type == GL_INT)
                {
                    switch (val.get_count())
                    {
                    case 1:
                        glVertexAttribI1iv(index, (const GLint *)v);
                        return;
                    case 2:
                        glVertexAttribI2iv(index, (const GLint *)v);
                        return;
                    case 3:
                        glVertexAttribI3iv(index, (const GLint *)v);
                        return;
                    case 4:
                        glVertexAttribI4iv(index, (const GLint *)v);
                        return;
                    default:
                        throw;
                    }
                }
                else if (GL_UNSIGNED_BYTE)
                {
                    switch (val.get_count())
                    {
                    case 1:
                        glVertexAttribI1uiv(index, (const GLuint *)v);
                        return;
                    case 2:
                        glVertexAttribI2uiv(index, (const GLuint *)v);
                        return;
                    case 3:
                        glVertexAttribI3uiv(index, (const GLuint *)v);
                        return;
                    case 4:
                        glVertexAttribI4uiv(index, (const GLuint *)v);
                        return;
                    default:
                        throw;
                    }
                }
                else if (val.get_count() == 4)
                {
                    switch (val.type)
                    {
                    case GL_BYTE:
                        glVertexAttribI4bv(index, (const GLbyte *)v);
                        return;
                    case GL_UNSIGNED_BYTE:
                        glVertexAttribI4ubv(index, (const GLubyte *)v);
                        return;
                    case GL_SHORT:
                        glVertexAttribI4sv(index, (const GLshort *)v);
                        return;
                    case GL_UNSIGNED_SHORT:
                        glVertexAttribI4usv(index, (const GLushort *)v);
                        return;
                    default:
                        throw;
                    }
                }
                else
                {
                    throw;
                }
            }
            else
            {
                switch (val.type)
                {
                case GL_BYTE:
                    glVertexAttrib4Nbv(index, (const GLbyte *)v);
                    return;
                case GL_UNSIGNED_BYTE:
                    glVertexAttrib4Nubv(index, (const GLubyte *)v);
                    return;
                case GL_SHORT:
                    glVertexAttrib4Nsv(index, (const GLshort *)v);
                    return;
                case GL_UNSIGNED_SHORT:
                    glVertexAttrib4Nusv(index, (const GLushort *)v);
                    return;
                case GL_INT:
                    glVertexAttrib4Niv(index, (const GLint *)v);
                    return;
                case GL_UNSIGNED_INT:
                    glVertexAttrib4Nuiv(index, (const GLuint *)v);
                    return;
                default:
                    break;
                }
            }
        }
        if (val.type == GL_DOUBLE)
        {
            switch (val.get_count())
            {
            case 1:
                glVertexAttribL1dv(index, (const GLdouble *)v);
                return;
            case 2:
                glVertexAttribL1dv(index, (const GLdouble *)v);
                return;
            case 3:
                glVertexAttribL3dv(index, (const GLdouble *)v);
                return;
            case 4:
                glVertexAttribL4dv(index, (const GLdouble *)v);
                return;
            default:
                break;
            }
        }
        switch (val.get_count())
        {
        case 1:
            switch (val.type)
            {
            case GL_FLOAT:
                glVertexAttrib1fv(index, (const GLfloat *)v);
                return;
            case GL_DOUBLE:
                glVertexAttrib1dv(index, (const GLdouble *)v);
                return;
            case GL_SHORT:
                glVertexAttrib1sv(index, (const GLshort *)v);
                return;
            default:
                throw;
            }
        case 2:
            switch (val.type)
            {
            case GL_FLOAT:
                glVertexAttrib2fv(index, (const GLfloat *)v);
                return;
            case GL_DOUBLE:
                glVertexAttrib2dv(index, (const GLdouble *)v);
                return;
            case GL_SHORT:
                glVertexAttrib2sv(index, (const GLshort *)v);
                return;
            default:
                throw;
            }
        case 3:
            switch (val.type)
            {
            case GL_FLOAT:
                glVertexAttrib3fv(index, (const GLfloat *)v);
                return;
            case GL_DOUBLE:
                glVertexAttrib3dv(index, (const GLdouble *)v);
                return;
            case GL_SHORT:
                glVertexAttrib3sv(index, (const GLshort *)v);
                return;
            default:
                throw;
            }
        case 4:
            switch (val.type)
            {
            case GL_FLOAT:
                glVertexAttrib4fv(index, (const GLfloat *)v);
                return;
            case GL_DOUBLE:
                glVertexAttrib4dv(index, (const GLdouble *)v);
                return;
            case GL_SHORT:
                glVertexAttrib4sv(index, (const GLshort *)v);
                return;
            case GL_BYTE:
                glVertexAttrib4bv(index, (const GLbyte *)v);
                return;
            case GL_UNSIGNED_BYTE:
                glVertexAttrib4ubv(index, (const GLubyte *)v);
                return;
            case GL_INT:
                glVertexAttrib4iv(index, (const GLint *)v);
                return;
            case GL_UNSIGNED_INT:
                glVertexAttrib4uiv(index, (const GLuint *)v);
                return;
            default:
                throw;
            }
        default:
            throw;
        }
    }

    Shader::Shader(const std::string &path, GLenum type)
        : shader_type(type)
    {

        std::ifstream reader(path, std::ios::in);
        if (!reader.is_open())
        {
            this->shader_id = 0;
            ERROR("Engine", "未能成功打开文件：")
            ERRORINFO(path)
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
            ERROR("OpenGL", "着色器编译失败:\n着色器文件:")
            ERRORINFO(shader_code << "\n----------------------------------------")
            char *log = (char *)malloc(sizeof(char) * length);
            if (log == nullptr)
            {
                ERROR("Memory", "日志内存申请失败")
                throw std::bad_alloc();
            }
            else
            {
                glGetShaderInfoLog(this->shader_id, length, &length, log);
                ERROR("OpenGL", "错误信息：\n")
                ERRORINFO("错误信息：\n"
                          << log)
                free(log);
            }
        }
    }
    Shader::Shader(const char *data, GLenum type)
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
            ERROR("OpenGL", "着色器编译失败:\n着色器文件:")
            ERRORINFO(data << "\n----------------------------------------")
            char *log = (char *)malloc(sizeof(char) * length);
            if (log == nullptr)
            {
                ERROR("Memory", "日志内存申请失败")
                throw std::bad_alloc();
            }
            else
            {
                glGetShaderInfoLog(this->shader_id, length, &length, log);
                ERROR("OpenGL", "错误信息：\n")
                ERRORINFO("错误信息：\n"
                          << log)
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
            ERROR("OpenGL", "着色器程序链接错误")
            char *log = (char *)malloc(length);
            if (log == nullptr)
            {
                ERROR("OpenGL", "日志内存申请失败")
                throw std::bad_alloc();
            }
            else
            {
                glGetProgramInfoLog(this->program_id, PROGRAM_LOG_MAX_SIZE, nullptr, log);
                ERROR("OpenGL", "错误信息：")
                ERRORINFO(log)
                free(log);
            }
        }
    }
#endif
    void Program::SetUniformblockBinding(const std::string &name, GLuint index)
    {
        GLint location = GetUniformBlockLocation(name);
        if (location != -1)
            glUniformBlockBinding(this->program_id, location, index);
    }
    GLint Program::GetUniformblockSize(const std::string &name)
    {
        GLint location = GetUniformBlockLocation(name), size;
        if (location == -1)
            return 0;
        glGetActiveUniformBlockiv(program_id, location, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
        return size;
    }
    void Program::SetUniformblock(const std::string &name, UniformBuffer &ubo)
    {
        GLint location = GetUniformBlockLocation(name);
        if (location == -1)
            return;
        ubo.Bind();
        glBindBufferBase(GL_UNIFORM_BUFFER, location, GLuint(ubo));
    }
    void Program::SetUniformblock(const std::string &name, UniformBuffer &ubo, const data_range &range)
    {
        GLint location = GetUniformBlockLocation(name);
        if (location == -1)
            return;
        glBindBufferRange(GL_UNIFORM_BUFFER, location, GLuint(ubo), range.offset, range.length);
    }
    // 设置纹理，使value对应的纹理单元被使用（注：先glActiveTexture）
    void Program::SetTexture(const std::string &name, GLint unit)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform1i(location, unit);
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
            glUniform2fv(location, count, (GLfloat*)value);
    }
    // 设置三维向量数组（float）
    void Program::SetVec3Array(const std::string &name, GLsizei count, const glm::vec3 *value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform3fv(location, count, (GLfloat*)value);
    }
    // 设置四维向量数组（float）
    void Program::SetVec4Array(const std::string &name, GLsizei count, const glm::vec4 *value)
    {
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform4fv(location, count, (GLfloat*)value);
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
} // namespace Boundless