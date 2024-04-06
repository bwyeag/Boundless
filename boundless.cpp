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
    int res =
        zlib::compress2(data_ptr, (zlib::uLong*)(data_ptr - sizeof(uint64)),
                        data, *length, compress_level);
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
    int res = zlib::uncompress2(uncompress_data, (zlib::uLong*)(ret_length),
                                data + sizeof(uint64) * 2,
                                (zlib::uLong*)(data + sizeof(uint64)));
    if (res != Z_OK) {
        free(uncompress_data);
        throw zlib::ZlibException(res);
    }
    return uncompress_data;
}

Mesh::Mesh() {
    vertex_array = 0;
}
Mesh::Mesh(IndexStatus indexst, size_t bufcnt) {
    glCreateVertexArrays(1, &vertex_array);
    glCreateBuffers(1, &vertex_buffer);
    if (bufcnt > 0) {
        buffers.resize(bufcnt);
        glCreateBuffers(bufcnt, &buffers[0]);
    }
    index_status = indexst;
    if (indexst != IndexStatus::NO_INDEX) {
        glCreateBuffers(1, &index_buffer);
    } else {
        index_buffer = 0;
    }
}
void Mesh::TryInit(IndexStatus indexst) {
    if (vertex_array == 0) {
        glCreateVertexArrays(1, &vertex_array);
        glCreateBuffers(1, &vertex_buffer);
        index_status = indexst;
        if (indexst != IndexStatus::NO_INDEX) {
            glCreateBuffers(1, &index_buffer);
        } else {
            index_buffer = 0;
        }
    } else if (indexst != IndexStatus::NO_INDEX) {
        if (index_buffer == 0) {
            glCreateBuffers(1, &index_buffer);
        }
    } else {
        if (index_buffer != 0) {
            glDeleteBuffers(1, &index_buffer);
        }
    }
}
void Mesh::InitMesh(const GLsizei* stride_array, const GLintptr* start_array) {
    if (index_status != IndexStatus::NO_INDEX) {
        glVertexArrayElementBuffer(vertex_array, index_buffer);
    }
    if (!start_array) {
        glVertexArrayVertexBuffer(vertex_array, 0, vertex_buffer,
                                  start_array[0], stride_array[0]);
        if (!buffers.empty()) {
            glVertexArrayVertexBuffers(vertex_array, 0, buffers.size(),
                                       &buffers[0], &start_array[1],
                                       &stride_array[1]);
        }
    } else {
        glVertexArrayVertexBuffer(vertex_array, 0, vertex_buffer, 0,
                                  stride_array[0]);
        if (!buffers.empty()) {
            GLintptr arr[buffers.size()];
            memset(arr, 0, sizeof(arr));
            glVertexArrayVertexBuffers(vertex_array, 0, buffers.size(),
                                       &buffers[0], arr, &stride_array[1]);
        }
    }
}
inline const std::vector<GLuint>& Mesh::GetBuffer() {
    return buffers;
}
inline void Mesh::SetPrimitiveType(GLenum type) {
    primitive_type = type;
}
inline void Mesh::SetRestartIndex(GLuint index) {
    restart_index = index;
}
inline GLuint Mesh::GetRestartIndex() {
    return restart_index;
}
inline GLenum Mesh::GetIndexType() {
    return index_type;
}
inline GLenum Mesh::GetPrimitiveType() {
    return primitive_type;
}
inline IndexStatus Mesh::GetIndexStatus() {
    return index_status;
}
inline GLuint Mesh::GetCount() {
    return mesh_count;
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
void Mesh::LoadMesh(const Byte* data, Mesh& mesh) {
    if (*(uint64*)data != MESH_HEADER) {
        throw std::runtime_error("Mesh head code error.");
    }
    size_t len;
    Byte* dt = UncompressData(data + sizeof(uint64), &len);
    MeshFile& head = *(MeshFile*)(dt);
    // const Byte* dp = dt + sizeof(MeshFile) + sizeof(DataRange) *
    // head.buffer_count;
    mesh.primitive_type = head.primitive_type;
    mesh.index_status = head.index_status;
    mesh.restart_index = head.restart_index;
    mesh.index_type = head.index_type;
    mesh.mesh_count = head.mesh_count;
    glCreateVertexArrays(1, &mesh.vertex_array);
    glCreateBuffers(1, &mesh.vertex_buffer);
    if (head.buffer_count > 0) {
        mesh.buffers.resize(head.buffer_count);
        glCreateBuffers(head.buffer_count, &mesh.buffers[0]);
    }
    glNamedBufferStorage(mesh.vertex_buffer, head.vbo.length,
                         dt + head.vbo.start, GL_MAP_READ_BIT);
    if (mesh.index_status != IndexStatus::NO_INDEX) {
        glCreateBuffers(1, &mesh.index_buffer);
        glNamedBufferStorage(mesh.index_buffer, head.ibo.length,
                             dt + head.ibo.start, GL_MAP_READ_BIT);
    }
    for (size_t i = 0; i < head.buffer_count; i++) {
        glNamedBufferStorage(mesh.buffers[i], head.buffers[i].length,
                             dt + head.buffers[i].start, GL_MAP_READ_BIT);
    }
    free(dt);
}
void Mesh::LoadMesh(std::ifstream& in, Mesh& mesh) {
    size_t length;
    std::streampos cur = in.tellg();
    in.seekg(sizeof(uint64), std::ios_base::cur);
    in.read((char*)&length, sizeof(uint64));
    in.seekg(cur);
    Byte* data = (Byte*)malloc(length);
    if (data == nullptr) {
        throw std::bad_alloc();
    }
    in.read((char*)data, length);
    LoadMesh(data, mesh);
    free(data);
}
inline void Mesh::LoadMesh(const std::string& path, Mesh& mesh) {
    std::ifstream fin(path, std::ios_base::in | std::ios_base::binary);
    if (!fin.is_open()) {
        throw std::runtime_error("Cannot open file:" + path);
    }
    LoadMesh(fin, mesh);
    fin.close();
}
inline void Mesh::LoadMesh(const char* path, Mesh& mesh) {
    std::ifstream fin(path, std::ios_base::in | std::ios_base::binary);
    if (!fin.is_open()) {
        throw std::runtime_error(std::string("Cannot open file:") + path);
    }
    LoadMesh(fin, mesh);
    fin.close();
}
void Mesh::LoadMeshMultple(const std::string& path, std::vector<Mesh>& meshs) {
    std::ifstream fin(path, std::ios_base::in | std::ios_base::binary);
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
void Mesh::LoadMeshMultple(const char* path, std::vector<Mesh>& meshs) {
    LoadMeshMultple(std::string(path), meshs);
}
Byte* Mesh::PackMesh(size_t* ret_length, const Mesh& mesh) {
    size_t full_size =
        sizeof(MeshFile) + sizeof(DataRange) * mesh.buffers.size();
    GLint64 length;
    glGetNamedBufferParameteri64v(mesh.vertex_buffer, GL_BUFFER_SIZE, &length);
    full_size += static_cast<size_t>(length);
    if (mesh.index_status != IndexStatus::NO_INDEX) {
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
    cur += sizeof(MeshFile) + sizeof(DataRange) * mesh.buffers.size();
    head.primitive_type = mesh.primitive_type;
    head.index_status = mesh.index_status;
    head.restart_index = mesh.restart_index;
    head.buffer_count = mesh.buffers.size();
    head.index_type = mesh.index_type;
    head.mesh_count = mesh.mesh_count;

    Byte* ptr;
    glGetNamedBufferParameteri64v(mesh.vertex_buffer, GL_BUFFER_SIZE,
                                  (GLint64*)&head.vbo.length);
    head.vbo.start = cur - data;
    cur += head.vbo.length;
    ptr = (Byte*)glMapNamedBuffer(mesh.vertex_buffer, GL_READ_ONLY);
    memcpy(cur, ptr, head.vbo.length);
    glUnmapNamedBuffer(mesh.vertex_buffer);
    if (mesh.index_status != IndexStatus::NO_INDEX) {
        glGetNamedBufferParameteri64v(mesh.index_buffer, GL_BUFFER_SIZE,
                                      (GLint64*)&head.ibo.length);
        head.ibo.start = cur - data;
        cur += head.ibo.length;
        ptr = (Byte*)glMapNamedBuffer(mesh.index_buffer, GL_READ_ONLY);
        memcpy(cur, ptr, head.ibo.length);
        glUnmapNamedBuffer(mesh.index_buffer);
    }
    for (size_t i = 0; i < mesh.buffers.size(); i++) {
        glGetNamedBufferParameteri64v(mesh.buffers[i], GL_BUFFER_SIZE,
                                      (GLint64*)&head.buffers[i].length);
        head.buffers[i].start = cur - data;
        cur += head.buffers[i].length;
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
void Mesh::PackMesh(const std::string& path, const Mesh& mesh) {
    size_t length;
    Byte* data = PackMesh(&length, mesh);
    std::ofstream fout(path, std::ios_base::out | std::ios_base::binary |
                                 std::ios_base::trunc);
    if (!fout.is_open()) {
        throw std::runtime_error("Cannot open file:" + path);
    }
    fout.write((char*)data, length);
    free(data);
    fout.close();
}
void Mesh::GenMeshFile(const aiMesh* ptr, const std::string& save_path) {
    size_t length;
    Byte* data = GenMeshFile(ptr, save_path, &length);
    std::ofstream fout(save_path, std::ios_base::out | std::ios_base::binary |
                                      std::ios_base::trunc);
    if (!fout.is_open()) {
        throw std::runtime_error("Cannot open file:" + save_path);
    }
    fout.write((char*)data, length);
    fout.close();
}
Byte* Mesh::GenMeshFile(const aiMesh* pointer,
                        const std::string& name,
                        size_t* ret_length) {
    MeshFile head;
    head.restart_index = UINT32_MAX;
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
                std::cout << "\n\t"
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
    head.index_type = GL_UNSIGNED_INT;
    if (pointer->HasFaces()) {
        head.index_status = IndexStatus::ONLY_INDEX;
        head.primitive_type = GL_TRIANGLES;
        head.ibo.start = sizeof(MeshFile) + head.vbo.length;
        head.ibo.length = pointer->mNumFaces * sizeof(unsigned int) * 3;
        head.mesh_count = pointer->mNumFaces;
        std::cout << "\nFaces: triangles, unsigned int * 3";
    } else {
        head.index_status = IndexStatus::NO_INDEX;
        head.primitive_type = GL_NONE;
        head.ibo.start = UINT64_MAX;
        head.ibo.length = 0ULL;
        head.mesh_count = pointer->mNumVertices;
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
    std::cout << "Vertices Count:" << pointer->mNumVertices << '\n';
    std::cout << "Indices Count:" << pointer->mNumFaces * 3 << '\n';
    std::cout << "Data size:" << rl + sizeof(uint64) << "Bytes\n";
    std::cout << "END;" << std::endl;
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
        GenMeshFile(scene->mMeshes[i], std::string(path) + std::to_string(i) +
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
void Mesh::MakeCube(Mesh& mesh, float size, VertexData df) {
    size = abs(size);
    if (df == VertexData::POSITION) {
        mesh.primitive_type = GL_TRIANGLE_STRIP;
        mesh.restart_index = UINT16_MAX;
        mesh.index_type = GL_UNSIGNED_SHORT;
        mesh.mesh_count = 17;
        mesh.TryInit(IndexStatus::RESTART_INDEX);
        size /= 2.0f;
        const float vparr[24]{
            -size, size,  size,  size,  size,  size,  -size, size,
            -size, size,  size,  -size, -size, -size, -size, size,
            -size, -size, -size, -size, size,  size,  -size, size,
        };
        const uint16 viarr[17]{0, 1, 2, 3, 4, 5, 6, 7, UINT16_MAX,
                               2, 4, 0, 6, 1, 7, 3, 5};
        glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                             GL_STATIC_READ);
        glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
        glNamedBufferStorage(mesh.vertex_array, sizeof(vparr), vparr,
                             GL_STATIC_READ);
        glVertexArrayVertexBuffer(mesh.vertex_array, 0, mesh.vertex_buffer, 0,
                                  12);
        glEnableVertexArrayAttrib(mesh.vertex_array, 0);
        glVertexArrayAttribFormat(mesh.vertex_array, 0, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribBinding(mesh.vertex_array, 0, 0);
    } else if (df == VertexData::NORMAL) {
        mesh.primitive_type = GL_TRIANGLE_STRIP;
        mesh.restart_index = UINT32_MAX;
        mesh.index_type = GL_UNSIGNED_SHORT;
        mesh.mesh_count = 17;
        mesh.TryInit(IndexStatus::RESTART_INDEX);
        size /= 2.0f;
        const float vparr[144]{
            -size, size,  size,  0.0f,  1.0f,  0.0f,  size,  size,
            size,  0.0f,  1.0f,  0.0f,  -size, size,  -size, 0.0f,
            1.0f,  0.0f,  size,  size,  -size, 0.0f,  1.0f,  0.0f,

            -size, size,  -size, 0.0f,  0.0f,  -1.0f, size,  size,
            -size, 0.0f,  0.0f,  -1.0f, -size, -size, -size, 0.0f,
            0.0f,  -1.0f, size,  -size, -size, 0.0f,  0.0f,  -1.0f,

            -size, -size, -size, 0.0f,  -1.0f, 0.0f,  size,  -size,
            -size, 0.0f,  -1.0f, 0.0f,  -size, -size, size,  0.0f,
            -1.0f, 0.0f,  size,  -size, size,  0.0f,  -1.0f, 0.0f,

            -size, size,  size,  0.0f,  0.0f,  1.0f,  size,  size,
            size,  0.0f,  0.0f,  1.0f,  -size, -size, size,  0.0f,
            0.0f,  1.0f,  size,  -size, size,  0.0f,  0.0f,  1.0f,

            -size, size,  size,  -1.0f, 0.0f,  0.0f,  -size, -size,
            size,  -1.0f, 0.0f,  0.0f,  -size, size,  -size, -1.0f,
            0.0f,  0.0f,  -size, -size, -size, -1.0f, 0.0f,  0.0f,

            size,  size,  size,  1.0f,  0.0f,  0.0f,  size,  -size,
            size,  1.0f,  0.0f,  0.0f,  size,  size,  -size, 1.0f,
            0.0f,  0.0f,  size,  -size, -size, 1.0f,  0.0f,  0.0f};
        const uint16 viarr[]{
            0,  1,  2,  3,  UINT16_MAX, 4,  5,  6,  7,  UINT16_MAX,
            8,  9,  10, 11, UINT16_MAX, 12, 13, 14, 15, UINT16_MAX,
            16, 17, 18, 19, UINT16_MAX, 20, 21, 22, 23, UINT16_MAX};
        glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                             GL_STATIC_READ);
        glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
        glNamedBufferStorage(mesh.vertex_array, sizeof(vparr), vparr,
                             GL_STATIC_READ);
        glVertexArrayVertexBuffer(mesh.vertex_array, 0, mesh.vertex_buffer, 0,
                                  24);
        glEnableVertexArrayAttrib(mesh.vertex_array, 0);
        glEnableVertexArrayAttrib(mesh.vertex_array, 1);
        glVertexArrayAttribFormat(mesh.vertex_array, 0, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribFormat(mesh.vertex_array, 1, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribBinding(mesh.vertex_array, 0, 0);
        glVertexArrayAttribBinding(mesh.vertex_array, 1, 0);
    }
}
// static void Mesh::MakePlane(Mesh& mesh,
//                             float size,
//                             int xdiv,
//                             int ydiv,
//                             VertexData df) {}
void Mesh::MakeSphere(Mesh& mesh, float r, int rdiv, int hdiv, VertexData df) {
    r = std::abs(r);
    rdiv = std::max(rdiv, 8);
    hdiv = std::max(hdiv, 8);
    if (df == VertexData::POSITION) {
        mesh.primitive_type = GL_TRIANGLE_STRIP;
        mesh.restart_index = UINT32_MAX;
        mesh.TryInit(IndexStatus::RESTART_INDEX);
        int cnt = rdiv * hdiv + 2;
        float *vparr = (float*)malloc(sizeof(float) * 3 * cnt), *cur = vparr;
        if (!vparr)
            throw std::bad_alloc();
        *(cur++) = 0.0f;
        *(cur++) = 0.0f;
        *(cur++) = r;
        *(cur++) = 0.0f;
        *(cur++) = 0.0f;
        *(cur++) = -r;
        for (int i = 0; i < rdiv; i++) {
            float t = 2 * BL_MATH_PI / rdiv * i;
            Vector3f rvec(cosf(t), sinf(t), 0.0f);
            Vector3f zvec(0.0f, 0.0f, 1.0f);
            for (int j = 1; j < hdiv + 1; j++) {
                t = -BL_MATH_PI / 2 + BL_MATH_PI / (rdiv + 1) * j;
                Vector3f pos = r * (rvec * cosf(t) + zvec * sinf(t));
                *(cur++) = pos.x();
                *(cur++) = pos.y();
                *(cur++) = pos.z();
            }
        }
        glNamedBufferStorage(mesh.vertex_array, sizeof(vparr), vparr,
                             GL_STATIC_READ);
        glVertexArrayVertexBuffer(mesh.vertex_array, 0, mesh.vertex_buffer, 0,
                                  12);
        glEnableVertexArrayAttrib(mesh.vertex_array, 0);
        glVertexArrayAttribFormat(mesh.vertex_array, 0, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribBinding(mesh.vertex_array, 0, 0);
        if (cnt < UINT16_MAX) {
            mesh.index_type = GL_UNSIGNED_SHORT;
            mesh.mesh_count =
                rdiv * (hdiv * 2 + 1) + 2 * (rdiv / 2 * 5 + rdiv % 2 * 3);
            uint16 *viarr = (uint16*)malloc(sizeof(uint16) * mesh.mesh_count),
                   *cur = viarr;
            if (!viarr)
                throw std::bad_alloc();
            for (int i = 0; i < rdiv - 1; i++) {
                for (int j = 0; j < hdiv - 1; j++) {
                    int k = 2 + j + i * hdiv;
                    *(cur++) = static_cast<uint16>(k);
                    *(cur++) = static_cast<uint16>(k + hdiv);
                    *(cur++) = static_cast<uint16>(k + 1);
                }
                int k = 2 + hdiv - 1 + (rdiv - 1) * hdiv;
                *(cur++) = static_cast<uint16>(k);
                *(cur++) = static_cast<uint16>(k + hdiv);
                *(cur++) = UINT16_MAX;
            }
            int offset = (rdiv - 1) * (hdiv * 2 + 1);
            for (int j = 0; j < hdiv - 1; j++) {
                *(cur++) = static_cast<uint16>(j + 2 + offset);
                *(cur++) = static_cast<uint16>(j + 2);
                *(cur++) = static_cast<uint16>(j + 2 + 1);
            }
            *(cur++) = static_cast<uint16>(hdiv - 1 + 2 + offset);
            *(cur++) = static_cast<uint16>(hdiv - 1 + 2);
            *(cur++) = UINT16_MAX;

            int i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint16>(2 + (i - 2) * hdiv);
                *(cur++) = 1;
                *(cur++) = static_cast<uint16>(2 + (i - 1) * hdiv);
                *(cur++) = static_cast<uint16>(2 + i * hdiv);
                *(cur++) = UINT16_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) = static_cast<uint16>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
                *(cur++) = static_cast<uint16>(2 + hdiv);
            } else {
                *(cur++) = static_cast<uint16>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
            }

            i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint16>(2 + (i - 2) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint16>(2 + (i - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint16>(2 + i * hdiv + hdiv - 1);
                *(cur++) = UINT16_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) =
                    static_cast<uint16>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint16>(2 + hdiv - 1);
                *(cur++) = static_cast<uint16>(2 + 2 * hdiv - 1);
            } else {
                *(cur++) = 0;
                *(cur++) =
                    static_cast<uint16>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint16>(2 + hdiv - 1);
            }
            glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                                 GL_STATIC_READ);
            glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
            free(viarr);
        } else {
            mesh.index_type = GL_UNSIGNED_INT;
            mesh.mesh_count =
                rdiv * (hdiv * 2 + 1) + 2 * (rdiv / 2 * 5 + rdiv % 2 * 3);
            uint32 *viarr = (uint32*)malloc(sizeof(uint32) * mesh.mesh_count),
                   *cur = viarr;
            if (!viarr)
                throw std::bad_alloc();
            for (int i = 0; i < rdiv - 1; i++) {
                for (int j = 0; j < hdiv - 1; j++) {
                    int k = 2 + j + i * hdiv;
                    *(cur++) = static_cast<uint32>(k);
                    *(cur++) = static_cast<uint32>(k + hdiv);
                    *(cur++) = static_cast<uint32>(k + 1);
                }
                int k = 2 + hdiv - 1 + (rdiv - 1) * hdiv;
                *(cur++) = static_cast<uint32>(k);
                *(cur++) = static_cast<uint32>(k + hdiv);
                *(cur++) = UINT32_MAX;
            }
            int offset = (rdiv - 1) * (hdiv * 2 + 1);
            for (int j = 0; j < hdiv - 1; j++) {
                *(cur++) = static_cast<uint32>(j + 2 + offset);
                *(cur++) = static_cast<uint32>(j + 2);
                *(cur++) = static_cast<uint32>(j + 2 + 1);
            }
            *(cur++) = static_cast<uint32>(hdiv - 1 + 2 + offset);
            *(cur++) = static_cast<uint32>(hdiv - 1 + 2);
            *(cur++) = UINT32_MAX;

            int i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint32>(2 + (i - 2) * hdiv);
                *(cur++) = 1;
                *(cur++) = static_cast<uint32>(2 + (i - 1) * hdiv);
                *(cur++) = static_cast<uint32>(2 + i * hdiv);
                *(cur++) = UINT32_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) = static_cast<uint32>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
                *(cur++) = static_cast<uint32>(2 + hdiv);
            } else {
                *(cur++) = static_cast<uint32>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
            }

            i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint32>(2 + (i - 2) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint32>(2 + (i - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint32>(2 + i * hdiv + hdiv - 1);
                *(cur++) = UINT32_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) =
                    static_cast<uint32>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint32>(2 + hdiv - 1);
                *(cur++) = static_cast<uint32>(2 + 2 * hdiv - 1);
            } else {
                *(cur++) = 0;
                *(cur++) =
                    static_cast<uint32>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint32>(2 + hdiv - 1);
            }
            glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                                 GL_STATIC_READ);
            glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
            free(viarr);
        }
        free(vparr);
    } else if (df == VertexData::NORMAL) {
        mesh.primitive_type = GL_TRIANGLE_STRIP;
        mesh.restart_index = UINT32_MAX;
        mesh.TryInit(IndexStatus::RESTART_INDEX);
        int cnt = rdiv * hdiv + 2;
        float *vparr = (float*)malloc(sizeof(float) * 3 * 2 * cnt),
              *cur = vparr;
        if (!vparr)
            throw std::bad_alloc();
        *(cur++) = 0.0f;
        *(cur++) = 0.0f;
        *(cur++) = r;
        *(cur++) = 0.0f;
        *(cur++) = 0.0f;
        *(cur++) = -r;
        for (int i = 0; i < rdiv; i++) {
            float t = 2 * BL_MATH_PI / rdiv * i;
            Vector3f rvec(cosf(t), sinf(t), 0.0f);
            Vector3f zvec(0.0f, 0.0f, 1.0f);
            for (int j = 1; j < hdiv + 1; j++) {
                t = -BL_MATH_PI / 2 + BL_MATH_PI / (rdiv + 1) * j;
                Vector3f pos = rvec * cosf(t) + zvec * sinf(t);
                cur += 3;
                *(cur++) = pos.x();
                *(cur++) = pos.y();
                *(cur++) = pos.z();
                cur -= 6;
                pos *= r;
                *(cur++) = pos.x();
                *(cur++) = pos.y();
                *(cur++) = pos.z();
            }
        }
        glNamedBufferStorage(mesh.vertex_array, sizeof(vparr), vparr,
                             GL_STATIC_READ);
        glVertexArrayVertexBuffer(mesh.vertex_array, 0, mesh.vertex_buffer, 0,
                                  12);
        glEnableVertexArrayAttrib(mesh.vertex_array, 0);
        glVertexArrayAttribFormat(mesh.vertex_array, 0, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribBinding(mesh.vertex_array, 0, 0);
        if (cnt < UINT16_MAX) {
            mesh.index_type = GL_UNSIGNED_SHORT;
            mesh.mesh_count =
                rdiv * (hdiv * 2 + 1) + 2 * (rdiv / 2 * 5 + rdiv % 2 * 3);
            uint16 *viarr = (uint16*)malloc(sizeof(uint16) * mesh.mesh_count),
                   *cur = viarr;
            if (!viarr)
                throw std::bad_alloc();
            for (int i = 0; i < rdiv - 1; i++) {
                for (int j = 0; j < hdiv - 1; j++) {
                    int k = 2 + j + i * hdiv;
                    *(cur++) = static_cast<uint16>(k);
                    *(cur++) = static_cast<uint16>(k + hdiv);
                    *(cur++) = static_cast<uint16>(k + 1);
                }
                int k = 2 + hdiv - 1 + (rdiv - 1) * hdiv;
                *(cur++) = static_cast<uint16>(k);
                *(cur++) = static_cast<uint16>(k + hdiv);
                *(cur++) = UINT16_MAX;
            }
            int offset = (rdiv - 1) * (hdiv * 2 + 1);
            for (int j = 0; j < hdiv - 1; j++) {
                *(cur++) = static_cast<uint16>(j + 2 + offset);
                *(cur++) = static_cast<uint16>(j + 2);
                *(cur++) = static_cast<uint16>(j + 2 + 1);
            }
            *(cur++) = static_cast<uint16>(hdiv - 1 + 2 + offset);
            *(cur++) = static_cast<uint16>(hdiv - 1 + 2);
            *(cur++) = UINT16_MAX;

            int i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint16>(2 + (i - 2) * hdiv);
                *(cur++) = 1;
                *(cur++) = static_cast<uint16>(2 + (i - 1) * hdiv);
                *(cur++) = static_cast<uint16>(2 + i * hdiv);
                *(cur++) = UINT16_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) = static_cast<uint16>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
                *(cur++) = static_cast<uint16>(2 + hdiv);
            } else {
                *(cur++) = static_cast<uint16>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
            }

            i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint16>(2 + (i - 2) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint16>(2 + (i - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint16>(2 + i * hdiv + hdiv - 1);
                *(cur++) = UINT16_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) =
                    static_cast<uint16>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint16>(2 + hdiv - 1);
                *(cur++) = static_cast<uint16>(2 + 2 * hdiv - 1);
            } else {
                *(cur++) = 0;
                *(cur++) =
                    static_cast<uint16>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint16>(2 + hdiv - 1);
            }
            glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                                 GL_STATIC_READ);
            glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
            free(viarr);
        } else {
            mesh.index_type = GL_UNSIGNED_INT;
            uint32 *viarr = (uint32*)malloc(
                       sizeof(uint32) * (rdiv * (hdiv * 2 + 1) +
                                         2 * (rdiv / 2 * 5 + rdiv % 2 * 3))),
                   *cur = viarr;
            if (!viarr)
                throw std::bad_alloc();
            for (int i = 0; i < rdiv - 1; i++) {
                for (int j = 0; j < hdiv - 1; j++) {
                    int k = 2 + j + i * hdiv;
                    *(cur++) = static_cast<uint32>(k);
                    *(cur++) = static_cast<uint32>(k + hdiv);
                    *(cur++) = static_cast<uint32>(k + 1);
                }
                int k = 2 + hdiv - 1 + (rdiv - 1) * hdiv;
                *(cur++) = static_cast<uint32>(k);
                *(cur++) = static_cast<uint32>(k + hdiv);
                *(cur++) = UINT32_MAX;
            }
            int offset = (rdiv - 1) * (hdiv * 2 + 1);
            for (int j = 0; j < hdiv - 1; j++) {
                *(cur++) = static_cast<uint32>(j + 2 + offset);
                *(cur++) = static_cast<uint32>(j + 2);
                *(cur++) = static_cast<uint32>(j + 2 + 1);
            }
            *(cur++) = static_cast<uint32>(hdiv - 1 + 2 + offset);
            *(cur++) = static_cast<uint32>(hdiv - 1 + 2);
            *(cur++) = UINT32_MAX;

            int i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint32>(2 + (i - 2) * hdiv);
                *(cur++) = 1;
                *(cur++) = static_cast<uint32>(2 + (i - 1) * hdiv);
                *(cur++) = static_cast<uint32>(2 + i * hdiv);
                *(cur++) = UINT32_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) = static_cast<uint32>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
                *(cur++) = static_cast<uint32>(2 + hdiv);
            } else {
                *(cur++) = static_cast<uint32>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
            }

            i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint32>(2 + (i - 2) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint32>(2 + (i - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint32>(2 + i * hdiv + hdiv - 1);
                *(cur++) = UINT32_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) =
                    static_cast<uint32>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint32>(2 + hdiv - 1);
                *(cur++) = static_cast<uint32>(2 + 2 * hdiv - 1);
            } else {
                *(cur++) = 0;
                *(cur++) =
                    static_cast<uint32>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint32>(2 + hdiv - 1);
            }
            glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                                 GL_STATIC_READ);
            glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
            free(viarr);
        }
        free(vparr);
    }
}
// void Mesh::MakeTorus(Mesh& mesh,
//                             float r,
//                             float d,
//                             int rdiv,
//                             int hdiv,
//                             VertexData df) {}
void Mesh::MakeCubord(Mesh& mesh, float a, float b, float c, VertexData df) {
    if (df == VertexData::POSITION) {
        mesh.primitive_type = GL_TRIANGLE_STRIP;
        mesh.restart_index = UINT16_MAX;
        mesh.index_type = GL_UNSIGNED_SHORT;
        mesh.mesh_count = 17;
        mesh.TryInit(IndexStatus::RESTART_INDEX);
        a /= 2.0f;
        b /= 2.0f;
        c /= 2.0f;
        const float vparr[24]{
            -a, b,  c,  a, b,  c,  -a, b,  -c, a, b,  -c,
            -a, -b, -c, a, -b, -c, -a, -b, c,  a, -b, c,
        };
        const uint16 viarr[17]{0, 1, 2, 3, 4, 5, 6, 7, UINT16_MAX,
                               2, 4, 0, 6, 1, 7, 3, 5};
        glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                             GL_STATIC_READ);
        glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
        glNamedBufferStorage(mesh.vertex_array, sizeof(vparr), vparr,
                             GL_STATIC_READ);
        glVertexArrayVertexBuffer(mesh.vertex_array, 0, mesh.vertex_buffer, 0,
                                  12);
        glEnableVertexArrayAttrib(mesh.vertex_array, 0);
        glVertexArrayAttribFormat(mesh.vertex_array, 0, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribBinding(mesh.vertex_array, 0, 0);
    } else if (df == VertexData::NORMAL) {
        mesh.primitive_type = GL_TRIANGLE_STRIP;
        mesh.restart_index = UINT16_MAX;
        mesh.index_type = GL_UNSIGNED_SHORT;
        mesh.mesh_count = 17;
        mesh.TryInit(IndexStatus::RESTART_INDEX);
        a /= 2.0f;
        b /= 2.0f;
        c /= 2.0f;
        const float vparr[144]{
            -a, b,  c,  0.0f,  1.0f,  0.0f,  a,  b,  c,  0.0f,  1.0f,  0.0f,
            -a, b,  -c, 0.0f,  1.0f,  0.0f,  a,  b,  -c, 0.0f,  1.0f,  0.0f,

            -a, b,  -c, 0.0f,  0.0f,  -1.0f, a,  b,  -c, 0.0f,  0.0f,  -1.0f,
            -a, -b, -c, 0.0f,  0.0f,  -1.0f, a,  -b, -c, 0.0f,  0.0f,  -1.0f,

            -a, -b, -c, 0.0f,  -1.0f, 0.0f,  a,  -b, -c, 0.0f,  -1.0f, 0.0f,
            -a, -b, c,  0.0f,  -1.0f, 0.0f,  a,  -b, c,  0.0f,  -1.0f, 0.0f,

            -a, b,  c,  0.0f,  0.0f,  1.0f,  a,  b,  c,  0.0f,  0.0f,  1.0f,
            -a, -b, c,  0.0f,  0.0f,  1.0f,  a,  -b, c,  0.0f,  0.0f,  1.0f,

            -a, b,  c,  -1.0f, 0.0f,  0.0f,  -a, -b, c,  -1.0f, 0.0f,  0.0f,
            -a, b,  -c, -1.0f, 0.0f,  0.0f,  -a, -b, -c, -1.0f, 0.0f,  0.0f,

            a,  b,  c,  1.0f,  0.0f,  0.0f,  a,  -b, c,  1.0f,  0.0f,  0.0f,
            a,  b,  -c, 1.0f,  0.0f,  0.0f,  a,  -b, -c, 1.0f,  0.0f,  0.0f};
        const uint16 viarr[]{
            0,  1,  2,  3,  UINT16_MAX, 4,  5,  6,  7,  UINT16_MAX,
            8,  9,  10, 11, UINT16_MAX, 12, 13, 14, 15, UINT16_MAX,
            16, 17, 18, 19, UINT16_MAX, 20, 21, 22, 23, UINT16_MAX};
        glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                             GL_STATIC_READ);
        glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
        glNamedBufferStorage(mesh.vertex_array, sizeof(vparr), vparr,
                             GL_STATIC_READ);
        glVertexArrayVertexBuffer(mesh.vertex_array, 0, mesh.vertex_buffer, 0,
                                  24);
        glEnableVertexArrayAttrib(mesh.vertex_array, 0);
        glEnableVertexArrayAttrib(mesh.vertex_array, 1);
        glVertexArrayAttribFormat(mesh.vertex_array, 0, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribFormat(mesh.vertex_array, 1, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribBinding(mesh.vertex_array, 0, 0);
        glVertexArrayAttribBinding(mesh.vertex_array, 1, 0);
    }
}
Mesh::~Mesh() {
    glDeleteVertexArrays(1, &vertex_array);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(buffers.size(), &buffers[0]);
    if (index_status != IndexStatus::NO_INDEX) {
        glDeleteBuffers(1, &index_buffer);
    }
}
Texture::Texture() {
    texture_id = 0;
}
Texture::~Texture() {
    glDeleteTextures(1, &texture_id);
}

void Texture::LoadTexture(const std::string& path,
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
    size_t length, end;
    length = in.tellg();
    in.seekg(0, std::ios::end);
    end = in.tellg();
    length = end - length;
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
                             (GLint*)tf.swizzle);
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
                                tf.mip[i].height, tf.format, tf.type,
                                data + tf.mip[i].range.start);
        }
    } else if (tf.target == GL_TEXTURE_3D) {
        glTextureStorage3D(tex.texture_id, tf.mipLevels + add_mipmap_level,
                           tf.internal_format, tf.mip[0].width,
                           tf.mip[0].height, tf.mip[0].depth);
        for (GLsizei i = 0; i < tf.mipLevels; i++) {
            glTextureSubImage3D(tex.texture_id, i, 0, 0, 0, tf.mip[i].width,
                                tf.mip[i].height, tf.mip[i].depth, tf.format,
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
inline void Texture::LoadTexture(const char* path,
                                 Texture& tex,
                                 GLsizei add_mipmap_level,
                                 GLsizei samples,
                                 GLboolean fixedsample) {
    LoadTexture(std::string(path), tex, add_mipmap_level, samples, fixedsample);
}
void Texture::PackTexture(const std::string& save_path,
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
    if (type == GL_R3_G3_B2 || type == GL_R8 || type == GL_R8_SNORM ||
        type == GL_R8I || type == GL_R8UI || type == GL_RGBA2) {
        return 1;
    } else if (type == GL_R16 || type == GL_R16_SNORM || type == GL_R16F ||
               type == GL_R16I || type == GL_R16UI || type == GL_RG16 ||
               type == GL_RG8 || type == GL_RG8_SNORM || type == GL_RG8I ||
               type == GL_RG8UI || type == GL_RGB5_A1 || type == GL_RGB565 ||
               type == GL_RGBA4) {
        return 2;
    } else if (type == GL_RGB8 || type == GL_RGB8_SNORM || type == GL_RGB8I ||
               type == GL_RGB8UI || type == GL_SRGB8) {
        return 3;
    } else if (type == GL_R11F_G11F_B10F || type == GL_R32F ||
               type == GL_R32I || type == GL_R32UI || type == GL_RG16_SNORM ||
               type == GL_RG16F || type == GL_RG16I || type == GL_RG16UI ||
               type == GL_RGB10_A2 || type == GL_RGB9_E5 || type == GL_RGBA8 ||
               type == GL_RGBA8_SNORM || type == GL_RGBA8I ||
               type == GL_RGBA8UI || type == GL_SRGB8_ALPHA8 ||
               type == GL_RGB16) {
        return 4;
    } else if (type == GL_RGB16_SNORM || type == GL_RGB16F ||
               type == GL_RGB16I || type == GL_RGB16UI || type == GL_RGBA12) {
        return 6;
    } else if (type == GL_RG32F || type == GL_RG32I || type == GL_RG32UI ||
               type == GL_RGBA16 || type == GL_RGBA16_SNORM ||
               type == GL_RGBA16F || type == GL_RGBA16I || type == GL_RGBA16I ||
               type == GL_RGBA16UI) {
        return 8;
    } else if (type == GL_RGB32F || type == GL_RGB32I || type == GL_RGB32UI) {
        return 12;
    } else if (type == GL_RGBA32F || type == GL_RGBA32I ||
               type == GL_RGBA32UI) {
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
Byte* Texture::PackTexture(size_t* ret_length,
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
            maxlen = mips[i].range.length;
        }
        length += mips[i].range.length;
    }
    Byte *data = (Byte*)malloc(length), *cur = data;
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
        memcpy(cur, map, mips[i].range.length);
        glUnmapNamedBuffer(ppb);
        head.mip[i].range.start = cur - data;
        cur += head.mip[i].range.length;
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glDeleteBuffers(1, &ppb);
    // 生成TextureFile头数据
    memcpy(head.mip, mips, sizeof(TextureMipData) * level);
    head.target = tex.target;
    glGetTextureLevelParameteriv(tex.texture_id, 0, GL_TEXTURE_INTERNAL_FORMAT,
                                 (GLint*)&head.internal_format);
    head.format = format;
    head.type = type;
    glGetTextureParameterIuiv(tex.texture_id, GL_TEXTURE_SWIZZLE_RGBA,
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
    *ret_length = length;
    free(data);
    *(uint64*)compress = TEXTURE_HEADER;
    return compress;
}
inline void Texture::PackTexture(const char* save_path,
                                 Texture& tex,
                                 GLsizei level,
                                 GLenum format,
                                 GLenum type) {
    PackTexture(std::string(save_path), tex, level, format, type);
}
inline void Texture::GenTextureFile(const std::string& path) {
    TextureFile<1> tf;
    tf.target = GL_TEXTURE_2D;
    tf.type = GL_UNSIGNED_BYTE;
    tf.enable_swizzle = GL_FALSE;
    tf.mipLevels = 1;
    tf.slices = 0;
    int n;
    Byte *data = (Byte*)stbi_load(path.c_str(), &tf.mip[0].width,
                                  &tf.mip[0].height, &n, 0),
         *copy;
    if (data == nullptr) {
        throw std::runtime_error("STB_IMAGE:无法加载图像");
    }
    tf.totalSize = tf.mip[0].range.length =
        tf.mip[0].width * tf.mip[0].height * n * sizeof(uint8_t);
    tf.mip[0].depth = 0;
    tf.mip[0].range.start = sizeof(tf);
    std::cout << "File:\t" << path << '\n';
    std::cout << "Image Data Size:\t" << tf.totalSize << "Bytes\n";
    switch (n) {
        case 1:
            tf.internal_format = GL_R8;
            tf.format = GL_RED;
            tf.swizzle[0] = GL_RED;
            tf.swizzle[1] = GL_ZERO;
            tf.swizzle[2] = GL_ZERO;
            tf.swizzle[3] = GL_ZERO;
            break;
        case 2:
            tf.internal_format = GL_RG8;
            tf.format = GL_RG;
            tf.swizzle[0] = GL_RED;
            tf.swizzle[1] = GL_GREEN;
            tf.swizzle[2] = GL_ZERO;
            tf.swizzle[3] = GL_ZERO;
            break;
        case 3:
            tf.internal_format = GL_RGB8;
            tf.format = GL_RGB;
            tf.swizzle[0] = GL_RED;
            tf.swizzle[1] = GL_GREEN;
            tf.swizzle[2] = GL_BLUE;
            tf.swizzle[3] = GL_ZERO;
            break;
        case 4:
            tf.internal_format = GL_RGBA8;
            tf.format = GL_RGBA;
            tf.swizzle[0] = GL_RED;
            tf.swizzle[1] = GL_GREEN;
            tf.swizzle[2] = GL_BLUE;
            tf.swizzle[3] = GL_ALPHA;
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
    GenTextureFile(std::string(path));
}
inline Program::Program() {
    program_id = glCreateProgram();
}
inline Program::~Program() {
    glDeleteProgram(program_id);
}
inline Program::ShaderInfo& Program::operator[](size_t index) {
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
void Program::AddShaderByCode(std::string_view data, GLenum type) {
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

GLuint Program::LoadShader(std::string_view path, GLenum type) {
    std::ifstream reader(std::string(path), std::ios::in);
    if (!reader.is_open()) {
        throw std::runtime_error("Cannot open file.");
    }
    std::stringstream buffer;
    buffer << reader.rdbuf();
    std::string shader_code(buffer.str());
    reader.close();
    GLuint shader_id = glCreateShader(type);
    GLint success;
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
inline GLuint Program::LoadShader(const char* path, GLenum type) {
    return LoadShader(std::string_view(path), type);
}
GLuint Program::ComplieShader(std::string_view code, GLenum type) {
    GLuint shader_id = glCreateShader(type);
    GLint success;
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
        return 0;
    }
    return shader_id;
}

const Matrix4f& Transform::get_model() {
    if (edited) {
        model = Matrix4f::Zero();
        model(0, 0) = static_cast<float>(scale.x());
        model(1, 1) = static_cast<float>(scale.y());
        model(2, 2) = static_cast<float>(scale.z());
        model(3, 3) = 1.0f;
        rotate.normalize();
        Matrix4f tmp;
        Eigen::Matrix3f rm = rotate.toRotationMatrix().cast<float>();
        tmp << rm(0, 0), rm(0, 1), rm(0, 2), 0.0f, rm(1, 0), rm(1, 1), rm(1, 2),
            0.0f, rm(2, 0), rm(2, 1), rm(2, 2), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f;
        model = tmp * model;
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
        forword.normalize();
        Vector3d s = forword.cross(up);
        s.normalize();
        Vector3d u = forword.cross(s);
        u.normalize();
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
        forword.normalize();
        Vector3d s = forword.cross(up);
        s.normalize();
        Vector3d u = forword.cross(s);
        u.normalize();
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
Renderer::Renderer() : tr_pool(1), ro_pool(1), transform_head(nullptr) {}
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
    // const Matrix4f& view = camera.get_view();
    Transform *cur_root = transform_head, *p, *tp;
    Vector3f eye_dir = camera.forword.cast<float>();
    while (!cur_root) {
        if (cur_root->enable) {
            mat_stack.push(cur_root->get_model());
            if (cur_root->roenble)
                cur_root->render_obj->draw(
                    vp * mat_stack.top(), mat_stack.top(),
                    mat_stack.top().inverse().transpose(), eye_dir);
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
                    if (!p->enable) {
                        continue;
                    }
                    mat_stack.push(p->get_model() * mat_stack.top());
                    if (cur_root->roenble)
                        cur_root->render_obj->draw(
                            vp * mat_stack.top(), mat_stack.top(),
                            mat_stack.top().inverse().transpose(), eye_dir);
                    tp = p->child_head;
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
        }
        cur_root = cur_root->next_brother;
    }
}
Renderer::~Renderer() {
    Transform *cur_root = transform_head, *p, *tp;
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
                tp = p->child_head;
                while (!tp) {
                    draw_ptrstack.push(tp);
                    tp = tp->next_brother;
                }
            } while (!draw_ptrstack.empty());
        }
        cur_root = cur_root->next_brother;
    }
}

const GLuint ads_vertpos_attrib = 4;
const GLuint ads_vertnormal_attrib = 5;
const GLuint ads_mvpmatrix_uniform = 0;
const GLuint ads_modelmatrix_uniform = 1;
const GLuint ads_normalmatrix_uniform = 2;
const GLuint ads_vertcolor_uniform = 3;
const GLuint ads_eyedirection_uniform = 6;
const GLuint ads_materialindex_uniform = 7;
const char* ads_vertshader_path = ".\\shader\\classic_shader_vertex.glsl";
const char* ads_fragshader_path = ".\\shader\\classic_shader_fragment.glsl";
const GLsizei ads_stride_array[1]{24};

Program shader;
GLuint uniform_buffer;

LightProp lightdata[NUM_MAX_LIGHTS];
MaterialProp materialdata[NUM_MAX_MATERIALS];

void MeshFunctions::InitMeshADS(Mesh& mesh) {
    mesh.InitMesh(ads_stride_array);
    glEnableVertexArrayAttrib(mesh.GetVAO(), ads_vertpos_attrib);
    glEnableVertexArrayAttrib(mesh.GetVAO(), ads_vertnormal_attrib);
    glVertexArrayAttribFormat(mesh.GetVAO(), ads_vertpos_attrib, 3, GL_FLOAT,
                              GL_FALSE, 0);
    glVertexArrayAttribFormat(mesh.GetVAO(), ads_vertnormal_attrib, 3, GL_FLOAT,
                              GL_FALSE, 12);
    glVertexArrayAttribBinding(mesh.GetVAO(), ads_vertpos_attrib, 0);
    glVertexArrayAttribBinding(mesh.GetVAO(), ads_vertnormal_attrib, 0);
}
void ADSBase::Init() {
    glCreateBuffers(1, &uniform_buffer);
    glNamedBufferStorage(uniform_buffer,
                         NUM_MAX_LIGHTS * 16 * 7 + NUM_MAX_MATERIALS * 16 * 4,
                         nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);
    shader.AddShader(ads_vertshader_path, GL_VERTEX_SHADER);
    shader.AddShader(ads_fragshader_path, GL_FRAGMENT_SHADER);
    shader.Link();
}
void ADSBase::UpdateUniformBuffer() {
    for (size_t i = 0; i < NUM_MAX_LIGHTS; i++) {
        if (lightdata[i].edited) {
            glNamedBufferSubData(uniform_buffer, 0 + 16 * 7 * i,
                                 sizeof(Vector3f), &lightdata[i].ambient);
            glNamedBufferSubData(uniform_buffer, 16 + 16 * 7 * i,
                                 sizeof(Vector3f), &lightdata[i].color);
            glNamedBufferSubData(uniform_buffer, 32 + 16 * 7 * i,
                                 sizeof(Vector3f), &lightdata[i].position);
            glNamedBufferSubData(uniform_buffer, 48 + 16 * 7 * i,
                                 sizeof(Vector3f), &lightdata[i].halfVector);
            glNamedBufferSubData(uniform_buffer, 64 + 16 * 7 * i,
                                 sizeof(Vector3f), &lightdata[i].coneDirection);
            glNamedBufferSubData(uniform_buffer, 76 + 16 * 7 * i,
                                 sizeof(Vector3f), &lightdata[i].enable);
            glNamedBufferSubData(uniform_buffer, 77 + 16 * 7 * i,
                                 sizeof(Vector3f), &lightdata[i].isLocal);
            glNamedBufferSubData(uniform_buffer, 78 + 16 * 7 * i,
                                 sizeof(Vector3f), &lightdata[i].isSpot);
            glNamedBufferSubData(uniform_buffer, 80 + 16 * 7 * i, sizeof(float),
                                 &lightdata[i].spotCosCutoff);
            glNamedBufferSubData(uniform_buffer, 84 + 16 * 7 * i, sizeof(float),
                                 &lightdata[i].spotExponent);
            glNamedBufferSubData(uniform_buffer, 88 + 16 * 7 * i, sizeof(float),
                                 &lightdata[i].constantAttenuation);
            glNamedBufferSubData(uniform_buffer, 92 + 16 * 7 * i, sizeof(float),
                                 &lightdata[i].linearAttenuation);
            glNamedBufferSubData(uniform_buffer, 96 + 16 * 7 * i, sizeof(float),
                                 &lightdata[i].quadraticAttenuation);
        }
    }
    for (size_t i = 0; i < NUM_MAX_MATERIALS; i++) {
        if (materialdata[i].edited) {
            glNamedBufferSubData(uniform_buffer,
                                 0 + 16 * 7 * NUM_MAX_LIGHTS + 16 * 4 * i,
                                 sizeof(Vector3f), &materialdata[i].emission);
            glNamedBufferSubData(uniform_buffer,
                                 16 + 16 * 7 * NUM_MAX_LIGHTS + 16 * 4 * i,
                                 sizeof(Vector3f), &materialdata[i].ambient);
            glNamedBufferSubData(uniform_buffer,
                                 32 + 16 * 7 * NUM_MAX_LIGHTS + 16 * 4 * i,
                                 sizeof(Vector3f), &materialdata[i].diffuse);
            glNamedBufferSubData(uniform_buffer,
                                 48 + 16 * 7 * NUM_MAX_LIGHTS + 16 * 4 * i,
                                 sizeof(Vector3f), &materialdata[i].specular);
            glNamedBufferSubData(uniform_buffer,
                                 60 + 16 * 7 * NUM_MAX_LIGHTS + 16 * 4 * i,
                                 sizeof(float), &materialdata[i].shininess);
        }
    }
}
void ADSRender::draw(const Matrix4f& mvp_matrix,
                     const Matrix4f& model_matrix,
                     const Matrix4f& normal_matrix,
                     const Vector3f& eye_dir) {
    glBindVertexArray(mesh.GetVAO());
    Program::UseProgram(shader);
    glProgramUniform1i(shader.GetID(), ads_materialindex_uniform,
                       ((ADSData*)data_ptr)->materialindex);
    glProgramUniform4fv(shader.GetID(), ads_vertcolor_uniform, 1,
                        &(((ADSData*)data_ptr)->vertexcolor.x()));
    glProgramUniformMatrix4fv(shader.GetID(), ads_mvpmatrix_uniform, 1,
                              GL_FALSE, &mvp_matrix(0, 0));
    glProgramUniformMatrix4fv(shader.GetID(), ads_modelmatrix_uniform,
                              1, GL_FALSE, &model_matrix(0, 0));
    glProgramUniformMatrix4fv(shader.GetID(), ads_normalmatrix_uniform,
                              1, GL_FALSE, &normal_matrix(0, 0));
    glProgramUniform3fv(shader.GetID(), ads_eyedirection_uniform, 1,
                        &eye_dir.x());
    if (mesh.GetIndexStatus() == IndexStatus::NO_INDEX) {
        glDrawArrays(mesh.GetPrimitiveType(), 0, mesh.GetCount());
    } else if (mesh.GetIndexStatus() == IndexStatus::ONLY_INDEX) {
        glDrawElements(mesh.GetPrimitiveType(), mesh.GetCount(),
                       mesh.GetIndexType(), 0);
    } else if (mesh.GetIndexStatus() == IndexStatus::RESTART_INDEX) {
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(mesh.GetRestartIndex());
        glDrawElements(mesh.GetPrimitiveType(), mesh.GetCount(),
                       mesh.GetIndexType(), 0);
        glDisable(GL_PRIMITIVE_RESTART);
    }
}
ADSRender::~ADSRender() {}
}  // namespace Boundless
