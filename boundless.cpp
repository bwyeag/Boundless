#include "boundless.hpp"

namespace Boundless
{
    Byte *CompressData(const Byte *data, size_t *length, size_t space)
    {
        size_t dlen = zlib::compressBound(*length) + sizeof(uint64) * 2;
        Byte *compress_data = (Byte *)malloc(space + dlen);
        if (compress_data == nullptr)
        {
            throw std::bad_alloc();
        }
        Byte *data_ptr = compress_data + space;
        *(uint64 *)data_ptr = *length;
        data_ptr += sizeof(uint64) * 2;
        *(uint64 *)(data_ptr - sizeof(uint64)) = dlen - sizeof(uint64) * 2;
        int res = zlib::compress2(data_ptr, data_ptr - sizeof(uint64), data, *length, compress_level);
        if (res != Z_OK)
        {
            free(compress_data);
            throw zlib::ZlibException(res);
        }
        return compress_data;
    }
    Byte *UncompressData(const Byte *data, size_t *ret_length)
    {
        Byte *uncompress_data = (Byte *)malloc(*(uint64 *)data);
        if (uncompress_data == nullptr)
        {
            throw std::bad_alloc();
        }
        uint64 dlen = *(uint64 *)data;
        int res = zlib::uncompress2(uncompress_data, &dlen, data + sizeof(uint64) * 2, *(uint64 *)(data + sizeof(uint64)));
        if (res != Z_OK)
        {
            free(uncompress_data);
            throw zlib::ZlibException(res);
        }
        return uncompress_data;
    }

    Mesh::Mesh() {}
    Mesh::Mesh(IndexStatus indexst, size_t bufcnt)
    {
        glCreateVertexArrays(1, &vertex_array);
        glCreateBuffers(1, &vertex_buffer);
        buffers.resize(bufcnt);
        glCreateBuffers(bufcnt, &buffers[0]);
        index_status = indexst;
        if (indexst > 31)
        {
            glCreateBuffers(1, &index_buffer);
        }
    }
    inline const std::vector<GLuint> &Mesh::GetBuffer() { return buffers; }
    inline const std::vector<GLuint> &Mesh::GetTexture() { return textures; }
    inline void Mesh::SetPrimitiveType(GLenum type) { primitive_type = type; }
    inline void Mesh::SetRestartIndex(GLuint index) { restart_index = index; }
    inline GLuint Mesh::GetVAO() { return vertex_array; }
    inline GLuint Mesh::GetVBO() { return vertex_buffer; }
    inline GLuint Mesh::GetIBO() { return index_buffer; }
    void LoadMesh(const Byte *data, size_t length, Mesh &mesh)
    {
        if (*(uint64 *)data != 0xF241282943FF0001)
        {
            throw std::runtime_error("Mesh head code error.");
        }
        size_t len;
        data = UncompressData(data + sizeof(uint64), &len);
        MeshFile &head = *static_cast<MeshFile *>(data);
        Byte *dp = data + sizeof(MeshFile) + sizeof(DataRange) * head.buffer_count;
        mesh.primitive_type = head.primitive_type;
        mesh.index_status = head.index_status;
        mesh.restart_index = head.restart_index;
        glCreateVertexArrays(1, &mesh.vertex_array);
        glCreateBuffers(1, &mesh.vertex_buffer);
        mesh.buffers.resize(head.buffer_count);
        glCreateBuffers(head.buffer_count, &mesh.buffers[0]);
        glNamedBufferStorage(mesh.vertex_buffer, head.vbo.length, data + head.vbo.start, GL_MAP_READ_BIT);
        if (mesh.index_status > 31)
        {
            glCreateBuffers(1, &mesh.index_buffer);
            glNamedBufferStorage(mesh.index_buffer, head.ibo.length, data + head.ibo.start, GL_MAP_READ_BIT);
        }
        for (size_t i = 0; i < head.buffer_count; i++)
        {
            glNamedBufferStorage(mesh.buffers[i], head.buffers[i].length, data + head.buffers[i].start, GL_MAP_READ_BIT);
        }
        free(data);
    }
    void LoadMesh(std::ifstream &in, Mesh &mesh)
    {
        size_t length;
        std::streampos cur = in.tellg();
        in.seekg(sizeof(uint64), std::ios::cur);
        in.read((char *)&length, sizeof(uint64));
        in.seekg(cur);
        Byte *data = (Byte *)malloc(length);
        if (data == nullptr)
        {
            throw std::bad_alloc();
        }
        in.read((char *)data, length);
        LoadMesh(data, length, mesh);
        free(data);
    }
    inline void LoadMesh(const std::string &path, Mesh &mesh)
    {
        std::ifstream fin(path, std::ios::in | std::ios::binary);
        if (!fin.is_open())
        {
            throw std::runtime_error("Cannot open file:" + path);
        }
        LoadMesh(fin, mesh);
        fin.close();
    }
    inline void LoadMesh(const char *path, Mesh &mesh)
    {
        LoadMesh(std::string(path), mesh);
    }
    void LoadMeshMultple(const std::string &path, std::vector<Mesh> &meshs)
    {
        std::ifstream fin(path, std::ios::in | std::ios::binary);
        if (!fin.is_open())
        {
            throw std::runtime_error("Cannot open file:" + path);
        }
        uint64 headcode;
        fin.read((char *)&headcode, sizeof(uint64));
        if (headcode != 0xF242191756FF0003)
        {
            throw std::runtime_error("Mesh head code error.");
        }
        uint32 mesh_count;
        fin.read((char *)&mesh_count, sizeof(uint32));
        meshs.reserve(mesh_count);
        for (Mesh &m : meshs)
        {
            LoadMesh(fin, m);
        }
        fin.close();
    }
    void LoadMeshMultple(const char *path, std::vector<Mesh> &meshs)
    {
        LoadMeshMultple(std::string(path), meshs);
    }
    Byte *PackMesh(size_t *ret_length, const Mesh &mesh)
    {
        size_t full_size = sizeof(MeshFile) + sizeof(DataRange) * mesh.buffers.size();
        GLint64 length;
        glGetNamedBufferParameteri64v(mesh.vertex_buffer, GL_BUFFER_SIZE, &length);
        full_size += static_cast<size_t>(length);
        if (mesh.index_status > 31)
        {
            glGetNamedBufferParameteri64v(mesh.index_buffer, GL_BUFFER_SIZE, &length);
            full_size += static_cast<size_t>(length);
        }
        for (size_t i = 0; i < mesh.buffers.size(); i++)
        {
            glGetNamedBufferParameteri64v(mesh.buffers[i], GL_BUFFER_SIZE, &length);
            full_size += static_cast<size_t>(length);
        }
        Byte *data = (Byte *)malloc(full_size), *cur = data;
        if (data == nullptr)
        {
            throw std::bad_alloc();
        }
        MeshFile &head = *(MeshFile *)cur;
        cur += sizeof(MeshFile) + sizeof(DataRange) * mesh.buffers;
        head.primitive_type = mesh.primitive_type;
        head.index_status = mesh.index_status;
        head.restart_index = mesh.restart_index;
        head.buffer_count = mesh.buffers.size();

        Byte *ptr;
        glGetNamedBufferParameteri64v(mesh.vertex_buffer, GL_BUFFER_SIZE, &head.vbo.length);
        head.vbo.start = cur - data;
        cur += head.vbo.length;
        ptr = (Byte *)glMapNamedBuffer(mesh.vertex_buffer, GL_READ_ONLY);
        memcpy(cur, ptr, head.vbo.length);
        glUnmapNamedBuffer(mesh.vertex_buffer);
        if (mesh.index_status > 31)
        {
            glGetNamedBufferParameteri64v(mesh.index_buffer, GL_BUFFER_SIZE, &head.ibo.length);
            head.ibo.start = cur - data;
            cur += head.ibo.length;
            ptr = (Byte *)glMapNamedBuffer(mesh.index_buffer, GL_READ_ONLY);
            memcpy(cur, ptr, head.ibo.length);
            glUnmapNamedBuffer(mesh.index_buffer);
        }
        for (size_t i = 0; i < mesh.buffers.size(); i++)
        {
            glGetNamedBufferParameteri64v(mesh.buffers[i], GL_BUFFER_SIZE, &head.buffers[i].length);
            head.buffers[i].start = cur - data
            cur += head.buffers[i].length;
            ptr = (Byte *)glMapNamedBuffer(mesh.buffers[i], GL_READ_ONLY);
            memcpy(cur, ptr, head.buffers[i].length);
            glUnmapNamedBuffer(mesh.buffers[i]);
        }
        Byte* res = CompressData(data,&full_size,sizeof(uint64));
        free(data);
        *(uint64*)res = MESH_HEADER;
        *ret_length = full_size;
        return res;
    }
    void PackMesh(const std::string &path, const Mesh &mesh)
    {
        size_t length;
        Byte* data = PackMesh(&length,mesh);
        std::ofstream fout(path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!fout.is_open())
        {
            throw std::runtime_error("Cannot open file:" + path);
        }
        fout.write((char*)data,length);
        free(data);
        fout.close();
    }
    void Mesh::GenMeshFile(const aiMesh *ptr, const string &save_path)
    {
        size_t length;
        Byte *data = GenMeshFile(ptr, save_path, &length);
        std::ofstream fout(save_path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!fout.is_open())
        {
            throw std::runtime_error("Cannot open file:" + save_path);
        }
        fout.write(data, length);
        fout.close();
    }
    Byte *Mesh::GenMeshFile(const aiMesh *pointer, const std::string &name, size_t *ret_length)
    {
        MeshFile head;
        head.restart_index = UINT64_MAX;
        head.buffer_count = 0;
        size_t vertex_length = sizeof(aiVector3D); // 单个顶点数据的长度
        std::cout << "File:" << name << '\t' << pointer->mName.C_Str() << '\n';
        std::cout << "Vertex Format:\nPositions vec3";
        if (pointer->HasNormals())
        {
            vertex_length += sizeof(aiVector3D);
            std::cout << "\nNormals vec3";
        }
        if (pointer->GetNumUVChannels() > 0)
        {
            std::cout << "\nTexture Coords:";
            for (size_t i = 0; i < pointer->GetNumUVChannels(); i++)
            {
                if (pointer->HasTextureCoords(i))
                {
                    vertex_length += sizeof(ai_real) * pointer->mNumUVComponents[i];
                    cout << "\n\t" << (pointer->HasTextureCoordsName(i) ? pointer->mTextureCoordsNames[i]->C_Str() : "NULL") << ": vec" << pointer->mNumUVComponents[i];
                }
            }
        }
        if (pointer->GetNumColorChannels() > 0)
        {
            vertex_length += sizeof(aiColor4D) * pointer->GetNumColorChannels();
            std::cout << "\nColors: vec4 *" << pointer->GetNumColorChannels();
        }
        if (pointer->HasTangentsAndBitangents())
        {
            vertex_length += sizeof(aiVector3D) * 2;
            std::cout << "\nTangents vec3\nBitangents vec3";
        }

        head.vbo.start = sizeof(MeshFile);
        head.vbo.length = vertex_length * pointer->mNumVertices;
        if (pointer->HasFaces())
        {
            head.index_status = IndexStatus::ONLY_INDEX;
            head.primitive_type = GL_TRIANGLES;
            head.ibo.start = sizeof(MeshFile) + head.vbo.length;
            head.ibo.length = pointer->mNumFaces * sizeof(unsigned int) * 3;
            std::cout << "\nFaces: triangles, unsigned int * 3";
        }
        else
        {
            head.index_status = IndexStatus::NO_INDEX;
            head.primitive_type = GL_NONE;
            head.ibo.start = UINT64_MAX;
            head.ibo.length = 0ULL;
        }
        std::cout << std::endl;

        size_t length = sizeof(MeshFile) + head.vbo.length + head.ibo.length, rl = length;
        Byte *mesh_data = (Byte *)malloc(length), *curpos = mesh_data + sizeof(MeshFile);
        if (mesh_data == nullptr)
        {
            throw std::bad_alloc();
        }
        std::memcpy(mesh_data, &head, sizeof(MeshFile));
        for (size_t i = 0; i < pointer->mNumVertices; i++)
        {
            *((aiVector3D *)curpos) = pointer->mVertices[i];
            curpos += sizeof(aiVector3D);
            if (pointer->HasNormals())
            {
                *((aiVector3D *)curpos) = pointer->mNormals[i];
                curpos += sizeof(aiVector3D);
            }
            for (size_t j = 0; j < pointer->GetNumUVChannels(); j++)
            {
                if (pointer->HasTextureCoords(j))
                {
                    *((aiVector3D *)curpos) = pointer->mTextureCoords[j][i];
                    curpos += sizeof(ai_real) * pointer->mNumUVComponents[j];
                }
            }
            for (size_t j = 0; j < pointer->GetNumColorChannels(); j++)
            {
                if (pointer->HasVertexColors(j))
                {
                    *((aiColor4D *)curpos) = pointer->mColors[j][i];
                    curpos += sizeof(aiColor4D);
                }
            }
            if (pointer->HasTangentsAndBitangents())
            {
                *((aiVector3D *)curpos) = pointer->mTangents[i];
                curpos += sizeof(aiVector3D);
                *((aiVector3D *)curpos) = pointer->mBitangents[i];
                curpos += sizeof(aiVector3D);
            }
        }
        for (size_t i = 0; i < pointer->mNumFaces; i++)
        {
            for (size_t j = 0; j < 3; j++)
            {
                *((unsigned int *)curpos) = pointer->mFaces[i].mIndices[j];
                curpos += sizeof(unsigned int);
            }
        }
        Byte *data = CompressData(mesh_data, &length, sizeof(uint64));
        free(mesh_data);
        *(uint64 *)data = MESH_HEADER;
        cout << "Vertices Count:" << pointer->mNumVertices << '\n';
        cout << "Indices Count:" << pointer->mNumFaces * 3 << '\n';
        cout << "Data size:" << rl + sizeof(uint64) << "Bytes\n";
        cout << "END;" << endl;
        *ret_length = length + sizeof(uint64);
        return data;
    }
    void Mesh::GenMeshFile(const string &path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, assimp_load_process);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            throw std::runtime_error(importer.GetErrorString());
        }
        for (size_t i = 0; i < scene->mNumMeshes; i++)
        {
            GenMeshFile(scene->mMeshes[i], path + std::to_string(i) + scene->mMeshes[i]->mName.C_Str() + ".mesh");
        }
    }
    inline void Mesh::GenMeshFile(const char *path)
    {
        GenMeshFile(std::string(path));
    }
    void Mesh::GenMeshFileMerged(const string &path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, assimp_load_process);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            throw std::runtime_error(importer.GetErrorString());
        }
        std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            throw std::runtime_error("Cannot open out file.");
        }
        uint64 out = 0xF242191756FF0003;
        file.write((char *)&out, sizeof(out));
        out = scene->mNumMeshes;
        file.write((char *)&out, sizeof(out));
        Byte *data;
        size_t length;
        for (size_t i = 0; i < scene->mNumMeshes; i++)
        {
            data = GenMeshFile(scene->mMeshes[i], path + std::to_string(i) + scene->mMeshes[i]->mName.C_Str() + ".mesh", &length);
            file.write((char *)data, length);
            free(data);
        }
    }
    inline void Mesh::GenMeshFileMerged(const char *path)
    {
        GenMeshFileMerged(std::string(path));
    }
    Mesh::~Mesh()
    {
        glDeleteVertexArrays(1, &vertex_array);
        glDeleteBuffers(1, &vertex_buffer);
        glDeleteBuffers(buffers.size(), &buffers[0]);
        if (index_status > 31)
        {
            glDeleteBuffers(1, &index_buffer);
        }
    }
} // namespace Boundless
