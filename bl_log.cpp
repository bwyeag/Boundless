#include "bl_log.hpp"

namespace Boundless
{
#ifdef _DEBUG
    /*OpenGL错误文本*/
    static const char *const str_BUFFER = "缓存区对象";
    static const char *const str_SHADER = "着色器对象";
    static const char *const str_PROGRAM = "程序对象";
    static const char *const str_VERTEX_ARRAY = "顶点数组对象";
    static const char *const str_QUERY = "查询对象";
    static const char *const str_PROGRAM_PIPELINE = "程序管道对象";
    static const char *const str_TRANSFORM_FEEDBACK = "转换反馈对象";
    static const char *const str_SAMPLER = "采样器对象";
    static const char *const str_TEXTURE = "纹理对象";
    static const char *const str_RENDERBUFFER = "渲染缓存区对象";
    static const char *const str_FRAMEBUFFER = "帧缓存对象";
    static const char *const str_null = "未知";
    static const char *const str_DEBUG_SOURCE_API = "对 OpenGL API 的调用";
    static const char *const str_DEBUG_SOURCE_WINDOW_SYSTEM = "对窗口系统 API 的调用";
    static const char *const str_DEBUG_SOURCE_SHADER_COMPILER = "GLSL 编译器";
    static const char *const str_DEBUG_SOURCE_THIRD_PARTY = "与 OpenGL 关联的应用程序";
    static const char *const str_DEBUG_SOURCE_APPLICATION = "应用程序的用户";
    static const char *const str_DEBUG_SOURCE_OTHER = "一些不是其中之一的来源";
    static const char *const str_GL_DEBUG_TYPE_ERROR = "错误(来自 API)";
    static const char *const str_GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR = "使用某些标记为已弃用的行为";
    static const char *const str_GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR = "调用了未定义的行为";
    static const char *const str_GL_DEBUG_TYPE_PORTABILITY = "用户依赖的某些功能不可移植";
    static const char *const str_GL_DEBUG_TYPE_PERFORMANCE = "代码触发了可能的性能问题";
    static const char *const str_GL_DEBUG_TYPE_MARKER = " 命令流注释";
    static const char *const str_GL_DEBUG_TYPE_PUSH_GROUP = " 组推送";
    static const char *const str_GL_DEBUG_TYPE_POP_GROUP = "组弹出";
    static const char *const str_GL_DEBUG_TYPE_OTHER = "某种类型不属于这些类型之一";
    static const char *const str_GL_DEBUG_SEVERITY_HIGH = "高等级警报";
    static const char *const str_GL_DEBUG_SEVERITY_MEDIUM = "中等级错误";
    static const char *const str_GL_DEBUG_SEVERITY_LOW = "低等级错误";
    static const char *const str_GL_DEBUG_SEVERITY_NOTIFICATION = "消息";

    static const char *const str_SHADER_COMPILE = "着色器编译失败:\n着色器文件:";
    static const char *const str_SHADER_LINK = "着色器程序链接错误";

    const char *OpenGLError::GetObjectType(GLenum o)
    {
        switch (o)
        {
        case GL_BUFFER:
            return str_BUFFER;
        case GL_SHADER:
            return str_SHADER;
        case GL_PROGRAM:
            return str_PROGRAM;
        case GL_VERTEX_ARRAY:
            return str_VERTEX_ARRAY;
        case GL_QUERY:
            return str_QUERY;
        case GL_PROGRAM_PIPELINE:
            return str_PROGRAM_PIPELINE;
        case GL_TRANSFORM_FEEDBACK:
            return str_TRANSFORM_FEEDBACK;
        case GL_SAMPLER:
            return str_SAMPLER;
        case GL_TEXTURE:
            return str_TEXTURE;
        case GL_RENDERBUFFER:
            return str_RENDERBUFFER;
        case GL_FRAMEBUFFER:
            return str_FRAMEBUFFER;
        default:
            return str_null;
        }
    }
    const char *OpenGLError::GetSourceFrom(GLenum s)
    {
        switch (s)
        {
        case GL_DEBUG_SOURCE_API:
            return str_DEBUG_SOURCE_API;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return str_DEBUG_SOURCE_WINDOW_SYSTEM;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return str_DEBUG_SOURCE_SHADER_COMPILER;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return str_DEBUG_SOURCE_THIRD_PARTY;
        case GL_DEBUG_SOURCE_APPLICATION:
            return str_DEBUG_SOURCE_APPLICATION;
        case GL_DEBUG_SOURCE_OTHER:
            return str_DEBUG_SOURCE_OTHER;
        default:
            return str_null;
        }
    }
    const char *OpenGLError::GetSourceType(GLenum s)
    {
        switch (s)
        {
        case GL_DEBUG_TYPE_ERROR:
            return str_GL_DEBUG_TYPE_ERROR;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return str_GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return str_GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR;
        case GL_DEBUG_TYPE_PORTABILITY:
            return str_GL_DEBUG_TYPE_PORTABILITY;
        case GL_DEBUG_TYPE_PERFORMANCE:
            return str_GL_DEBUG_TYPE_PERFORMANCE;
        case GL_DEBUG_TYPE_MARKER:
            return str_GL_DEBUG_TYPE_MARKER;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return str_GL_DEBUG_TYPE_PUSH_GROUP;
        case GL_DEBUG_TYPE_POP_GROUP:
            return str_GL_DEBUG_TYPE_POP_GROUP;
        case GL_DEBUG_TYPE_OTHER:
            return str_GL_DEBUG_TYPE_OTHER;
        default:
            return str_null;
        }
    }
    const char *OpenGLError::GetSeverity(GLenum s)
    {
        switch (s)
        {
        case GL_DEBUG_SEVERITY_HIGH:
            return str_GL_DEBUG_SEVERITY_HIGH;
        case GL_DEBUG_SEVERITY_MEDIUM:
            return str_GL_DEBUG_SEVERITY_MEDIUM;
        case GL_DEBUG_SEVERITY_LOW:
            return str_GL_DEBUG_SEVERITY_LOW;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            return str_GL_DEBUG_SEVERITY_NOTIFICATION;
        default:
            return str_null;
        }
    }

    void OpenGLError::ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
    {
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
            return;
        const char *char_source = GetSourceFrom(source);
        const char *char_type = GetSourceType(type);
        const char *char_severity = GetSeverity(severity);
        ERROR("OpenGL", char_severity , ">from " , char_source , ";type " , char_type , ";id " , id , ";\n"
                                      , message);
    }
#endif //_DEBUG
} // namespace Boundless
