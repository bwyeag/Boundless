#ifndef _BOUNDLESS_HPP_FILE_
#define _BOUNDLESS_HPP_FILE_

#include "glad/glad.h"
#include "glfw/glfw3.h"

#include "bl_log.hpp"

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <vector>
#include <fstream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

namespace zlib
{
    #include <zlib.h>
    class ZlibException : public std::exception
    {
    private:
        int code;
    public:
        ZlibException(int code)
        const char * what () const noexcept
        {
            if (code == Z_MEM_ERROR)
            {
                return "zlib memory error";
            }
            else if (code == Z_BUF_ERROR)
            {
                return "zlib buffer out off range";
            }
            else if (code == Z_DATA_ERROR)
            {
                return "zlib data corrupted";
            }
            else
            {
                return "unknow";
            }
        }

    };
} // namespace zlib


namespace Boundless
{
    typedef std::int8_t int8;
    typedef std::int16_t int16;
    typedef std::int32_t int32;
    typedef std::int64_t int6;
    typedef std::uint8_t Byte;
    typedef std::uint16_t uint16;
    typedef std::uint32_t uint32;
    typedef std::uint64_t uint64;

    const int32 compress_level = 7;
    /*数据结构:|压缩前长度8Byte|压缩后长度8Byte|压缩数据|*/
    // 压缩数据,data为压缩前数据指针,length为数据长度,结束后变为压缩后长度,space在开头预留空间
    Byte *CompressData(const Byte *data, size_t *length, size_t space = 0ULL);
    // 解压缩数据,data为压缩后数据指针,ret_length返回数据长度
    Byte *UncompressData(const Byte *data, size_t *ret_length);

    enum struct IndexStatus {
        NO_INDEX=0,ONLY_INDEX=32,RESTART_INDEX=33
    };
    struct DataRange
    {
        size_t start,length;
    };
    
    struct MeshFile
    {
        GLenum primitive_type;IndexStatus index_status;
        GLuint restart_index;uint32 buffer_count;
        DataRange vbo,ibo;
        DataRange buffers[];
    };
#define MESH_HEADER 0xF241282943FF0001
#define TEXTURE_HEADER 0xF24241339FFF0002
#define MUTI_MESH_HEADER 0xF242191756FF0003
    const aiPostProcessSteps assimp_load_process = aiProcess_Triangulate | aiProcess_FlipUVs;
    class Mesh
    {
    private:
        GLuint vertex_array,vertex_buffer,index_buffer;
        std::vector<GLuint> buffers,textures;
        GLenum primitive_type;IndexStatus index_status;
        GLuint restart_index;
    public:
        Mesh();
        Mesh(IndexStatus indexst,size_t bufcnt);
        Mesh(Mesh&&) = default;
        Mesh(const Mesh&) = delete;
        Mesh& operator=(Mesh&&) = default;
        Mesh& operator=(const Mesh&) = delete;
        const std::vector<GLuint>& GetBuffer();
        const std::vector<GLuint>& GetTexture();
        void SetPrimitiveType(GLenum type);
        void SetRestartIndex(GLuint index);
        GLuint GetVAO();
        GLuint GetVBO();
        GLuint GetIBO();
        friend void LoadMesh(const Byte* data,size_t length, Mesh& mesh);
        friend void LoadMesh(std::ifstream& in, Mesh& mesh);
        friend void LoadMesh(const std::string& path, Mesh& mesh);
        friend void LoadMesh(const char* path, Mesh& mesh);
        friend void LoadMeshMultple(const std::string &path, std::vector<Mesh>& meshs);
        friend void LoadMeshMultple(const char* path, std::vector<Mesh>& meshs);
        friend Byte* PackMesh(size_t *ret_length,const Mesh& mesh);
        friend void PackMesh(const std::string& path,const Mesh& mesh);
        static void GenMeshFile(const aiMesh* ptr, const std::string &save_path);
        static Byte* GenMeshFile(const aiMesh* ptr,const std::string &name, size_t* ret_length);
        static void GenMeshFile(const string &path);
        static void GenMeshFile(const char*path);
        static void GenMeshFileMerged(const string &path);
        static void GenMeshFileMerged(const char*path);
        ~Mesh();
    };
} // namespace Boundless

#endif //!_BOUNDLESS_HPP_FILE_