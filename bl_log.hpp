#ifndef _BOUNDLESS_LOG_HPP_FILE_
#define _BOUNDLESS_LOG_HPP_FILE_

#include "glad/glad.h"
#include <cstdlib>
#include <cassert>
#include <iostream>

namespace Boundless
{
    #define ERROR(type, ...) Boundless::PrintError(type, __FILE__, __LINE__, ##__VA_ARGS__)
    #define WARNING(type, ...) Boundless::PrintWarning(type, __FILE__, __LINE__, ##__VA_ARGS__)
    #define INFO(type, ...) Boundless::PrintInfomation(type, __FILE__, __LINE__, ##__VA_ARGS__)
    template <typename... args>
    void PrintError(const char *type, const char *file, int line, const args &...arguments)
    {
        std::cerr << "[ERROR][" << type << "]file:" << file << ";line:" << line << "|info:";
        (std::cerr << ... << arguments) << std::endl;
        exit(-1);
    }
    template <typename... args>
    void PrintWarning(const char *type, const char *file, int line, const args &...arguments)
    {
        std::cerr << "[WARNING][" << type << "]file:" << file << ";line:" << line << "|info:";
        (std::cerr << ... << arguments) << std::endl;
    }
    template <typename... args>
    void PrintInfomation(const char *type, const char *file, int line, const args &...arguments)
    {
        std::cout << "[INFOMATION][" << type << "]file:" << file << ";line:" << line << "|info:";
        (std::cout << ... << arguments) << std::endl;
    }
#ifdef _DEBUG
    namespace OpenGLError
    {
        static const char *GetObjectType(GLenum o);
        static const char *GetSourceFrom(GLenum s);
        static const char *GetSourceType(GLenum s);
        static const char *GetSeverity(GLenum s);
        static void ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
    }
#endif //_DEBUG
}
#endif //!_BOUNDLESS_LOG_HPP_FILE_