#ifndef _INITIALIZATION_HPP_FILE_
#define _INITIALIZATION_HPP_FILE_

namespace Boundless::Initlization
{
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
} // namespace Boundless::Initlization

#endif //!_INITIALIZATION_HPP_FILE_