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
}
#endif //!_ERROR_HANDLE_HPP_FILE_