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
    const char* string_BUFFER = "缓存区对象";
    const char* string_SHADER = "着色器对象";
    const char* string_PROGRAM = "程序对象";
    const char* string_VERTEX_ARRAY = "顶点数组对象";
    const char* string_QUERY = "查询对象";
    const char* string_PROGRAM_PIPELINE = "程序管道对象";
    const char* string_TRANSFORM_FEEDBACK = "转换反馈对象";
    const char* string_SAMPLER = "采样器对象";
    const char* string_TEXTURE = "纹理对象";
    const char* string_RENDERBUFFER = "渲染缓存区对象";
    const char* string_FRAMEBUFFER = "帧缓存对象";
    const char* string_null = "未知";
    const char* string_DEBUG_SOURCE_API = "对 OpenGL API 的调用";
    const char* string_DEBUG_SOURCE_WINDOW_SYSTEM = "对窗口系统 API 的调用";
    const char* string_DEBUG_SOURCE_SHADER_COMPILER = "GLSL 编译器";
    const char* string_DEBUG_SOURCE_THIRD_PARTY = "与 OpenGL 关联的应用程序";
    const char* string_DEBUG_SOURCE_APPLICATION = "应用程序的用户";
    const char* string_DEBUG_SOURCE_OTHER = "一些不是其中之一的来源";
    const char* string_GL_DEBUG_TYPE_ERROR = "错误(来自 API)";
    const char* string_GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR = "使用某些标记为已弃用的行为";
    const char* string_GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR = "调用了未定义的行为";
    const char* string_GL_DEBUG_TYPE_PORTABILITY = "用户依赖的某些功能不可移植";
    const char* string_GL_DEBUG_TYPE_PERFORMANCE = "代码触发了可能的性能问题";
    const char* string_GL_DEBUG_TYPE_MARKER = " 命令流注释";
    const char* string_GL_DEBUG_TYPE_PUSH_GROUP = " 组推送";
    const char* string_GL_DEBUG_TYPE_POP_GROUP = "组弹出";
    const char* string_GL_DEBUG_TYPE_OTHER = "某种类型不属于这些类型之一";
    const char* string_GL_DEBUG_SEVERITY_HIGH = "高等级警报";
    const char* string_GL_DEBUG_SEVERITY_MEDIUM = "中等级错误";
    const char* string_GL_DEBUG_SEVERITY_LOW = "低等级错误";
    const char* string_GL_DEBUG_SEVERITY_NOTIFICATION = "消息";
    const char* GetObjectType(GLenum o)
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
    const char* GetSourceFrom(GLenum s)
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
    const char* GetSourceType(GLenum s)
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
    const char* GetSeverity(GLenum s)
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
        GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
    {
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
        const char* char_source = GetSourceFrom(source);
        const char* char_type = GetSourceType(type);
        const char* char_severity = GetSeverity(severity);
        ERROR("OpenGL",char_severity << ">from " << char_source << ";type " << char_type << ";id " << id << ";\n" << message)
    }

//////////////////////////////////////////////////////////////////
//  GLFW回调处理部分
//  GLFW Callback Handle
//
    GLFWwindow* window_ptr;

    static std::list<GLFWframebuffersizefun> list_callback_framebuffersize;

    static std::list<GLFWerrorfun> list_errorfun;
    static std::list<GLFWwindowposfun> list_windowposfun;
    static std::list<GLFWwindowsizefun> list_windowsizefun;
    static std::list<GLFWwindowclosefun> list_windowclosefun;
    static std::list<GLFWwindowrefreshfun> list_windowrefreshfun;
    static std::list<GLFWwindowfocusfun> list_windowfocusfun;
    static std::list<GLFWwindowiconifyfun> list_windowiconifyfun;
    static std::list<GLFWwindowmaximizefun> list_windowmaximizefun;
    static std::list<GLFWframebuffersizefun> list_framebuffersizefun;
    static std::list<GLFWwindowcontentscalefun> list_windowcontentscalefun;
    static std::list<GLFWmousebuttonfun> list_mousebuttonfun;
    static std::list<GLFWcursorposfun> list_cursorposfun;
    static std::list<GLFWcursorenterfun> list_cursorenterfun;
    static std::list<GLFWscrollfun> list_scrollfun;
    static std::list<GLFWkeyfun> list_keyfun;
    static std::list<GLFWcharfun> list_charfun;
    static std::list<GLFWcharmodsfun> list_charmodsfun;
    static std::list<GLFWdropfun> list_dropfun;
    static std::list<GLFWmonitorfun> list_monitorfun;
    static std::list<GLFWjoystickfun> list_joystickfun;

    void utility_errorfun(int error_code, const char* description)
    {
        for (auto val : list_errorfun)
        {
            val(error_code, description);
        }
    }

    void utility_windowposfun(GLFWwindow* window, int xpos, int ypos)
    {
        for (auto val : list_windowposfun)
        {
            val(window, xpos, ypos);
        }
    }

    void utility_windowsizefun(GLFWwindow* window, int width, int height)
    {
        for (auto val : list_windowsizefun)
        {
            val(window, width, height);
        }
    }

    void utility_windowclosefun(GLFWwindow* window)
    {
        for (auto val : list_windowclosefun)
        {
            val(window);
        }
    }

    void utility_windowrefreshfun(GLFWwindow* window)
    {
        for (auto val : list_windowrefreshfun)
        {
            val(window);
        }
    }

    void utility_windowfocusfun(GLFWwindow* window, int focused)
    {
        for (auto val : list_windowfocusfun)
        {
            val(window, focused);
        }
    }

    void utility_windowiconifyfun(GLFWwindow* window, int iconified)
    {
        for (auto val : list_windowiconifyfun)
        {
            val(window, iconified);
        }
    }

    void utility_windowmaximizefun(GLFWwindow* window, int maximized)
    {
        for (auto val : list_windowmaximizefun)
        {
            val(window, maximized);
        }
    }

    void utility_framebuffersizefun(GLFWwindow* window, int width, int height)
    {
        for (auto val : list_framebuffersizefun)
        {
            val(window, width, height);
        }
    }

    void utility_windowcontentscalefun(GLFWwindow* window, float xscale, float yscale)
    {
        for (auto val : list_windowcontentscalefun)
        {
            val(window, xscale, yscale);
        }
    }

    void utility_mousebuttonfun(GLFWwindow* window, int button, int action, int mods)
    {
        for (auto val : list_mousebuttonfun)
        {
            val(window, button, action, mods);
        }
    }

    void utility_cursorposfun(GLFWwindow* window, double xpos, double ypos)
    {
        for (auto val : list_cursorposfun)
        {
            val(window, xpos, ypos);
        }
    }

    void utility_cursorenterfun(GLFWwindow* window, int entered)
    {
        for (auto val : list_cursorenterfun)
        {
            val(window, entered);
        }
    }

    void utility_scrollfun(GLFWwindow* window, double xoffset, double yoffset)
    {
        for (auto val : list_scrollfun)
        {
            val(window, xoffset, yoffset);
        }
    }

    void utility_keyfun(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        for (auto val : list_keyfun)
        {
            val(window, key, scancode, action, mods);
        }
    }

    void utility_charfun(GLFWwindow* window, unsigned int codepoint)
    {
        for (auto val : list_charfun)
        {
            val(window, codepoint);
        }
    }

    void utility_charmodsfun(GLFWwindow* window, unsigned int codepoint, int mods)
    {
        for (auto val : list_charmodsfun)
        {
            val(window, codepoint, mods);
        }
    }

    void utility_dropfun(GLFWwindow* window, int path_count, const char* paths[])
    {
        for (auto val : list_dropfun)
        {
            val(window, path_count, paths);
        }
    }

    void utility_monitorfun(GLFWmonitor* monitor, int event)
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
        list_errorfun.push_back(callback);
    }
    void AddMonitorCallback(GLFWmonitorfun callback)
    {
        list_monitorfun.push_back(callback);
    }
    void AddWindowPosCallback(GLFWwindowposfun callback)
    {
        list_windowposfun.push_back(callback);
    }
    void AddWindowSizeCallback(GLFWwindowsizefun callback)
    {
        list_windowsizefun.push_back(callback);
    }
    void AddWindowCloseCallback(GLFWwindowclosefun callback)
    {
        list_windowclosefun.push_back(callback);
    }
    void AddWindowRefreshCallback(GLFWwindowrefreshfun callback)
    {
        list_windowrefreshfun.push_back(callback);
    }
    void AddWindowFocusCallback(GLFWwindowfocusfun callback)
    {
        list_windowfocusfun.push_back(callback);
    }
    void AddWindowIconifyCallback(GLFWwindowiconifyfun callback)
    {
        list_windowiconifyfun.push_back(callback);
    }
    void AddWindowMaximizeCallback(GLFWwindowmaximizefun callback)
    {
        list_windowmaximizefun.push_back(callback);
    }
    void AddFramebufferSizeCallback(GLFWframebuffersizefun callback)
    {
        list_framebuffersizefun.push_back(callback);
    }
    void AddWindowContentScaleCallback(GLFWwindowcontentscalefun callback)
    {
        list_windowcontentscalefun.push_back(callback);
    }
    void AddKeyCallback(GLFWkeyfun callback)
    {
        list_keyfun.push_back(callback);
    }
    void AddCharCallback(GLFWcharfun callback)
    {
        list_charfun.push_back(callback);
    }
    void AddCharModsCallback(GLFWcharmodsfun callback)
    {
        list_charmodsfun.push_back(callback);
    }
    void AddMouseButtonCallback(GLFWmousebuttonfun callback)
    {
        list_mousebuttonfun.push_back(callback);
    }
    void AddCursorPosCallback(GLFWcursorposfun callback)
    {
        list_cursorposfun.push_back(callback);
    }
    void AddCursorEnterCallback(GLFWcursorenterfun callback)
    {
        list_cursorenterfun.push_back(callback);
    }
    void AddScrollCallback(GLFWscrollfun callback)
    {
        list_scrollfun.push_back(callback);
    }
    void AddDropCallback(GLFWdropfun callback)
    {
        list_dropfun.push_back(callback);
    }
    void AddJoystickCallback(GLFWjoystickfun callback)
    {
        list_joystickfun.push_back(callback);
    }
//////////////////////////////////////////////////////////////////
//  OpenGL加载部分
//  OpenGL Loading Handle
//
    //窗口大小
    unsigned int screen_width = SCREEN_INIT_WIDTH;
    unsigned int screen_height = SCREEN_INIT_HEIGHT;

    //时间处理
    double delta_time = 0.0;
    double current_time = 0.0;
    double last_time = 0.0;

    //鼠标信息追踪
    double last_mouse_x = screen_width / 2.0;
    double last_mouse_y = screen_height / 2.0;
    double delta_mouse_x = 0.0;
    double delta_mouse_y = 0.0;
    bool firstMouse = true;

    void process_input(GLFWwindow* windowptr)
    {
        if (glfwGetKey(windowptr, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(windowptr, true);

    }

    void framebuffer_size_callback(GLFWwindow* windowptr, int width, int height)
    {
        screen_height = height;
        screen_width  = width;
        glViewport(0, 0, width, height);
    }

    void mouse_callback(GLFWwindow* windowptr, double xposIn, double yposIn)
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
            ERROR("GLFW","创建窗口失败")
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window_ptr);
        //设置回调函数
        {
            glfwSetErrorCallback(utility_errorfun);
            glfwSetMonitorCallback(utility_monitorfun);
            glfwSetWindowPosCallback(window_ptr, utility_windowposfun);
            glfwSetWindowSizeCallback(window_ptr, utility_windowsizefun);
            glfwSetWindowCloseCallback(window_ptr, utility_windowclosefun);
            glfwSetWindowRefreshCallback(window_ptr, utility_windowrefreshfun);
            glfwSetWindowFocusCallback(window_ptr, utility_windowfocusfun);
            glfwSetWindowIconifyCallback(window_ptr, utility_windowiconifyfun);
            glfwSetWindowMaximizeCallback(window_ptr, utility_windowmaximizefun);
            glfwSetFramebufferSizeCallback(window_ptr, utility_framebuffersizefun);
            glfwSetWindowContentScaleCallback(window_ptr, utility_windowcontentscalefun);
            glfwSetKeyCallback(window_ptr, utility_keyfun);
            glfwSetCharCallback(window_ptr, utility_charfun);
            glfwSetCharModsCallback(window_ptr, utility_charmodsfun);
            glfwSetMouseButtonCallback(window_ptr, utility_mousebuttonfun);
            glfwSetCursorPosCallback(window_ptr, utility_cursorposfun);
            glfwSetCursorEnterCallback(window_ptr, utility_cursorenterfun);
            glfwSetScrollCallback(window_ptr, utility_scrollfun);
            glfwSetDropCallback(window_ptr, utility_dropfun);
            glfwSetJoystickCallback(utility_joystickfun);
        }
        //添加回调函数
        AddFramebufferSizeCallback(framebuffer_size_callback);
        AddCursorPosCallback(mouse_callback);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            ERROR("GLAD","创建失败")
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
            INFO("OpenGL","启用调试上下文成功")
        }
        else
        {
            INFO("OpenGL","启用调试上下文失败")
        }
#endif
        return true;
    }
} // namespace Boundless