#ifndef _RESOURCE_LOAD_HPP_
#define _RESOURCE_LOAD_HPP_

#include "glad/glad.h"
#include "zlib/zlib.h"
#ifdef BOUNDLESS_GENERATE_FUNCTIONS
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#endif //BOUNDLESS_GENERATE_FUNCTIONS

#include <cstdint>
#include <cstddef>
#include <utility>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>

#include "error_handle.hpp"

namespace Boundless::Resource
{
    using std::string;
    using std::fstream;
    using std::ifstream;
    using std::ofstream;
    using std::cout;
    using std::cin;
    using std::endl;
    using std::ios;
    using std::vector;
    using std::stack;
/*
*   模型文件存储规范 2024/2/6
*   以.mesh为后缀名
*   (以下按文件顺序)(开头)
*   8字节存储标志符: 0xF241282943FF0001
*   8字节压缩文件原本长度
*   (以下为余下部分解压缩后的内容)
*   开头位置以解压缩后的开头起始
*   顶点数据开头,顶点数据长度(各8Byte)
*   索引数据开头(如果没有为UINT64_MAX),索引数据结尾(如果没有为0)(各8Byte)
*   重启动索引(8Byte)(如果没有为UINT64_MAX)
*   缓存数目(4Byte)
*   缓存数目个{数据开头,数据长度(各8Byte)}结构体
*/
#define MODEL_HEADER 0xF241282943FF0001
    struct MeshHead
    {
        uint64_t vertex_start,vertex_length;
        uint64_t index_start,index_length,restart_index;
        uint32_t buffer_count;
    };
    struct BufferInfoData
    {
        uint64_t start,length;
    };

    typedef void (*MeshLoadFunction)(void*, MeshHead*, BufferInfoData*);

    enum struct IndexStatus : uint8_t
    {
        NONE,INDEX_WITH_RESTARTINDEX,ONLY_INDEX
    }
    class MeshLoader
    {
    private:
        ifstream model_file;
        vector<GLuint> buffers;
        uint64_t restart_index;
        IndexStatus index_status;
        bool has_other_buffers;
    public:

        MeshLoader(const char* path, MeshLoadFunction f);
        inline MeshLoader(string path, MeshLoadFunction f)
            : MeshLoader(path.c_str(), f) {}
        MeshLoader(const MeshLoader&)=delete;
        MeshLoader(MeshLoader&&)=default;
        MeshLoader& operator=(const MeshLoader&)=delete;
        MeshLoader& operator=(MeshLoader&&)=default;

        void LoadFile(const char* path, MeshLoadFunction f);
        inline IndexStatus GetIndexStatus() const {return has_index;};
        inline bool HasOtherBuffers() const {return has_other_buffers;};
        inline const vector<GLuint>& GetBuffers() const {return buffers};
        inline uint64_t GetRestartIndex() const {return restart_index;};
    };

#define TEXTURE_HEADER 0xF24241339FFF0002
#define COMPRESS_LEVEL 6
    struct TextureHead
    {
        GLenum gl_target;
        GLenum gl_internal_format;
        GLenum gl_format,gl_type;
        GLsizei mipmap_level;
        GLsizei width,height,depth;
    };
    typedef void (*TextureLoadFunction)(GLuint,TextureHead*);  
    class Texture2DLoader
    {
    private:
        ifstream texture_file;
        GLuint texture;
        TextureHead texture_info;
    public:
        Texture2DLoader(const char* path, TextureLoadFunction f);
        inline Texture2DLoader(string path, TextureLoadFunction f)
            : Texture2DLoader(path.c_str(), f) {}
        Texture2DLoader(const Texture2DLoader&)=delete;
        Texture2DLoader(Texture2DLoader&&)=default;
        Texture2DLoader& operator=(const Texture2DLoader&)=delete;
        Texture2DLoader& operator=(Texture2DLoader&&)=default;

        inline GLuint GetTexture() {return texture;}
        void LoadFile(const char* path, TextureLoadFunction f);
    };
    class FileGeneraters
    {
    public:
        static void GenInitialize();
        static void GenModelFile(const char* path);
        static void GenMeshFile(const aiMesh* ptr, const string &path);
        static void GenTextureFile2D(const char* path);
    };
    

} // namespace Boundless::Resource

#endif //!_RESOURCE_LOAD_HPP_