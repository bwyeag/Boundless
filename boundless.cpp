#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "boundless.hpp"

namespace Boundless {
Byte* CompressData(const Byte* data, size_t* length, size_t space) {
    size_t dlen = zlib::compressBound(*length) + sizeof(uint64) * 2;
    Byte* compress_data = (Byte*)malloc(space + dlen);
    if (compress_data == nullptr) {
        throw std::bad_alloc();
    }
    Byte* data_ptr = compress_data + space;
    *(uint64*)data_ptr = *length;
    data_ptr += sizeof(uint64) * 2;
    *(uint64*)(data_ptr - sizeof(uint64)) = dlen - sizeof(uint64) * 2;
    int res = zlib::compress2(data_ptr, data_ptr - sizeof(uint64), data,
                              *length, compress_level);
    if (res != Z_OK) {
        free(compress_data);
        throw zlib::ZlibException(res);
    }
    *length =
        sizeof(uint64) * 2 + space + *(uint64*)(data_ptr - sizeof(uint64));
    return compress_data;
}
Byte* UncompressData(const Byte* data, size_t* ret_length) {
    Byte* uncompress_data = (Byte*)malloc(*(uint64*)data);
    if (uncompress_data == nullptr) {
        throw std::bad_alloc();
    }
    *ret_length = *(uint64*)data;
    int res = zlib::uncompress2(uncompress_data, ret_length,
                                data + sizeof(uint64) * 2,
                                *(uint64*)(data + sizeof(uint64)));
    if (res != Z_OK) {
        free(uncompress_data);
        throw zlib::ZlibException(res);
    }
    return uncompress_data;
}

Mesh::Mesh() {}
Mesh::Mesh(IndexStatus indexst, size_t bufcnt) {
    glCreateVertexArrays(1, &vertex_array);
    glCreateBuffers(1, &vertex_buffer);
    buffers.resize(bufcnt);
    glCreateBuffers(bufcnt, &buffers[0]);
    index_status = indexst;
    if (indexst > 31) {
        glCreateBuffers(1, &index_buffer);
    }
}
inline const std::vector<GLuint>& Mesh::GetBuffer() {
    return buffers;
}
inline const std::vector<GLuint>& Mesh::GetTexture() {
    return textures;
}
inline void Mesh::SetPrimitiveType(GLenum type) {
    primitive_type = type;
}
inline void Mesh::SetRestartIndex(GLuint index) {
    restart_index = index;
}
inline GLuint Mesh::GetVAO() {
    return vertex_array;
}
inline GLuint Mesh::GetVBO() {
    return vertex_buffer;
}
inline GLuint Mesh::GetIBO() {
    return index_buffer;
}
void LoadMesh(const Byte* data, size_t length, Mesh& mesh) {
    if (*(uint64*)data != 0xF241282943FF0001) {
        throw std::runtime_error("Mesh head code error.");
    }
    size_t len;
    data = UncompressData(data + sizeof(uint64), &len);
    MeshFile& head = *static_cast<MeshFile*>(data);
    Byte* dp = data + sizeof(MeshFile) + sizeof(DataRange) * head.buffer_count;
    mesh.primitive_type = head.primitive_type;
    mesh.index_status = head.index_status;
    mesh.restart_index = head.restart_index;
    glCreateVertexArrays(1, &mesh.vertex_array);
    glCreateBuffers(1, &mesh.vertex_buffer);
    mesh.buffers.resize(head.buffer_count);
    glCreateBuffers(head.buffer_count, &mesh.buffers[0]);
    glNamedBufferStorage(mesh.vertex_buffer, head.vbo.length,
                         data + head.vbo.start, GL_MAP_READ_BIT);
    if (mesh.index_status > 31) {
        glCreateBuffers(1, &mesh.index_buffer);
        glNamedBufferStorage(mesh.index_buffer, head.ibo.length,
                             data + head.ibo.start, GL_MAP_READ_BIT);
    }
    for (size_t i = 0; i < head.buffer_count; i++) {
        glNamedBufferStorage(mesh.buffers[i], head.buffers[i].length,
                             data + head.buffers[i].start, GL_MAP_READ_BIT);
    }
    free(data);
}
void LoadMesh(std::ifstream& in, Mesh& mesh) {
    size_t length;
    std::streampos cur = in.tellg();
    in.seekg(sizeof(uint64), std::ios::cur);
    in.read((char*)&length, sizeof(uint64));
    in.seekg(cur);
    Byte* data = (Byte*)malloc(length);
    if (data == nullptr) {
        throw std::bad_alloc();
    }
    in.read((char*)data, length);
    LoadMesh(data, length, mesh);
    free(data);
}
inline void LoadMesh(const std::string& path, Mesh& mesh) {
    std::ifstream fin(path, std::ios::in | std::ios::binary);
    if (!fin.is_open()) {
        throw std::runtime_error("Cannot open file:" + path);
    }
    LoadMesh(fin, mesh);
    fin.close();
}
inline void LoadMesh(const char* path, Mesh& mesh) {
    LoadMesh(std::string(path), mesh);
}
void LoadMeshMultple(const std::string& path, std::vector<Mesh>& meshs) {
    std::ifstream fin(path, std::ios::in | std::ios::binary);
    if (!fin.is_open()) {
        throw std::runtime_error("Cannot open file:" + path);
    }
    uint64 headcode;
    fin.read((char*)&headcode, sizeof(uint64));
    if (headcode != 0xF242191756FF0003) {
        throw std::runtime_error("Mesh head code error.");
    }
    uint32 mesh_count;
    fin.read((char*)&mesh_count, sizeof(uint32));
    meshs.reserve(mesh_count);
    for (Mesh& m : meshs) {
        LoadMesh(fin, m);
    }
    fin.close();
}
void LoadMeshMultple(const char* path, std::vector<Mesh>& meshs) {
    LoadMeshMultple(std::string(path), meshs);
}
Byte* PackMesh(size_t* ret_length, const Mesh& mesh) {
    size_t full_size =
        sizeof(MeshFile) + sizeof(DataRange) * mesh.buffers.size();
    GLint64 length;
    glGetNamedBufferParameteri64v(mesh.vertex_buffer, GL_BUFFER_SIZE, &length);
    full_size += static_cast<size_t>(length);
    if (mesh.index_status > 31) {
        glGetNamedBufferParameteri64v(mesh.index_buffer, GL_BUFFER_SIZE,
                                      &length);
        full_size += static_cast<size_t>(length);
    }
    for (size_t i = 0; i < mesh.buffers.size(); i++) {
        glGetNamedBufferParameteri64v(mesh.buffers[i], GL_BUFFER_SIZE, &length);
        full_size += static_cast<size_t>(length);
    }
    Byte *data = (Byte*)malloc(full_size), *cur = data;
    if (data == nullptr) {
        throw std::bad_alloc();
    }
    MeshFile& head = *(MeshFile*)cur;
    cur += sizeof(MeshFile) + sizeof(DataRange) * mesh.buffers;
    head.primitive_type = mesh.primitive_type;
    head.index_status = mesh.index_status;
    head.restart_index = mesh.restart_index;
    head.buffer_count = mesh.buffers.size();

    Byte* ptr;
    glGetNamedBufferParameteri64v(mesh.vertex_buffer, GL_BUFFER_SIZE,
                                  &head.vbo.length);
    head.vbo.start = cur - data;
    cur += head.vbo.length;
    ptr = (Byte*)glMapNamedBuffer(mesh.vertex_buffer, GL_READ_ONLY);
    memcpy(cur, ptr, head.vbo.length);
    glUnmapNamedBuffer(mesh.vertex_buffer);
    if (mesh.index_status > 31) {
        glGetNamedBufferParameteri64v(mesh.index_buffer, GL_BUFFER_SIZE,
                                      &head.ibo.length);
        head.ibo.start = cur - data;
        cur += head.ibo.length;
        ptr = (Byte*)glMapNamedBuffer(mesh.index_buffer, GL_READ_ONLY);
        memcpy(cur, ptr, head.ibo.length);
        glUnmapNamedBuffer(mesh.index_buffer);
    }
    for (size_t i = 0; i < mesh.buffers.size(); i++) {
        glGetNamedBufferParameteri64v(mesh.buffers[i], GL_BUFFER_SIZE,
                                      &head.buffers[i].length);
        head.buffers[i].start = cur - data cur += head.buffers[i].length;
        ptr = (Byte*)glMapNamedBuffer(mesh.buffers[i], GL_READ_ONLY);
        memcpy(cur, ptr, head.buffers[i].length);
        glUnmapNamedBuffer(mesh.buffers[i]);
    }
    Byte* res = CompressData(data, &full_size, sizeof(uint64));
    free(data);
    *(uint64*)res = MESH_HEADER;
    *ret_length = full_size;
    return res;
}
void PackMesh(const std::string& path, const Mesh& mesh) {
    size_t length;
    Byte* data = PackMesh(&length, mesh);
    std::ofstream fout(path,
                       std::ios::out | std::ios::binary | std::ios::trunc);
    if (!fout.is_open()) {
        throw std::runtime_error("Cannot open file:" + path);
    }
    fout.write((char*)data, length);
    free(data);
    fout.close();
}
void Mesh::GenMeshFile(const aiMesh* ptr, const string& save_path) {
    size_t length;
    Byte* data = GenMeshFile(ptr, save_path, &length);
    std::ofstream fout(save_path,
                       std::ios::out | std::ios::binary | std::ios::trunc);
    if (!fout.is_open()) {
        throw std::runtime_error("Cannot open file:" + save_path);
    }
    fout.write(data, length);
    fout.close();
}
Byte* Mesh::GenMeshFile(const aiMesh* pointer,
                        const std::string& name,
                        size_t* ret_length) {
    MeshFile head;
    head.restart_index = UINT64_MAX;
    head.buffer_count = 0;
    size_t vertex_length = sizeof(aiVector3D);  // 单个顶点数据的长度
    std::cout << "File:" << name << '\t' << pointer->mName.C_Str() << '\n';
    std::cout << "Vertex Format:\nPositions vec3";
    if (pointer->HasNormals()) {
        vertex_length += sizeof(aiVector3D);
        std::cout << "\nNormals vec3";
    }
    if (pointer->GetNumUVChannels() > 0) {
        std::cout << "\nTexture Coords:";
        for (size_t i = 0; i < pointer->GetNumUVChannels(); i++) {
            if (pointer->HasTextureCoords(i)) {
                vertex_length += sizeof(ai_real) * pointer->mNumUVComponents[i];
                cout << "\n\t"
                     << (pointer->HasTextureCoordsName(i)
                             ? pointer->mTextureCoordsNames[i]->C_Str()
                             : "NULL")
                     << ": vec" << pointer->mNumUVComponents[i];
            }
        }
    }
    if (pointer->GetNumColorChannels() > 0) {
        vertex_length += sizeof(aiColor4D) * pointer->GetNumColorChannels();
        std::cout << "\nColors: vec4 *" << pointer->GetNumColorChannels();
    }
    if (pointer->HasTangentsAndBitangents()) {
        vertex_length += sizeof(aiVector3D) * 2;
        std::cout << "\nTangents vec3\nBitangents vec3";
    }

    head.vbo.start = sizeof(MeshFile);
    head.vbo.length = vertex_length * pointer->mNumVertices;
    if (pointer->HasFaces()) {
        head.index_status = IndexStatus::ONLY_INDEX;
        head.primitive_type = GL_TRIANGLES;
        head.ibo.start = sizeof(MeshFile) + head.vbo.length;
        head.ibo.length = pointer->mNumFaces * sizeof(unsigned int) * 3;
        std::cout << "\nFaces: triangles, unsigned int * 3";
    } else {
        head.index_status = IndexStatus::NO_INDEX;
        head.primitive_type = GL_NONE;
        head.ibo.start = UINT64_MAX;
        head.ibo.length = 0ULL;
    }
    std::cout << std::endl;

    size_t length = sizeof(MeshFile) + head.vbo.length + head.ibo.length,
           rl = length;
    Byte *mesh_data = (Byte*)malloc(length),
         *curpos = mesh_data + sizeof(MeshFile);
    if (mesh_data == nullptr) {
        throw std::bad_alloc();
    }
    std::memcpy(mesh_data, &head, sizeof(MeshFile));
    for (size_t i = 0; i < pointer->mNumVertices; i++) {
        *((aiVector3D*)curpos) = pointer->mVertices[i];
        curpos += sizeof(aiVector3D);
        if (pointer->HasNormals()) {
            *((aiVector3D*)curpos) = pointer->mNormals[i];
            curpos += sizeof(aiVector3D);
        }
        for (size_t j = 0; j < pointer->GetNumUVChannels(); j++) {
            if (pointer->HasTextureCoords(j)) {
                *((aiVector3D*)curpos) = pointer->mTextureCoords[j][i];
                curpos += sizeof(ai_real) * pointer->mNumUVComponents[j];
            }
        }
        for (size_t j = 0; j < pointer->GetNumColorChannels(); j++) {
            if (pointer->HasVertexColors(j)) {
                *((aiColor4D*)curpos) = pointer->mColors[j][i];
                curpos += sizeof(aiColor4D);
            }
        }
        if (pointer->HasTangentsAndBitangents()) {
            *((aiVector3D*)curpos) = pointer->mTangents[i];
            curpos += sizeof(aiVector3D);
            *((aiVector3D*)curpos) = pointer->mBitangents[i];
            curpos += sizeof(aiVector3D);
        }
    }
    for (size_t i = 0; i < pointer->mNumFaces; i++) {
        for (size_t j = 0; j < 3; j++) {
            *((unsigned int*)curpos) = pointer->mFaces[i].mIndices[j];
            curpos += sizeof(unsigned int);
        }
    }
    Byte* data = CompressData(mesh_data, &length, sizeof(uint64));
    free(mesh_data);
    *(uint64*)data = MESH_HEADER;
    cout << "Vertices Count:" << pointer->mNumVertices << '\n';
    cout << "Indices Count:" << pointer->mNumFaces * 3 << '\n';
    cout << "Data size:" << rl + sizeof(uint64) << "Bytes\n";
    cout << "END;" << endl;
    *ret_length = length + sizeof(uint64);
    return data;
}
void Mesh::GenMeshFile(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, assimp_load_process);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        throw std::runtime_error(importer.GetErrorString());
    }
    for (size_t i = 0; i < scene->mNumMeshes; i++) {
        GenMeshFile(scene->mMeshes[i], path + std::to_string(i) +
                                           scene->mMeshes[i]->mName.C_Str() +
                                           ".mesh");
    }
}
inline void Mesh::GenMeshFile(const char* path) {
    GenMeshFile(std::string(path));
}
void Mesh::GenMeshFileMerged(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, assimp_load_process);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        throw std::runtime_error(importer.GetErrorString());
    }
    std::ofstream file(path,
                       std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open out file.");
    }
    uint64 out = 0xF242191756FF0003;
    file.write((char*)&out, sizeof(out));
    out = scene->mNumMeshes;
    file.write((char*)&out, sizeof(out));
    Byte* data;
    size_t length;
    for (size_t i = 0; i < scene->mNumMeshes; i++) {
        data = GenMeshFile(scene->mMeshes[i],
                           path + std::to_string(i) +
                               scene->mMeshes[i]->mName.C_Str() + ".mesh",
                           &length);
        file.write((char*)data, length);
        free(data);
    }
}
inline void Mesh::GenMeshFileMerged(const char* path) {
    GenMeshFileMerged(std::string(path));
}
Mesh::~Mesh() {
    glDeleteVertexArrays(1, &vertex_array);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(buffers.size(), &buffers[0]);
    if (index_status > 31) {
        glDeleteBuffers(1, &index_buffer);
    }
}
Texture::Texture() {
    texture_id = 0;
}
Texture::~Texture() {
    glDeleteTextures(1, &texture_id);
}

void LoadTexture(const std::string& path,
                 Texture& tex,
                 GLsizei add_mipmap_level,
                 GLsizei samples,
                 GLboolean fixedsample) {
    if (add_mipmap_level < 0) {
        throw std::logic_error("argument add_mipmap_level can't be negative.");
    }
    std::ifstream in(path, std::ios::in | std::ios::binary);
    {
        if (!in.is_open()) {
            throw std::runtime_error("Cannot open file:" + path);
        }
        uint64 head;
        in.read((char*)&head, sizeof(uint64));
        if (head != TEXTURE_HEADER) {
            throw std::runtime_error("Texture head code error.");
        }
    }
    size_t length;
    length = in.tellg();
    in.seekg(0, std::ios::end);
    length = in.tellg() - length;
    Byte* data = (Byte*)malloc(length);
    if (!data) {
        in.close();
        throw std::bad_alloc();
    }
    in.seekg(sizeof(uint64), std::ios::beg);
    in.read((char*)data, length);
    in.close();

    TextureFileN& tf = *(TextureFileN*)data;
    glCreateTextures(tf.target, 1, &tex.texture_id);
    if (tf.target == GL_TEXTURE_1D) {
        glTextureStorage1D(tex.texture_id, tf.mipLevels + add_mipmap_level,
                           tf.internal_format, tf.mip[0].width);
        for (GLsizei i = 0; i < tf.mipLevels; i++) {
            glTextureSubImage1D(tex.texture_id, i, 0, tf.mip[i].width,
                                tf.format, tf.type,
                                data + tf.mip[i].range.start);
        }
    } else if (tf.target == GL_TEXTURE_2D) {
        glTextureStorage2D(tex.texture_id, tf.mipLevels + add_mipmap_level,
                           tf.internal_format, tf.mip[0].width,
                           tf.mip[0].height);
        for (GLsizei i = 0; i < tf.mipLevels; i++) {
            glTextureSubImage2D(tex.texture_id, i, 0, 0, tf.mip[i].width,
                                tf.mip[i].height tf.format, tf.type,
                                data + tf.mip[i].range.start);
        }
    } else if (tf.target == GL_TEXTURE_3D) {
        glTextureStorage3D(tex.texture_id, tf.mipLevels + add_mipmap_level,
                           tf.internal_format, tf.mip[0].width,
                           tf.mip[0].height, tf.mip[0].depth);
        for (GLsizei i = 0; i < tf.mipLevels; i++) {
            glTextureSubImage3D(tex.texture_id, i, 0, 0, 0, tf.mip[i].width,
                                tf.mip[i].height, tf.mip[i].depth tf.format,
                                tf.type, data + tf.mip[i].range.start);
        }
    } else if (tf.target == GL_TEXTURE_1D_ARRAY) {
        glTextureStorage2D(tex.texture_id, tf.mipLevels + add_mipmap_level,
                           tf.internal_format, tf.mip[0].width, tf.slices);
        for (GLsizei i = 0; i < tf.mipLevels; i++) {
            glTextureSubImage2D(tex.texture_id, i, 0, 0, tf.mip[i].width,
                                tf.slices, tf.format, tf.type,
                                data + tf.mip[i].range.start);
        }
    } else if (tf.target == GL_TEXTURE_2D_ARRAY ||
               tf.target == GL_TEXTURE_CUBE_MAP ||
               tf.target == GL_TEXTURE_CUBE_MAP_ARRAY) {
        glTextureStorage3D(tex.texture_id, tf.mipLevels + add_mipmap_level,
                           tf.internal_format, tf.mip[0].width,
                           tf.mip[0].height, tf.slices);
        for (GLsizei i = 0; i < tf.mipLevels; i++) {
            glTextureSubImage3D(tex.texture_id, i, 0, 0, 0, tf.mip[i].width,
                                tf.mip[i].height, tf.slices, tf.format, tf.type,
                                data + tf.mip[i].range.start);
        }
    } else if (tf.target == GL_TEXTURE_2D_MULTISAMPLE) {
        glTexStorage2DMultisample(tex.texture_id, samples, tf.internal_format,
                                  tf.mip[0].width, tf.mip[0].height,
                                  fixedsample);
        glTextureSubImage2D(tex.texture_id, 0, 0, 0, tf.mip[0].width,
                            tf.mip[0].height, tf.format, tf.type,
                            data + tf.mip[0].range.start);
        if (tf.mipLevels > 1) {
            WARNING("OpenGL", "多重采样纹理不应含有mipmap");
        }
    } else if (tf.target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
        glTexStorage3DMultisample(tex.texture_id, samples, tf.internal_format,
                                  tf.mip[0].width, tf.mip[0].height, tf.slices,
                                  fixedsample);
        glTextureSubImage3D(tex.texture_id, 0, 0, 0, 0, tf.mip[0].width,
                            tf.mip[0].height, tf.slices, tf.format, tf.type,
                            data + tf.mip[0].range.start);
        if (tf.mipLevels > 1) {
            WARNING("OpenGL", "多重采样纹理不应含有mipmap");
        }
    }
    if (tf.enable_swizzle == GL_TRUE) {
        glTextureParameteriv(tex.texture_id, GL_TEXTURE_SWIZZLE_RGBA,
                             tf.swizzle);
    }
}
void Texture::GenTextureFile(const std::string& path) {
    GenTextureFile(path.c_str());
}
inline void Texture::GenTextureFile(const char* path) {
    TextureFile<1> tf;
    tf.target = GL_TEXTURE_2D;
    tf.type = GL_UNSIGNED_BYTE;
    tf.enable_swizzle = GL_FALSE;
    tf.mipLevels = 1;
    tf.slices = 0;
    int n;
    Byte *data =
             (Byte*)stbi_load(path, &tf.mip[0].width, &tf.mip[0].height, &n, 0),
         copy;
    if (data == nullptr) {
        throw std::runtime_error("STB_IMAGE:无法加载图像");
    }
    tf.totalSize = tf.mip[0].range.length =
        th.width * th.height * n * sizeof(uint8_t);
    tf.mip[0].depth = 0;
    tf.mip[0].range.start = sizeof(tf);
    std::cout << "File:\t" << path;
    std::cout << "Image Data Size:\t" << tf.totalSize << "Bytes\n";
    switch (n) {
        case 1:
            tf.internal_format = GL_R8;
            tf.format = GL_RED;
            tf.swizzle = {GL_RED, GL_ZERO, GL_ZERO, GL_ZERO};
            break;
        case 2:
            tf.internal_format = GL_RG8;
            tf.format = GL_RG;
            tf.swizzle = {GL_RED, GL_GREEN, GL_BLUE, GL_ZERO};
            break;
        case 3:
            tf.internal_format = GL_RGB8;
            tf.format = GL_RGB;
            tf.swizzle = {GL_RED, GL_GREEN, GL_BLUE, GL_ZERO};
            break;
        case 4:
            tf.internal_format = GL_RGBA8;
            tf.format = GL_RGBA;
            tf.swizzle = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
            break;
        default:
            throw std::runtime_error("图像通道数错误");
    }
    copy = (Byte*)malloc(sizeof(tf) + tf.totalSize);
    if (!copy) {
        throw std::bad_alloc();
    }
    memcpy(copy, &tf, sizeof(tf));
    memcpy(copy + sizeof(tf), data, tf.totalSize);
    stbi_image_free(data);
    size_t size = tf.totalSize;
    Byte* compress = CompressData(copy, &size, sizeof(uint64));
    free(copy);
    *(uint64*)compress = TEXTURE_HEADER;
    std::ofstream out(std::string(path) + ".out.texture",
                      std::ios::out | std::ios::binary | std::ios::trunc);
    out.write((char*)compress, size);
    out.close();
}
inline Program::Program() {
    program_id = glCreateProgram();
}
inline Program::~Program() {
    glDeleteProgram(program_id);
}
inline ShaderInfo& Program::operator[](size_t index) {
    return program_shader[index];
};
void Program::PrintLog() const {
    GLint success;
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLsizei length;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &length);
        char* log = (char*)malloc(length);
        if (log == nullptr) {
            throw std::bad_alloc();
        } else {
            glGetProgramInfoLog(program_id, length, nullptr, log);
            ERROR("OpenGL", "着色器链接错误:", log,
                  "\n--------------------------------");
            free(log);
        }
    }
}
void Program::AddShader(const std::string& path, GLenum type) {
    GLuint shader_id = Program::LoadShader(path, type);
    glAttachShader(program_id, shader_id);
    program_shader.push_back({type, shader_id});
}
inline void Program::Link() const {
    glLinkProgram(program_id);
    PrintLog();
}

inline void Program::Use() const {
    glUseProgram(program_id);
}
inline void Program::UnUse() const {
    glUseProgram(0);
}

static GLuint Program::LoadShader(const std::string& path, GLenum type) {
    std::ifstream reader(path, std::ios::in);
    if (!reader.is_open()) {
        throw std::runtime_error("Cannot open file.");
    }
    std::string shader_code(std::istreambuf_iterator<char>(reader),
                            std::istreambuf_iterator<char>());
    reader.close();
    GLuint shader_id = glCreateShader(type), success;
    const char* res = shader_code.c_str();
    const GLint length = static_cast<GLint>(shader_code.size());
    glShaderSource(shader_id, 1, &res, &length);
    glCompileShader(shader_id);
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        int length;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
        char* log = (char*)malloc(sizeof(char) * length);
        if (log == nullptr) {
            throw std::bad_alloc();
        } else {
            glGetShaderInfoLog(shader_id, length, &length, log);
            ERROR("OpenGL", "着色器编译错误:", shader_code,
                  "\n--------------------------------", log,
                  "\n--------------------------------");
            free(log);
            glDeleteShader(shader_id);
            throw std::runtime_error("OpenGL:着色器编译错误:");
        }
    }
    return shader_id;
}
static inline GLuint Program::LoadShader(const char* path, GLenum type) {
    return LoadShader(std::string(path), type);
}

GLint Program::GetUniformLocation(const std::string& target) const {
    if (uniformmap.find(target) != uniformmap.end())
        return uniformmap[target];
    GLint location = glGetUniformLocation(this->program_id, target.c_str());
    if (location != -1)
        uniformmap[target] = location;
    return location;
}
GLint Program::GetUniformBlockLocation(const std::string& target) const {
    if (uniformmap.find(target) != uniformmap.end())
        return uniformmap[target];
    GLint location = glGetUniformBlockIndex(this->program_id, target.c_str());
    if (location != -1)
        uniformmap[target] = location;
    return location;
}
void Program::SetTexture(const std::string& name, GLint value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform1i(location, value);
}
void Program::SetBool(const std::string& name, GLboolean value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform1i(location, (int)value);
}
void Program::SetInt(const std::string& name, GLint value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform1i(location, value);
}
void Program::SetUint(const std::string& name, GLuint value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform1ui(location, value);
}
void Program::SetFloat(const std::string& name, GLfloat value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform1f(location, value);
}
void Program::SetVec2(const std::string& name, const glm::vec2& value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform2fv(location, 1, &value[0]);
}
void Program::SetVec2(const std::string& name, GLfloat x, GLfloat y) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform2f(location, x, y);
}

void Program::SetVec3(const std::string& name, const glm::vec3& value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform3fv(location, 1, &value[0]);
}
void Program::SetVec3(const std::string& name,
                      GLfloat x,
                      GLfloat y,
                      GLfloat z) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform3f(location, x, y, z);
}

void Program::SetVec4(const std::string& name, const glm::vec4& value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform4fv(location, 1, &value[0]);
}
void Program::SetVec4(const std::string& name,
                      GLfloat x,
                      GLfloat y,
                      GLfloat z,
                      GLfloat w) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform4f(location, x, y, z, w);
}
void Program::SetVec2Array(const std::string& name,
                           GLsizei count,
                           const glm::vec2* value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform2fv(location, count, (GLfloat*)value);
}
void Program::SetVec3Array(const std::string& name,
                           GLsizei count,
                           const glm::vec3* value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform3fv(location, count, (GLfloat*)value);
}
void Program::SetVec4Array(const std::string& name,
                           GLsizei count,
                           const glm::vec4* value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform4fv(location, count, (GLfloat*)value);
}
void Program::SetMat2(const std::string& name, const glm::mat2& mat) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniformMatrix2fv(location, 1, GL_FALSE, &mat[0][0]);
}
void Program::SetMat3(const std::string& name, const glm::mat3& mat) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
}
void Program::SetMat4(const std::string& name, const glm::mat4& mat) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
}
void Program::SetMat2Array(const std::string& name,
                           GLsizei count,
                           const glm::mat2* mat) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniformMatrix2fv(location, count, GL_FALSE, (GLfloat*)mat);
}
void Program::SetMat3Array(const std::string& name,
                           GLsizei count,
                           const glm::mat3* mat) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniformMatrix3fv(location, count, GL_FALSE, (GLfloat*)mat);
}
void Program::SetMat4Array(const std::string& name,
                           GLsizei count,
                           const glm::mat4* mat) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniformMatrix4fv(location, count, GL_FALSE, (GLfloat*)mat);
}
void Program::SetDouble(const std::string& name, GLdouble value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform1d(location, value);
}
void Program::SetVec2(const std::string& name, const glm::dvec2& value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform2dv(location, 1, &value[0]);
}
void Program::SetVec2Array(const std::string& name,
                           GLsizei count,
                           const glm::dvec2* value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform2dv(location, count, (GLdouble*)value);
}
void Program::SetVec2(const std::string& name, GLdouble x, GLdouble y) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform2d(location, x, y);
}

void Program::SetVec3(const std::string& name, const glm::dvec3& value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform3dv(location, 1, &value[0]);
}
void Program::SetVec3Array(const std::string& name,
                           GLsizei count,
                           const glm::dvec3* value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform3dv(location, count, (GLdouble*)value);
}
void Program::SetVec3(const std::string& name,
                      GLdouble x,
                      GLdouble y,
                      GLdouble z) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform3d(location, x, y, z);
}

void Program::SetVec4(const std::string& name, const glm::dvec4& value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform4dv(location, 1, &value[0]);
}
void Program::SetVec4Array(const std::string& name,
                           GLsizei count,
                           const glm::dvec4* value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform4dv(location, count, (GLdouble*)value);
}
void Program::SetVec4(const std::string& name,
                      GLdouble x,
                      GLdouble y,
                      GLdouble z,
                      GLdouble w) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform4d(location, x, y, z, w);
}

void Program::SetMat2(const std::string& name, const glm::dmat2& mat) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniformMatrix2dv(location, 1, GL_FALSE, &mat[0][0]);
}
void Program::SetMat3(const std::string& name, const glm::dmat3& mat) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniformMatrix3dv(location, 1, GL_FALSE, &mat[0][0]);
}
void Program::SetMat4(const std::string& name, const glm::dmat4& mat) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniformMatrix4dv(location, 1, GL_FALSE, &mat[0][0]);
}
void Program::SetMat2Array(const std::string& name,
                           GLsizei count,
                           const glm::dmat2* mat) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniformMatrix2dv(location, count, GL_FALSE, (GLdouble*)mat);
}
void Program::SetMat3Array(const std::string& name,
                           GLsizei count,
                           const glm::dmat3* mat) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniformMatrix3dv(location, count, GL_FALSE, (GLdouble*)mat);
}
void Program::SetMat4Array(const std::string& name,
                           GLsizei count,
                           const glm::dmat4* mat) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniformMatrix4dv(location, count, GL_FALSE, (GLdouble*)mat);
}
void Program::SetVec2(const std::string& name, const glm::ivec2& value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform2iv(location, 1, &value[0]);
}
void Program::SetVec2(const std::string& name, GLint x, GLint y) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform2i(location, x, y);
}
void Program::SetVec3(const std::string& name, const glm::ivec3& value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform3iv(location, 1, &value[0]);
}
void Program::SetVec3(const std::string& name, GLint x, GLint y, GLint z) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform3i(location, x, y, z);
}
void Program::SetVec4(const std::string& name, const glm::ivec4& value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform4iv(location, 1, &value[0]);
}
void Program::SetVec4(const std::string& name,
                      GLint x,
                      GLint y,
                      GLint z,
                      GLint w) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform4i(location, x, y, z, w);
}
void Program::SetVec2(const std::string& name, const glm::uvec2& value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform2uiv(location, 1, &value[0]);
}
void Program::SetVec2(const std::string& name, GLuint x, GLuint y) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform2ui(location, x, y);
}
void Program::SetVec3(const std::string& name, const glm::uvec3& value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform3uiv(location, 1, &value[0]);
}
void Program::SetVec3(const std::string& name, GLuint x, GLuint y, GLuint z) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform3ui(location, x, y, z);
}
void Program::SetVec4(const std::string& name, const glm::uvec4& value) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform4uiv(location, 1, &value[0]);
}
void Program::SetVec4(const std::string& name,
                      GLuint x,
                      GLuint y,
                      GLuint z,
                      GLuint w) {
    GLint location = GetUniformLocation(name);
    if (location != -1)
        glUniform4ui(location, x, y, z, w);
}
}  // namespace Boundless
