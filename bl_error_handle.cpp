#include "error_handle.hpp"

namespace Boundless::Log
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
} // namespace Boundless
