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
inline void LoadMesh(std::string_view path, Mesh& mesh) {
    std::ifstream fin(path, std::ios::in | std::ios::binary);
    if (!fin.is_open()) {
        throw std::runtime_error("Cannot open file:" + path);
    }
    LoadMesh(fin, mesh);
    fin.close();
}
inline void LoadMesh(const char* path, Mesh& mesh) {
    LoadMesh(std::string_view(path), mesh);
}
void LoadMeshMultple(std::string_view path, std::vector<Mesh>& meshs) {
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
    LoadMeshMultple(std::string_view(path), meshs);
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
void PackMesh(std::string_view path, const Mesh& mesh) {
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
                        std::string_view name,
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
void Mesh::GenMeshFile(std::string_view path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, assimp_load_process);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        throw std::runtime_error(importer.GetErrorString());
    }
    for (size_t i = 0; i < scene->mNumMeshes; i++) {
        GenMeshFile(scene->mMeshes[i], std::string(path) + std::to_string(i) +
                                           scene->mMeshes[i]->mName.C_Str() +
                                           ".mesh");
    }
}
inline void Mesh::GenMeshFile(const char* path) {
    GenMeshFile(std::string_view(path));
}
void Mesh::GenMeshFileMerged(std::string_view path) {
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
    GenMeshFileMerged(std::string_view(path));
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

void LoadTexture(std::string_view path,
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
    tex.target = tf.target;
    tex.width = tf.mip[0].width;
    tex.height = tf.mip[0].height;
    tex.depth = tf.mip[0].depth;
    if (tf.enable_swizzle == GL_TRUE) {
        glTextureParameteriv(tex.texture_id, GL_TEXTURE_SWIZZLE_RGBA,
                             tf.swizzle);
    }
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
}
inline void LoadTexture(
    const char* path,
    Texture& tex,
    GLsizei add_mipmap_level = 0,
    GLsizei samples = default_texture_samples,
    GLboolean fixedsample = default_texture_fixedsamplelocation) {
    LoadTexture(std::string_view(path), tex, add_mipmap_level, samples,
                fixedsample);
}
void PackTexture(std::string_view save_path,
                 Texture& tex,
                 GLsizei level,
                 GLenum format,
                 GLenum type) {
    size_t length;
    Byte* data = PackTexture(&length, tex, level, format, type);
    std::ofstream out(save_path,
                      std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        throw std::runtime_error("Connot open out file");
    }
    out.write((char*)data, length);
    out.close();
    free(data);
}
constexpr size_t TextureInternalFormatSize(GLenum type) {
    if (GL_R3_G3_B2 || GL_R8 || GL_R8_SNORM || GL_R8I || GL_R8UI || GL_RGBA2) {
        return 1;
    } else if (GL_R16 || GL_R16_SNORM || GL_R16F || GL_R16I || GL_R16UI ||
               GL_RG16 || GL_RG8 || GL_RG8_SNORM || GL_RG8I || GL_RG8UI ||
               GL_RGB5_A1 || GL_RGB565 || GL_RGBA4) {
        return 2;
    } else if (GL_RGB8 || GL_RGB8_SNORM || GL_RGB8I || GL_RGB8UI || GL_SRGB8) {
        return 3;
    } else if (GL_R11F_G11F_B10F || GL_R32F || GL_R32I || GL_R32UI ||
               GL_RG16_SNORM || GL_RG16F || GL_RG16I || GL_RG16UI ||
               GL_RGB10_A2 || GL_RGB9_E5 || GL_RGBA8 || GL_RGBA8_SNORM ||
               GL_RGBA8I || GL_RGBA8UI || GL_SRGB8_ALPHA8 || GL_RGB16) {
        return 4;
    } else if (GL_RGB16_SNORM || GL_RGB16F || GL_RGB16I || GL_RGB16UI ||
               GL_RGBA12) {
        return 6;
    } else if (GL_RG32F || GL_RG32I || GL_RG32UI || GL_RGBA16 ||
               GL_RGBA16_SNORM || GL_RGBA16F || GL_RGBA16I || GL_RGBA16I ||
               GL_RGBA16UI) {
        return 8;
    } else if (GL_RGB32F || GL_RGB32I || GL_RGB32UI) {
        return 12;
    } else if (GL_RGBA32F || GL_RGBA32I || GL_RGBA32UI) {
        return 16;
    } else {
        return 0;
    }
}
constexpr size_t TypeSize(GLenum type) {
    switch (type) {
        case GL_BYTE:
            return sizeof(GLbyte);
        case GL_BOOL:
            return sizeof(GLbyte);
        case GL_UNSIGNED_BYTE:
            return sizeof(GLubyte);
        case GL_SHORT:
            return sizeof(GLshort);
        case GL_UNSIGNED_SHORT:
            return sizeof(GLushort);
        case GL_INT:
            return sizeof(GLint);
        case GL_UNSIGNED_INT:
            return sizeof(GLuint);
        case GL_FIXED:
            return sizeof(GLfixed);
        case GL_FLOAT:
            return sizeof(GLfloat);
        case GL_HALF_FLOAT:
            return sizeof(GLhalf);
        case GL_DOUBLE:
            return sizeof(GLdouble);
        case GL_INT_VEC2:
            return 2 * sizeof(GLint);
        case GL_UNSIGNED_INT_VEC2:
            return 2 * sizeof(GLuint);
        case GL_INT_VEC3:
            return 3 * sizeof(GLint);
        case GL_UNSIGNED_INT_VEC3:
            return 3 * sizeof(GLuint);
        case GL_INT_VEC4:
            return 4 * sizeof(GLint);
        case GL_UNSIGNED_INT_VEC4:
            return 4 * sizeof(GLuint);
        case GL_INT_2_10_10_10_REV:
            return sizeof(GLuint);
        case GL_UNSIGNED_INT_2_10_10_10_REV:
            return sizeof(GLuint);
        case GL_FLOAT_VEC2:
            return 2 * sizeof(GLfloat);
        case GL_FLOAT_VEC3:
            return 3 * sizeof(GLfloat);
        case GL_FLOAT_VEC4:
            return 4 * sizeof(GLfloat);
        case GL_FLOAT_MAT2:
            return 4 * sizeof(GLfloat);
        case GL_FLOAT_MAT2x3:
            return 6 * sizeof(GLfloat);
        case GL_FLOAT_MAT2x4:
            return 8 * sizeof(GLfloat);
        case GL_FLOAT_MAT3:
            return 9 * sizeof(GLfloat);
        case GL_FLOAT_MAT3x2:
            return 6 * sizeof(GLfloat);
        case GL_FLOAT_MAT3x4:
            return 12 * sizeof(GLfloat);
        case GL_FLOAT_MAT4:
            return 16 * sizeof(GLfloat);
        case GL_FLOAT_MAT4x2:
            return 8 * sizeof(GLfloat);
        case GL_FLOAT_MAT4x3:
            return 12 * sizeof(GLfloat);
        case GL_BOOL_VEC2:
            return 2 * sizeof(GLbyte);
        case GL_BOOL_VEC3:
            return 3 * sizeof(GLbyte);
        case GL_BOOL_VEC4:
            return 4 * sizeof(GLbyte);
        default:
            return 0;
    }
}
constexpr size_t TextureExternalFormatSize(GLenum format, GLenum type) {
    size_t res;
    if (format == GL_RED || format == GL_GREEN || format == GL_BLUE ||
        format == GL_RED_INTEGER || format == GL_GREEN_INTEGER ||
        format == GL_BLUE_INTEGER) {
        res = 1;
    } else if (format == GL_RG || format == GL_RG_INTEGER) {
        res = 2;
    } else if (format == GL_RGB || format == GL_RGB_INTEGER) {
        res = 3;
    } else if (format == GL_RGBA || format == GL_RGBA_INTEGER) {
        res = 4;
    }
    res *= TypeSize(type);
    return res;
}
Byte* PackTexture(size_t* ret_length,
                  Texture& tex,
                  GLsizei level,
                  GLenum format,
                  GLenum type) {
    // 统计纹理文件数据的长度
    size_t length = sizeof(TextureFileN) + sizeof(TextureMipData) * level,
           maxlen = 0;
    TextureMipData mips[level];  // 暂存mipmap数据
    size_t format_size, cnt;
    format_size =
        TextureExternalFormatSize(format, type);  // 存储单个像素点的数据长度
    // 遍历每一层mipmap，获取其长宽高
    for (GLsizei i = 0; i < level; i++) {
        glGetTextureLevelParameteriv(tex.texture_id, i, GL_TEXTURE_WIDTH,
                                     (GLint*)&mips[i].width);
        if (mips[i].width == 0) {
            throw std::runtime_error("纹理宽度不能为0");
        }
        cnt = mips[i].width;
        glGetTextureLevelParameteriv(tex.texture_id, i, GL_TEXTURE_HEIGHT,
                                     (GLint*)&mips[i].height);
        if (mips[i].height != 0) {
            cnt *= mips[i].height;
        }
        glGetTextureLevelParameteriv(tex.texture_id, i, GL_TEXTURE_DEPTH,
                                     (GLint*)&mips[i].depth);
        if (mips[i].depth != 0) {
            cnt *= mips[i].depth;
        }
        mips[i].range.length = cnt * format_size;
        if (mips[i].range.length > maxlen) {
            maxlen = miplen[i];
        }
        length += mips[i].range.length;
    }
    Byte *data = (Byte*)malloc(length), cur = data;
    if (!data) {
        throw std::bad_alloc();
    }
    TextureFileN& head = *(TextureFileN*)data;
    cur += sizeof(TextureFileN) + sizeof(TextureMipData) * level;

    GLuint ppb;
    glCreateBuffers(1, &ppb);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, ppb);
    glNamedBufferStorage(ppb, maxlen, nullptr, GL_MAP_READ_BIT);
    // 读取纹理数据
    for (GLsizei i = 0; i < level; i++) {
        glGetTextureImage(tex.texture_id, i, format, type, maxlen, 0);
        void* map = glMapNamedBuffer(ppb, GL_READ_ONLY);
        memcpy(cur, map, miplen[i]);
        glUnmapNamedBuffer(ppb);
        head.mip[i].range.start = cur - data;
        cur += head.mip[i].range.length;
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glDeleteBuffers(1, &ppb);
    // 生成TextureFile头数据
    memcpy(head.mip, mips, sizeof(TextureMipData) * level);
    head.target = tex.target;
    glGetTextureLevelParameteriv(tex.texture_id, i, GL_TEXTURE_INTERNAL_FORMAT,
                                 (GLint*)&head.internal_format);
    head.format = format;
    head.type = type;
    glGetTextureParameterIuiv(tex.texture_id, i, GL_TEXTURE_SWIZZLE_RGBA,
                              head.swizzle);
    // 检查是否需要RGBA乱序
    if (head.swizzle[0] == GL_RED && head.swizzle[1] == GL_GREEN &&
        head.swizzle[2] == GL_BLUE && head.swizzle[3] == GL_ALPHA) {
        head.enable_swizzle = GL_FALSE;
    } else {
        head.enable_swizzle = GL_TRUE;
    }
    head.mipLevels = level;
    // 根据纹理类型设置切片数据
    if (tex.target == GL_TEXTURE_1D_ARRAY) {
        head.slices = head.mip[0].height;
        for (GLsizei i = 0; i < level; i++) {
            head.mip[i].range.length /= head.slices;
        }
    } else if (tex.target == GL_TEXTURE_2D_ARRAY ||
               tex.target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY ||
               tex.target == GL_TEXTURE_CUBE_MAP ||
               tex.target == GL_TEXTURE_CUBE_MAP_ARRAY) {
        head.slices = head.mip[0].depth;
        for (GLsizei i = 0; i < level; i++) {
            head.mip[i].range.length /= head.slices;
        }
    }
    head.totalSize =
        length - sizeof(TextureFileN) - sizeof(TextureMipData) * level;
    // 压缩
    Byte* compress = CompressData(data, &length, sizeof(uint64));
    free(data);
    *(uint64*)compress = TEXTURE_HEADER;
    return compress;
}
inline void PackTexture(const char* save_path,
                        Texture& tex,
                        GLsizei level,
                        GLenum format,
                        GLenum type) {
    PackTexture(std::string_view(path), tex, level, format, type);
}
inline void Texture::GenTextureFile(std::string_view path) {
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
void Texture::GenTextureFile(const char* path) {
    GenTextureFile(std::string_view(path));
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
void Program::AddShader(std::string_view path, GLenum type) {
    GLuint shader_id = Program::LoadShader(path, type);
    glAttachShader(program_id, shader_id);
    program_shader.push_back({type, shader_id});
}
void AddShaderByCode(std::string_view data, GLenum type) {
    GLuint shader_id = Program::ComplieShader(data, type);
    glAttachShader(program_id, shader_id);
    program_shader.push_back({type, shader_id});
}
inline void Program::Link() const {
    glLinkProgram(program_id);
    PrintLog();
}

inline void Program::UseProgram(Program& p) {
    glUseProgram(p.program_id);
}
inline void Program::UnUseProgram() {
    glUseProgram(0);
}

static GLuint Program::LoadShader(std::string_view path, GLenum type) {
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
    return LoadShader(std::string_view(path), type);
}
static GLuint Program::ComplieShader(std::string_view code) {
    GLuint shader_id = glCreateShader(type), success;
    const char* res = code.begin();
    const GLint length = static_cast<GLint>(code.size());
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
            ERROR("OpenGL", "着色器编译错误:", code,
                  "\n--------------------------------", log,
                  "\n--------------------------------");
            free(log);
            glDeleteShader(shader_id);
            throw std::runtime_error("OpenGL:着色器编译错误:");
        }
    }
}

const Matrix4f& Transform::get_model() {
    if (edited) {
        model = Matrix4f::Zero();
        model(0, 0) = static_cast<float>(scale.x());
        model(1, 1) = static_cast<float>(scale.y());
        model(2, 2) = static_cast<float>(scale.z());
        model(3, 3) = 1.0f;
        model = rotate.normalize().toRotationMatrix().cast<float>() * model;
        model(0, 3) = static_cast<float>(
            position.x() * model(0, 0) + position.y() * model(0, 1) +
            position.z() * model(0, 2) + model(0, 3));
        model(1, 3) = static_cast<float>(
            position.x() * model(1, 0) + position.y() * model(1, 1) +
            position.z() * model(1, 2) + model(1, 3));
        model(2, 3) = static_cast<float>(
            position.x() * model(2, 0) + position.y() * model(2, 1) +
            position.z() * model(2, 2) + model(2, 3));
        model(3, 3) = static_cast<float>(
            position.x() * model(3, 0) + position.y() * model(3, 1) +
            position.z() * model(3, 2) + model(3, 3));
        edited = false;
    }
    return model;
}
inline void RenderObject::enable() {
    base_transform->roenble = true;
}
inline void RenderObject::disable() {
    base_transform->roenble = false;
}

inline Camera::Camera() : editedProj(true), editedView(true) {}
const Matrix4f& Camera::get_proj() {
    if (editedProj) {
        editedProj = false;
        if (isFrustum) {
            projection << static_cast<float>(2.0 * znear / width), 0.0f, 0.0f,
                0.0f, 0.0f, static_cast<float>(2.0 * znear / height), 0.0f,
                0.0f, 0.0f, 0.0f,
                static_cast<float>(-(zfar + znear) / (zfar - znear)),
                static_cast<float>(2.0 * zfar * znear / (zfar - znear)), 0.0f,
                0.0f, -1.0f, 0.0f;
        } else {
            projection << static_cast<float>(2.0 / width), 0.0f, 0.0f, 0.0f,
                0.0f, static_cast<float>(2.0 / height), 0.0f, 0.0f, 0.0f, 0.0f,
                static_cast<float>(-2.0 / (zfar - znear)),
                static_cast<float>(-(zfar + znear) / (zfar - znear)), 0.0f,
                0.0f, 0.0f, 1.0f;
        }
    }
    return projection;
}
const Matrix4f& Camera::get_view() {
    if (editedView) {
        editedView = false;
        forword = forword.normalize();
        Vector3d s = Vector3d::normalize(forword.cross(up));
        Vector3d u = Vector3d::normalize(forword.cross(s));
        view << static_cast<float>(s.x()), static_cast<float>(s.y()),
            static_cast<float>(s.z()), static_cast<float>(-s.dot(position)),
            static_cast<float>(u.x()), static_cast<float>(u.y()),
            static_cast<float>(u.z()), static_cast<float>(-u.dot(position)),
            static_cast<float>(-forword.x()), static_cast<float>(-forword.y()),
            static_cast<float>(-forword.z()),
            static_cast<float>(forword.dot(position)), 0.0f, 0.0f, 0.0f, 1.0f;
    }
    return view;
}
const Matrix4f& Camera::get_viewproj_matrix() {
    bool mult = false;
    if (editedProj) {
        editedProj = false;
        if (isFrustum) {
            projection << static_cast<float>(2.0 * znear / width), 0.0f, 0.0f,
                0.0f, 0.0f, static_cast<float>(2.0 * znear / height), 0.0f,
                0.0f, 0.0f, 0.0f,
                static_cast<float>(-(zfar + znear) / (zfar - znear)),
                static_cast<float>(2.0 * zfar * znear / (zfar - znear)), 0.0f,
                0.0f, -1.0f, 0.0f;
        } else {
            projection << static_cast<float>(2.0 / width), 0.0f, 0.0f, 0.0f,
                0.0f, static_cast<float>(2.0 / height), 0.0f, 0.0f, 0.0f, 0.0f,
                static_cast<float>(-2.0 / (zfar - znear)),
                static_cast<float>(-(zfar + znear) / (zfar - znear)), 0.0f,
                0.0f, 0.0f, 1.0f;
        }
        mult = true;
    }
    if (editedView) {
        editedView = false;
        forword = forword.normalize();
        Vector3d s = Vector3d::normalize(forword.cross(up));
        Vector3d u = Vector3d::normalize(forword.cross(s));
        view << static_cast<float>(s.x()), static_cast<float>(s.y()),
            static_cast<float>(s.z()), static_cast<float>(-s.dot(position)),
            static_cast<float>(u.x()), static_cast<float>(u.y()),
            static_cast<float>(u.z()), static_cast<float>(-u.dot(position)),
            static_cast<float>(-forword.x()), static_cast<float>(-forword.y()),
            static_cast<float>(-forword.z()),
            static_cast<float>(forword.dot(position)), 0.0f, 0.0f, 0.0f, 1.0f;
        mult = true;
    }
    if (mult) {
        vp_matrix = projection * view;
    }
    return vp_matrix;
}
Renderer::Renderer() : transform_head(nullptr) {}
Transform* Renderer::AddTransformNode(const Vector3d& vp,
                                      const Vector3d& vs,
                                      const Quaterniond& qr) {
    Transform* tfo = tr_pool.allocate();
    tfo->parent = nullptr;
    tfo->child_head = nullptr;
    if (!transform_head) {
        tfo->next_brother = nullptr;
        transform_head = tfo;
    } else {
        tfo->next_brother = transform_head;
        transform_head = tfo;
    }

    tfo->roenble = false;
    tfo->edited = true;
    tfo->position = vp;
    tfo->scale = vs;
    tfo->rotate = qr;
    tfo->render_obj = nullptr;
    return tfo;
}
Transform* Renderer::AddTransformNodeUnder(Transform* parent,
                                           const Vector3d& vp,
                                           const Vector3d& vs,
                                           const Quaterniond& qr) {
    Transform* tfo = tr_pool.allocate();
    tfo->parent = parent;
    tfo->child_head = nullptr;
    tfo->next_brother = parent->child_head;
    parent->child_head = tfo;

    tfo->roenble = false;
    tfo->edited = true;
    tfo->position = vp;
    tfo->scale = vs;
    tfo->rotate = qr;
    tfo->render_obj = nullptr;
    return tfo;
}
Transform* Renderer::AddTransformNodeRight(Transform* brother,
                                           const Vector3d& vp,
                                           const Vector3d& vs,
                                           const Quaterniond& qr) {
    Transform* tfo = tr_pool.allocate();
    tfo->parent = brother->parent;
    tfo->child_head = nullptr;
    tfo->next_brother = brother->next_brother;
    brother->next_brother = tfo;

    tfo->roenble = false;
    tfo->edited = true;
    tfo->position = vp;
    tfo->scale = vs;
    tfo->rotate = qr;
    tfo->render_obj = nullptr;
    return tfo;
}
void Renderer::DrawAll() {
    const Matrix4f& vp = camera.get_viewproj_matrix();
    Transform *cur_root = transform_head, p, tp;
    while (!cur_root) {
        mat_stack.push(vp * cur_root->get_model());
        if (cur_root->roenble)
            cur_root->render_obj->draw(mat_stack.top());
        p = cur_root->child_head;
        if (!p) {
            draw_ptrstack.push(nullptr);
            do {
                draw_ptrstack.push(p);
                p = p->next_brother;
            } while (!p);
            do {
                p = draw_ptrstack.top();
                draw_ptrstack.pop();
                if (p == nullptr) {
                    mat_stack.pop();
                    continue;
                }
                mat_stack.push(mat_stack.top() * p->get_model());
                if (cur_root->roenble)
                    cur_root->render_obj->draw(mat_stack.top());
                tp = p.child_head;
                if (!tp) {
                    draw_ptrstack.push(nullptr);
                    do {
                        draw_ptrstack.push(tp);
                        tp = tp->next_brother;
                    } while (!tp);
                } else {
                    mat_stack.pop();
                }
            } while (!draw_ptrstack.empty());
        }
        mat_stack.pop();
        cur_root = cur_root->next_brother;
    }
}
Renderer::~Renderer() {
    Transform *cur_root = transform_head, p, tp;
    while (!cur_root) {
        if (!cur_root->render_obj)
            cur_root->render_obj->~RenderObject();
        p = cur_root->child_head;
        if (!p) {
            do {
                draw_ptrstack.push(p);
                p = p->next_brother;
            } while (!p);
            do {
                p = draw_ptrstack.top();
                draw_ptrstack.pop();
                if (!cur_root->render_obj)
                    cur_root->render_obj->~RenderObject();
                tp = p.child_head;
                while (!tp) {
                    draw_ptrstack.push(tp);
                    tp = tp->next_brother;
                }
            } while (!draw_ptrstack.empty());
        }
        cur_root = cur_root->next_brother;
    }
}
}  // namespace Boundless
