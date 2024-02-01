#ifndef _RESOURCE_LOAD_HPP_
#define _RESOURCE_LOAD_HPP_

#include "glad/glad.h"

#include <cstdint>
#include <cstddef>

#include <utility>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

#define ERROR(type, info) std::cerr << "[ERROR][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl
#define ERRORINFO(info) std::cerr << "[ERROR]other:" << info << std::endl
#define WARNING(type, info) std::cerr << "[WARNING][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl
#define WARNINGINFO(info) std::cerr << "[WARNING]other:" << info << std::endl
#define INFO(type, info) std::cout << "[INFO][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl
#define INFODATA(info) std::cout << "[INFO]other:" << info << std::endl

namespace Boundless::Resource
{
    using std::string;
    using std::fstream;
    using std::ifstream;
    using std::ofstream;
    using std::ios;
    using std::vector;
/*
*   模型文件存储规范 2024/1/28
*   以.modelfile为后缀名
*   (以下按文件顺序)(开头)
*   8字节存储标志符: 0xF241282943FF0001
*   模型处理符号(8Byte)(用于指定使用的函数,0为默认函数)
*   顶点数据开头,顶点数据长度(各8Byte)
*   索引数据开头(如果没有为UINT64_MAX),索引数据结尾(如果没有为0)(各8Byte)
*   重启动索引(8Byte)(如果没有为UINT64_MAX)
*   缓存数目(4Byte)
*   缓存数目个{数据开头,数据长度(各8Byte)}结构体
*/
#define MODEL_HEADER 0xF241282943FF0001
    typedef void (*ModelLoadFunction)(ModelLoader*);
    static std::unordered_map<uint64_t,ModelLoadFunction> load_functions;

    struct ModelHead
    {
        uint64_t signal;
        uint64_t vertex_start,vertex_length;
        uint64_t index_start,index_length,ignore_index;
        uint32_t buffer_count;
    };
    struct BufferInfoData
    {
        uint64_t start,length;
    };
    class ModelLoader
    {
    public:
        static void AddLoadFunction(uint64_t code,ModelLoadFunction f)
        {
        #ifdef _DEBUG
            if (code==0UL)
            {
                WARNING("Resource","AddLoadFunction():code不能为0");
                return;
            }
        #endif
            load_functions.emplace(code,f);
        }
    private:
        ifstream model_file;
        ModelHead head_data;
        vector<GLuint> buffers;
        bool has_index;
    public:
        ModelLoader(const char* path);
        ModelLoader(string path);
        ModelLoader(const ModelLoader&)=delete;
        ModelLoader(ModelLoader&&)=default;
        ModelLoader& operator=(const ModelLoader&)=delete;
        ModelLoader& operator=(ModelLoader&&)=default;
    };
} // namespace Boundless::Resource

#endif //!_RESOURCE_LOAD_HPP_