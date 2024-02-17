#ifndef _INITIALIZATION_HPP_FILE_
#define _INITIALIZATION_HPP_FILE_
#include "bl_log.hpp"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <forward_list>
namespace Boundless::Init
{
    //////////////////////////////////////////////////////////////////
    //  GLFW处理部分
    //  GLFW Handle
    //
    struct WindowInfo
    {
        GLFWwindow *window_ptr;
        // 窗口大小
        unsigned int screen_width;
        unsigned int screen_height;

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
    };
    extern WindowInfo windowinfo;

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

    void process_input(GLFWwindow *windowptr);
    void framebuffer_size_callback(GLFWwindow *windowptr, int width, int height);
    void mouse_callback(GLFWwindow *windowptr, double xposIn, double yposIn);

    /// @brief 初始化OpenGL函数
    /// @return 初始化是否成功
    bool InitOpenGL();
} // namespace Boundless::Initlization

#endif //!_INITIALIZATION_HPP_FILE_