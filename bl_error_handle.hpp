#ifndef _ERROR_HANDLE_HPP_FILE_
#define _ERROR_HANDLE_HPP_FILE_

#include <cstdlib>
#include <cassert>
#include <iostream>

#define ERROR(type, info) std::cerr << "[ERROR][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl
#define ERRORINFO(info) std::cerr << "[ERROR]other:" << info << std::endl
#define WARNING(type, info) std::cerr << "[WARNING][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl
#define WARNINGINFO(info) std::cerr << "[WARNING]other:" << info << std::endl
#define INFO(type, info) std::cout << "[INFO][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl
#define INFODATA(info) std::cout << "[INFO]other:" << info << std::endl

namespace Boundless::Log
{
    const char* const RES_ERROR = "Resource";
    const char* const RENDER_ERROR = "Render";
    const char* const MEMORY_ERROR = "Memory";

    const char* const LIB_ZLIB_ERROR = "ZLIB";
    const char* const LIB_STB_IMAGE_ERROR = "STB_IMAGE";
    const char* const LIB_ASSMIP_ERROR = "ASSIMP";
    /*外部库错误文本*/
    const char* const LIB_ZLIB_UNCOMPRESS_ERROR = "在解压时出现错误:";
    const char* const LIB_ZLIB_COMPRESS_ERROR = "在压缩时出现错误:";

    const char* const LIB_STB_IMAGE_LOAD_ERROR = "在打开图像时出现错误:";

    /*渲染错误文本*/
    const char* const RENDER_INDEX_STATUS_ERROR = "索引状态错误";
    /*通用错误文本*/
    const char* const OUT_OF_MEMORY_ERROR = "内存耗尽";
    /*资源错误文本*/
    const char* const RES_FILE_LOAD_ERROR = "无法打开文件:";
    const char* const RES_FILE_TYPE_ERROR = "文件类型错误,文件头代码:";
    const char* const RES_TEXTURE_TYPE_ERROR = "纹理文件格式错误:";
    const char* const RES_TEXTURE_CANNEL_ERROR = "图像通道数错误:";
    const char* const RES_CANNOT_OPEN_FILE_ERROR = "无法打开文件:";
    const char* const RES_CANNOT_CREATE_FILE_ERROR = "无法创建文件:";

    inline void error_handle()
    {
        exit(-1);
    }

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
}
#endif //!_ERROR_HANDLE_HPP_FILE_