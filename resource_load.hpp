#ifndef _RESOURCE_LOAD_HPP_
#define _RESOURCE_LOAD_HPP_

#include "glad/glad.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "zlib/zlib.h"

#include <cstdint>
#include <cstddef>
#include <utility>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>


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
*   模型文件存储规范 2024/2/4
*   以.modelfile为后缀名
*   (以下按文件顺序)(开头)
*   8字节存储标志符: 0xF241282943FF0001
*   8字节压缩文件原本长度
*   (以下为余下部分解压缩后的内容)
*   开头位置以解压缩后的开头起始
*   顶点数据开头,顶点数据长度(各8Byte)
*   索引数据开头(如果没有为UINT64_MAX),索引数据结尾(如果没有为0)(各8Byte)
*   重启动索引(8Byte)(如果没有为UINT64_MAX-1)
*   缓存数目(4Byte)
*   缓存数目个{数据开头,数据长度(各8Byte)}结构体
*/
#define MODEL_HEADER 0xF241282943FF0001
    struct ModelHead
    {
        uint64_t vertex_start,vertex_length;
        uint64_t index_start,index_length,restart_index;
        uint32_t buffer_count;
    };
    struct BufferInfoData
    {
        uint64_t start,length;
    };

    typedef void (*ModelLoadFunction)(void*, ModelHead*, BufferInfoData*);

    class ModelLoader
    {
    private:
        ifstream model_file;
        vector<GLuint> buffers;
        uint64_t restart_index;
        bool has_index, has_other_buffers;
    public:

        ModelLoader(const char* path, ModelLoadFunction f);
        inline ModelLoader(string path, ModelLoadFunction f)
            : ModelLoader(path.c_str(), f) {}
        ModelLoader(const ModelLoader&)=delete;
        ModelLoader(ModelLoader&&)=default;
        ModelLoader& operator=(const ModelLoader&)=delete;
        ModelLoader& operator=(ModelLoader&&)=default;

        void LoadFile(const char* path, ModelLoadFunction f);
        inline bool HasIndex() const {return has_index;};
    };

#define TEXTURE_HEADER 0xF24241339FFF0002

    struct TextureHead
    {
        GLenum gl_target;
        GLenum gl_internal_format;
        GLenum gl_format,gl_type;
        GLsizei mipmap_level;
        GLsizei width,height,depth;
    };
    typedef void (*TextureLoadFunction)(TextureLoader*, GLuint, TextureHead*);
    class TextureLoader
    {
    private:
        ifstream texture_file;
        GLuint texture;
        TextureHead texture_info;
    public:
        TextureLoader(const char* path, TextureLoadFunction f);
        inline TextureLoader(string path, TextureLoadFunction f)
            : ModelLoader(path.c_str(), f) {}
        TextureLoader(const TextureLoader&)=delete;
        TextureLoader(TextureLoader&&)=default;
        TextureLoader& operator=(const TextureLoader&)=delete;
        TextureLoader& operator=(TextureLoader&&)=default;

        inline GLuint GetTexture() {return texture;}
        void LoadFile(const char* path, TextureLoadFunction f);
    };
    
    void GenerateInitialize();
    void GenerateModelFile(const char* path);
    void GenerateTextureFile2D(const char* path);

} // namespace Boundless::Resource

#endif //!_RESOURCE_LOAD_HPP_