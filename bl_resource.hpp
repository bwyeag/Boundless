#ifndef _BOUNDLESS_RESOURCE_HPP_FILE_
#define _BOUNDLESS_RESOURCE_HPP_FILE_
#include "boundless_base.hpp"
namespace Boundless
{
///////////////////////////////////////////////
// Mesh相关
//

// 索引状态
enum struct : int IndexStatus {
    NO_INDEX = 0,
    ONLY_INDEX = 32,
    RESTART_INDEX = 33
};
// 数据范围存储结构
struct DataRange {
    size_t start, length;
};
// 标识顶点类型
// 顶点结构存储约定:
// Mesh将VBO数据绑定在0号绑定点(glVertexArrayVertexBuffer)
// 其余数据存放在>0的绑定点
// 着色器输入绑定到0号位(glVertexArrayAttribBinding)
// POSITION: 0->位置
// NORMAL:   0->位置, 1->法向量
enum struct VertexData { POSITION, NORMAL };
struct MeshFile {
    GLenum primitive_type, index_type;  // 图元类型; 索引数据类型
    IndexStatus index_status;           // 索引状态
    uint32 restart_index;               // 重启动索引
    uint32 buffer_count,
        mesh_count;  // buffer数; 网格顶点数/索引数(取决于索引状态)
    DataRange vbo, ibo;
    DataRange buffers[];
};
const size_t MESH_HEADER = 0xF241282943FF0001;       // Mesh文件头代码
const size_t MUTI_MESH_HEADER = 0xF242191756FF0003;  // 多重Mesh文件头代码
const aiPostProcessSteps assimp_load_process =
    aiProcess_Triangulate | aiProcess_FlipUVs;
const GLenum opengl_buffer_storage = GL_MAP_READ_BIT;
class Mesh {
   private:
    GLuint vertex_array, vertex_buffer, index_buffer;
    std::vector<GLuint> buffers;
    GLenum primitive_type;
    IndexStatus index_status;
    GLuint restart_index;
    GLenum index_type;
    GLsizei mesh_count;

   public:
    Mesh();                                    // 不做任何事
    Mesh(IndexStatus indexst, size_t bufcnt);  // 按index状态和buffer数初始化
    Mesh(Mesh&&) noexcept = default;
    Mesh(const Mesh&) = delete;
    Mesh& operator=(Mesh&&) noexcept = default;
    Mesh& operator=(const Mesh&) = delete;
    // Init~()方法
    void InitIndexStatus(IndexStatus indexst);  // 初始化到指定的IndexStatus
    void InitMesh(const GLsizei* stride_array,
                  const GLintptr* start_array = nullptr);

    // 获取数据方法
    const std::vector<GLuint>& getBuffer();
    void setPrimitiveType(GLenum type);
    void setRestartIndex(GLuint index);
    GLenum getIndexType();
    GLenum getPrimitiveType();
    IndexStatus getIndexStatus();
    GLuint getCount();
    GLuint getRestartIndex();
    GLuint getVAO();
    GLuint getVBO();
    GLuint getIBO();
    // Load~()方法 从文件加载Mesh(仅加载数据)
    static void LoadMesh(const Byte* data, Mesh& mesh);
    static void LoadMesh(std::ifstream& in, Mesh& mesh); // 读取下一个Mesh
    static void LoadMesh(const std::string& path, Mesh& mesh);
    static void LoadMesh(const char* path, Mesh& mesh);
    static void LoadMeshMultple(const std::string& path,
                                std::vector<Mesh>& meshs);
    static void LoadMeshMultple(const char* path, std::vector<Mesh>& meshs);
    // Pack~()方法 将Mesh打包为文件
    static Byte* PackMesh(size_t* ret_length, const Mesh& mesh);
    static void PackMesh(const std::string& path, const Mesh& mesh);
    // Gen~()方法 从外部文件格式打包为文件
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

///////////////////////////////////////////////
// Texture相关
//

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

    static void LoadTexture(
        const std::string& path,
        Texture& tex,
        GLsizei add_mipmap_level = 0,
        GLsizei samples = default_texture_samples,
        GLboolean fixedsample = default_texture_fixedsamplelocation);
    static void LoadTexture(
        const char* path,
        Texture& tex,
        GLsizei add_mipmap_level = 0,
        GLsizei samples = default_texture_samples,
        GLboolean fixedsample = default_texture_fixedsamplelocation);
    static void PackTexture(const std::string& save_path,
                            Texture& tex,
                            GLsizei level,  // 打包的纹理mipmap层级数
                            GLenum format,  // 输出的纹元格式
                            GLenum type);   // 纹元数据类型
    static Byte* PackTexture(size_t* ret_length,
                             Texture& tex,
                             GLsizei level,
                             GLenum format,
                             GLenum type);
    static void PackTexture(const char* save_path,
                            Texture& tex,
                            GLsizei level,
                            GLenum format,
                            GLenum type);
    static void GenTextureFile(const std::string& path);
    static void GenTextureFile(const char* path);
};
} // namespace Boundless

#endif //!_BOUNDLESS_RESOURCE_HPP_FILE_