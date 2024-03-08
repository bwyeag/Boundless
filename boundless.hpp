#ifndef _BOUNDLESS_HPP_FILE_
#define _BOUNDLESS_HPP_FILE_

#include "glad/glad.h"
#include "glfw/glfw3.h"

#include "bl_log.hpp"

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Eigen/Core"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

namespace zlib {
#include <zlib.h>
class ZlibException : public std::exception {
   private:
    int code;

   public:
    ZlibException(int code) const char* what() const noexcept {
        if (code == Z_MEM_ERROR) {
            return "zlib memory error";
        } else if (code == Z_BUF_ERROR) {
            return "zlib buffer out off range";
        } else if (code == Z_DATA_ERROR) {
            return "zlib data corrupted";
        } else {
            return "unknow";
        }
    }
};
}  // namespace zlib

namespace Boundless {
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
Byte* CompressData(const Byte* data, size_t* length, size_t space = 0ULL);
// 解压缩数据,data为压缩后数据指针,ret_length返回数据长度
Byte* UncompressData(const Byte* data, size_t* ret_length);

enum struct IndexStatus { NO_INDEX = 0, ONLY_INDEX = 32, RESTART_INDEX = 33 };
struct DataRange {
    size_t start, length;
};

struct MeshFile {
    GLenum primitive_type;
    IndexStatus index_status;
    GLuint restart_index;
    uint32 buffer_count;
    DataRange vbo, ibo;
    DataRange buffers[];
};
const size_t MESH_HEADER = 0xF241282943FF0001;
const size_t MUTI_MESH_HEADER = 0xF242191756FF0003;
const aiPostProcessSteps assimp_load_process =
    aiProcess_Triangulate | aiProcess_FlipUVs;
class Mesh {
   private:
    GLuint vertex_array, vertex_buffer, index_buffer;
    std::vector<GLuint> buffers, textures;
    GLenum primitive_type;
    IndexStatus index_status;
    GLuint restart_index;

   public:
    Mesh();
    Mesh(IndexStatus indexst, size_t bufcnt);
    Mesh(Mesh&&) noexcept = default;
    Mesh(const Mesh&) = delete;
    Mesh& operator=(Mesh&&) noexcept = default;
    Mesh& operator=(const Mesh&) = delete;
    const std::vector<GLuint>& GetBuffer();
    const std::vector<GLuint>& GetTexture();
    void SetPrimitiveType(GLenum type);
    void SetRestartIndex(GLuint index);
    GLuint GetVAO();
    GLuint GetVBO();
    GLuint GetIBO();
    friend void LoadMesh(const Byte* data, size_t length, Mesh& mesh);
    friend void LoadMesh(std::ifstream& in, Mesh& mesh);
    friend void LoadMesh(std::string_view path, Mesh& mesh);
    friend void LoadMesh(const char* path, Mesh& mesh);
    friend void LoadMeshMultple(std::string_view path,
                                std::vector<Mesh>& meshs);
    friend void LoadMeshMultple(const char* path, std::vector<Mesh>& meshs);
    friend Byte* PackMesh(size_t* ret_length, const Mesh& mesh);
    friend void PackMesh(std::string_view path, const Mesh& mesh);
    static void GenMeshFile(const aiMesh* ptr, std::string_view save_path);
    static Byte* GenMeshFile(const aiMesh* ptr,
                             std::string_view name,
                             size_t* ret_length);
    static void GenMeshFile(std::string_view path);
    static void GenMeshFile(const char* path);
    static void GenMeshFileMerged(std::string_view path);
    static void GenMeshFileMerged(const char* path);
    ~Mesh();
};
constexpr size_t TextureInternalFormatSize(GLenum type);
constexpr size_t TypeSize(GLenum type);
struct TextureMipData {
    // 纹理该Mipmap层级的长宽高
    GLsizei width, height, depth;
    // 纹理该Mipmap层级的数据区域，range.start为起始位置，range.length为单个纹理切片的长度
    DataRange range;
};

template <size_t mip_max>
struct TextureFile {
    GLenum target;
    GLenum internal_format;
    GLenum format, type, swizzle[4];
    GLboolean enable_swizzle;
    GLsizei mipLevels, slices;
    size_t totalSize;
    TextureMipData mip[mip_max];
    TextureFile() { static_assert(mip_max < 24); }
};
struct TextureFileN {
    GLenum target;
    GLenum internal_format;
    GLenum format, type, swizzle[4];
    GLboolean enable_swizzle;
    GLsizei mipLevels, slices;
    size_t totalSize;
    TextureMipData mip[];
};
const size_t TEXTURE_HEADER = 0xF24241339FFF0002;
const GLsizei default_texture_samples = 4;
const GLboolean default_texture_fixedsamplelocation = GL_FALSE;
class Texture {
   private:
    GLuint texture_id;
    GLenum target;
    GLsizei width, height, depth;

   public:
    // 构造函数，不做任何事，使用LoadTexture()函数加载
    Texture();
    ~Texture();

    friend void LoadTexture(
        std::string_view path,
        Texture& tex,
        GLsizei add_mipmap_level = 0,
        GLsizei samples = default_texture_samples,
        GLboolean fixedsample = default_texture_fixedsamplelocation);
    friend void LoadTexture(
        const char* path,
        Texture& tex,
        GLsizei add_mipmap_level = 0,
        GLsizei samples = default_texture_samples,
        GLboolean fixedsample = default_texture_fixedsamplelocation);
    friend void PackTexture(std::string_view save_path,
                            Texture& tex,
                            GLsizei level,  // 打包的纹理mipmap层级数
                            GLenum format,  // 输出的纹元格式
                            GLenum type);   // 纹元数据类型
    friend Byte* PackTexture(size_t* ret_length,
                             Texture& tex,
                             GLsizei level,
                             GLenum format,
                             GLenum type);
    friend void PackTexture(const char* save_path,
                            Texture& tex,
                            GLsizei level,
                            GLenum format,
                            GLenum type);
    static void GenTextureFile(std::string_view path);
    static void GenTextureFile(const char* path);
};

class Program {
   public:
    struct ShaderInfo {
        GLenum type;
        GLuint id;
    };
   private:
    GLuint program_id;
    std::vector<ShaderInfo> program_shader;
    void PrintLog() const;
   public:
    Program();
    Program(const Program& target) = delete;
    Program(Program&& target) noexcept = default;
    Program& operator=(const Program& target) = delete;
    Program& operator=(Program&& target) noexcept = default;
    Shader& operator[](size_t index);
    ~Program();

    void AddShader(std::string_view path, GLenum type);
    void Link() const;

    static void UseProgram(Program& p);
    static void UnUseProgram();

    static GLuint LoadShader(std::string_view path, GLenum type);
    static GLuint LoadShader(const char* path, GLenum type);
};
class transform {
public:
    vec3 position;
};

class RenderObject {
   private:
    bool enable;
    Mesh mesh;
    transform trans;
   public:
    RenderObject();
    ~RenderObject()
    {
    }
};

}  // namespace Boundless
#endif  //!_BOUNDLESS_HPP_FILE_