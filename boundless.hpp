#ifndef _BOUNDLESS_HPP_FILE_
#define _BOUNDLESS_HPP_FILE_

#include "glad/glad.h"
#include "glfw/glfw3.h"

#include "bl_log.hpp"

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <unordered_map>
#include <vector>

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
    friend void LoadMesh(const std::string& path, Mesh& mesh);
    friend void LoadMesh(const char* path, Mesh& mesh);
    friend void LoadMeshMultple(const std::string& path,
                                std::vector<Mesh>& meshs);
    friend void LoadMeshMultple(const char* path, std::vector<Mesh>& meshs);
    friend Byte* PackMesh(size_t* ret_length, const Mesh& mesh);
    friend void PackMesh(const std::string& path, const Mesh& mesh);
    static void GenMeshFile(const aiMesh* ptr, const std::string& save_path);
    static Byte* GenMeshFile(const aiMesh* ptr,
                             const std::string& name,
                             size_t* ret_length);
    static void GenMeshFile(const std::string& path);
    static void GenMeshFile(const char* path);
    static void GenMeshFileMerged(const std::string& path);
    static void GenMeshFileMerged(const char* path);
    ~Mesh();
};

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
    GLenum type;
    GLenum format;
    GLsizei width, height, depth;

   public:
    Texture();
    ~Texture();

    friend void LoadTexture(const std::string& path,
                            Texture& tex,
                            GLsizei add_mipmap_level = 0,
                            GLsizei samples = default_texture_samples,
                            GLboolean fixedsample = default_texture_fixedsamplelocation);
    static void GenTextureFile(const std::string& path);
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
    mutable std::unordered_map<std::string, GLint> uniformmap;

    void PrintLog() const;
    GLint GetUniformLocation(const std::string& target) const;
    GLint GetUniformBlockLocation(const std::string& target) const;

   public:
    Program();
    Program(const Program& target) = delete;
    Program(Program&& target) noexcept = default;
    Program& operator=(const Program& target) = delete;
    Program& operator=(Program&& target) noexcept = default;
    Shader& operator[](size_t index);
    ~Program();

    void AddShader(const std::string& path, GLenum type);
    void Link() const;

    void Use() const;
    static void UnUse() const;

    static GLuint LoadShader(const std::string& path, GLenum type);
    static GLuint LoadShader(const char* path, GLenum type);

    void SetTexture(const std::string& name, GLint unit);
    void SetBool(const std::string& name, GLboolean value);
    void SetInt(const std::string& name, GLint value);
    void SetUint(const std::string& name, GLuint value);
    void SetFloat(const std::string& name, GLfloat value);
    void SetVec2(const std::string& name, const glm::vec2& value);
    void SetVec2(const std::string& name, GLfloat x, GLfloat y);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetVec3(const std::string& name, GLfloat x, GLfloat y, GLfloat z);
    void SetVec4(const std::string& name, const glm::vec4& value);
    void SetVec4(const std::string& name,
                 GLfloat x,
                 GLfloat y,
                 GLfloat z,
                 GLfloat w);
    void SetVec2Array(const std::string& name,
                      GLsizei count,
                      const glm::vec2* value);
    void SetVec3Array(const std::string& name,
                      GLsizei count,
                      const glm::vec3* value);
    void SetVec4Array(const std::string& name,
                      GLsizei count,
                      const glm::vec4* value);
    void SetMat2(const std::string& name, const glm::mat2& mat);
    void SetMat3(const std::string& name, const glm::mat3& mat);
    void SetMat4(const std::string& name, const glm::mat4& mat);
    void SetMat2Array(const std::string& name,
                      GLsizei count,
                      const glm::mat2* mat);
    void SetMat3Array(const std::string& name,
                      GLsizei count,
                      const glm::mat3* mat);
    void SetMat4Array(const std::string& name,
                      GLsizei count,
                      const glm::mat4* mat);
    void SetDouble(const std::string& name, GLdouble value);
    void SetVec2(const std::string& name, const glm::dvec2& value);
    void SetVec2Array(const std::string& name,
                      GLsizei count,
                      const glm::dvec2* value);
    void SetVec2(const std::string& name, GLdouble x, GLdouble y);
    void SetVec3(const std::string& name, const glm::dvec3& value);
    void SetVec3Array(const std::string& name,
                      GLsizei count,
                      const glm::dvec3* value);
    void SetVec3(const std::string& name, GLdouble x, GLdouble y, GLdouble z);
    void SetVec4(const std::string& name, const glm::dvec4& value);
    void SetVec4Array(const std::string& name,
                      GLsizei count,
                      const glm::dvec4* value);
    void SetVec4(const std::string& name,
                 GLdouble x,
                 GLdouble y,
                 GLdouble z,
                 GLdouble w);
    void SetMat2(const std::string& name, const glm::dmat2& mat);
    void SetMat3(const std::string& name, const glm::dmat3& mat);
    void SetMat4(const std::string& name, const glm::dmat4& mat);
    void SetMat2Array(const std::string& name,
                      GLsizei count,
                      const glm::dmat2* mat);
    void SetMat3Array(const std::string& name,
                      GLsizei count,
                      const glm::dmat3* mat);
    void SetMat4Array(const std::string& name,
                      GLsizei count,
                      const glm::dmat4* mat);
    void SetVec2(const std::string& name, const glm::ivec2& value);
    void SetVec2(const std::string& name, GLint x, GLint y);
    void SetVec3(const std::string& name, const glm::ivec3& value);
    void SetVec3(const std::string& name, GLint x, GLint y, GLint z);
    void SetVec4(const std::string& name, const glm::ivec4& value);
    void SetVec4(const std::string& name, GLint x, GLint y, GLint z, GLint w);
    void SetVec2(const std::string& name, const glm::uvec2& value);
    void SetVec2(const std::string& name, GLuint x, GLuint y);
    void SetVec3(const std::string& name, const glm::uvec3& value);
    void SetVec3(const std::string& name, GLuint x, GLuint y, GLuint z);
    void SetVec4(const std::string& name, const glm::uvec4& value);
    void SetVec4(const std::string& name,
                 GLuint x,
                 GLuint y,
                 GLuint z,
                 GLuint w);
};

}  // namespace Boundless
#endif  //!_BOUNDLESS_HPP_FILE_