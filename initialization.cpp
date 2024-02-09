#include "initlizition.hpp"

namespace Boundless::Initlization
{
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
} // namespace Boundless::Initlization