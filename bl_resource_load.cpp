#include "bl_resource_load.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace Boundless::Resource
{
    // 模型加载函数,将模型数据加载到OpenGL
    MeshLoader::MeshLoader(const char *path, MeshLoadFunction f)
    {
        LoadFile(path, f);
    }
    void MeshLoader::LoadFile(const char *path, MeshLoadFunction f)
    {
        model_file.open(path, ios::binary | ios::in);
        {
            if (!model_file.is_open())
            {
                ERROR("Resource", "无法打开文件:", path);
                return;
            }
            uint64_t head;
            model_file.read((char *)&head, sizeof(uint64_t));
            if (head != MODEL_HEADER)
            {
                ERROR("Resource", "文件类型错误,文件头代码:", head);
                return;
            }
        }
        // tar<--src
        size_t tarlength; // 读取压缩前大小
        model_file.read((char *)&tarlength, sizeof(size_t));

        size_t start, srclength; // 读取压缩后文件长度
        start = model_file.tellg();
        model_file.seekg(ios::end);
        srclength = model_file.tellg();
        srclength = srclength - start;
        model_file.seekg(start);

        // 分配解压缩所用的内存
        void *src = malloc(srclength);
        void *tar = malloc(tarlength);
        if (src == nullptr || tar == nullptr)
        {
            ERROR("Memory", "内存耗尽");
            return;
        }
        model_file.read((char *)src, srclength);                                                     // 读取压缩前内容
        int res = uncompress2((Bytef *)tar, (uLongf *)&tarlength, (Bytef *)src, (uLong *)srclength); // 解压缩
        free(src);
        if (res == Z_MEM_ERROR || res == Z_BUF_ERROR || res == Z_DATA_ERROR)
        {
            ERROR("ZLIB", "在解压时出现错误:", res);
            return;
        }
        MeshHead head_data;
        memcpy(&head_data, tar, sizeof(MeshHead)); // 复制纹理头数据

        GLsizei bcnt = head_data.buffer_count;
        has_other_buffers = (bcnt == 0) ? false : true;
        if (head_data.index_start == UINT64_MAX)
        {
            index_status = IndexStatus::NONE, bcnt += 1;
        }
        else if (head_data.restart_index == UINT64_MAX)
        {
            index_status = IndexStatus::ONLY_INDEX;
            bcnt += 2;
        }
        else
        {
            index_status = IndexStatus::INDEX_WITH_RESTARTINDEX;
            restart_index = head_data.restart_index;
            bcnt += 2;
        }

        buffers.resize(bcnt);
        glCreateBuffers(bcnt, &buffers[0]);

        BufferInfoData bufferdata[head_data.buffer_count];
        if (head_data.buffer_count > 0)
        {
            memcpy(&bufferdata, (uint8_t *)tar + sizeof(MeshHead), head_data.buffer_count * sizeof(BufferInfoData));
        }

        if (f == nullptr)
        {
            glNamedBufferStorage(buffers[0], head_data.vertex_length, nullptr, GL_MAP_WRITE_BIT);
            char *wdata = (char *)glMapNamedBuffer(buffers[0], GL_WRITE_ONLY);
            memcpy(wdata, (uint8_t *)tar + head_data.vertex_start, head_data.vertex_length);
            glUnmapNamedBuffer(buffers[0]);

            if (has_index)
            {
                glNamedBufferStorage(buffers[1], head_data.index_length, nullptr, GL_MAP_WRITE_BIT);
                char *wdata = (char *)glMapNamedBuffer(buffers[1], GL_WRITE_ONLY);
                memcpy(wdata, (uint8_t *)tar + head_data.index_start, head_data.index_length);
                glUnmapNamedBuffer(buffers[1]);
            }

            GLuint *bptr = &buffers[has_index ? 2 : 1];
            for (uint32_t i = 0; i < head_data.buffer_count; i++)
            {
                glNamedBufferStorage(bptr[i], bufferdata[i].length, nullptr, GL_MAP_WRITE_BIT);
                char *wdata = (char *)glMapNamedBuffer(bptr[i], GL_WRITE_ONLY);
                memcpy(wdata, (uint8_t *)tar + bufferdata[i].start, bufferdata[i].length);
                glUnmapNamedBuffer(bptr[i]);
            }
        }
        else
        {
            f(tar, &head_data, bufferdata);
        }
        free(tar);
        model_file.close();
    }

    Texture2DLoader::Texture2DLoader(const char *path, TextureLoadFunction f)
    {
        LoadFile(path, f);
    }
    void Texture2DLoader::LoadFile(const char *path, TextureLoadFunction f)
    {
        texture_file.open(path, ios::binary | ios::in);
        {
            if (!texture_file.is_open())
            {
                ERROR("Resource", "无法打开文件:", path);
                return;
            }
            uint64_t head;
            texture_file.read((char *)&head, sizeof(uint64_t));
            if (head != TEXTURE_HEADER)
            {
                ERROR("Resource", "文件类型错误,文件头代码:", head);
                return;
            }
        }
        size_t tarlength; // 读取压缩前大小
        texture_file.read((char *)&tarlength, sizeof(size_t));

        size_t start, srclength; // 读取压缩后文件长度
        start = texture_file.tellg();
        texture_file.seekg(ios::end);
        srclength = texture_file.tellg();
        srclength = srclength - start;
        texture_file.seekg(start);

        // 分配解压缩所用的内存
        void *src = malloc(srclength);
        void *tar = malloc(tarlength);
        if (src == nullptr || tar == nullptr)
        {
            ERROR("Memory", "内存耗尽");
            return;
        }
        texture_file.read((char *)src, srclength);                                                   // 读取压缩前内容
        int res = uncompress2((Bytef *)tar, (uLongf *)&tarlength, (Bytef *)src, (uLong *)srclength); // 解压缩
        if (res == Z_MEM_ERROR || res == Z_BUF_ERROR || res == Z_DATA_ERROR)
        {
            ERROR("ZLIB", "在解压时出现错误:", res);
            return;
        }
        memcpy(&texture_info, tar, sizeof(TextureHead)); // 复制纹理头数据
        glCreateTextures(texture_info.gl_target, 1, &texture);
        int count = 1 + (texture_info.height == 0) ? 0 : 1 + (texture_info.depth == 0) ? 0
                                                                                       : 1;
        switch (count) // 加载纹理数据
        {
        case 1:
            glTextureStorage1D(texture, texture_info.mipmap_level, texture_info.gl_internal_format, texture_info.width);
            glTextureSubImage1D(texture, 0, 0, texture_info.width, texture_info.gl_format, texture_info.gl_type, (uint8_t *)tar + sizeof(TextureHead));
            break;
        case 2:
            glTextureStorage2D(texture, texture_info.mipmap_level, texture_info.gl_internal_format, texture_info.width, texture_info.height);
            glTextureSubImage2D(texture, 0, 0, 0, texture_info.width, texture_info.height, texture_info.gl_format, texture_info.gl_type, (uint8_t *)tar + sizeof(TextureHead));
            break;
        case 3:
            glTextureStorage3D(texture, texture_info.mipmap_level, texture_info.gl_internal_format, texture_info.width, texture_info.height, texture_info.depth);
            glTextureSubImage3D(texture, 0, 0, 0, 0, texture_info.width, texture_info.height, texture_info.depth, texture_info.gl_format, texture_info.gl_type, (uint8_t *)tar + sizeof(TextureHead));
            break;
        default:
            ERROR(RES_ERROR, RES_TEXTURE_TYPE_ERROR << count);
            break;
        }
        free(src);
        free(tar);
        if (f != nullptr) // 允许进行额外处理
        {
            f(texture, &texture_info);
        }
        texture_file.close();
    }

    void FileGeneraters::GenInitialize()
    {
        stbi_set_flip_vertically_on_load(true);
    }
    void FileGeneraters::GenModelFile(const char *path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            ERROR("ASSIMP", importer.GetErrorString());
            return;
        }
        string pstr = path;
        for (size_t i = 0; i < scene->mNumMeshes; i++)
        {
            FileGeneraters::GenMeshFile(scene->mMeshes[i], pstr + std::to_string(i) + scene->mMeshes[i]->mName.C_Str() + ".mesh");
        }
    }
    void FileGeneraters::GenMeshFile(const aiMesh *pointer, const string &path)
    {
        ofstream file;
        MeshHead mh;
        mh.buffer_count = 0;
        mh.restart_index = UINT64_MAX;
        mh.index_length = 0;
        size_t vertex_length = sizeof(aiVector3D); // 单个顶点数据的长度
        cout << "File:" << path << '\t' << pointer->mName.C_Str() << '\n';
        cout << "Vertex Format:\nPositions vec3";
        if (pointer->HasNormals())
        {
            vertex_length += sizeof(aiVector3D);
            cout << "\nNormals vec3";
        }
        if (pointer->GetNumUVChannels() > 0)
        {
            cout << "\nTexture Coords:";
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
            cout << "\nColors: vec4 *" << pointer->GetNumColorChannels();
        }
        if (pointer->HasTangentsAndBitangents())
        {
            vertex_length += sizeof(aiVector3D) * 2;
            cout << "\nTangents vec3\nBitangents vec3";
        }
        if (pointer->HasFaces())
        {
            mh.index_length = pointer->mNumFaces * sizeof(unsigned int) * 3;
            cout << "\nFaces: triangles, unsigned int * 3";
        }
        cout << endl;
        mh.vertex_length = vertex_length * pointer->mNumVertices;
        mh.index_start = sizeof(MeshHead) + mh.vertex_length;
        mh.vertex_start = sizeof(MeshHead);

        uint8_t *mesh_data = (uint8_t *)malloc(mh.index_start + mh.index_length), *curpos = mesh_data;
        if (mesh_data == nullptr)
        {
            ERROR(MEMORY_ERROR, OUT_OF_MEMORY_ERROR);
            error_handle();
            return;
        }
        memcpy(mesh_data, &mh, sizeof(MeshHead));
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
        size_t compress_size = compressBound(mh.index_start + mh.index_length);
        void *compress_data = malloc(compress_size);
        if (compress_data == nullptr)
        {
            ERROR(MEMORY_ERROR, OUT_OF_MEMORY_ERROR);
            error_handle();
            return;
        }
        int res = compress2((Bytef *)compress_data, (uLongf *)&compress_size, (Bytef *)mesh_data, uLong(mh.index_start + mh.index_length), COMPRESS_LEVEL);
        free(mesh_data);
        if (res == Z_MEM_ERROR || res == Z_BUF_ERROR || res == Z_DATA_ERROR)
        {
            free(compress_data);
            ERROR(LIB_ZLIB_ERROR, LIB_ZLIB_COMPRESS_ERROR << res);
            error_handle();
            return;
        }
        uint64_t head = MODEL_HEADER;
        file.open(path, ios::binary | ios::out | ios::trunc);
        file.write((char *)&head, sizeof(uint64_t));
        head = mh.index_start + mh.index_length;
        file.write((char *)&head, sizeof(uint64_t));
        file.write((char *)compress_data, compress_size);
        file.close();
        free(compress_data);
        cout << "Vertices Count:" << pointer->mNumVertices << '\n';
        cout << "Indices Count:" << pointer->mNumFaces * 3 << '\n';
        cout << "Data size:" << head << "Bytes\n";
        cout << "END;" << endl;
    }
    void FileGeneraters::GenTextureFile2D(const char *path)
    {
        TextureHead th;
        th.gl_target = GL_TEXTURE_2D;
        th.gl_type = GL_UNSIGNED_BYTE;
        th.depth = 0;
        int n;
        uint8_t *data = stbi_load(path, &th.width, &th.height, &n, 0);
        if (data == nullptr)
        {
            ERROR(LIB_STB_IMAGE_ERROR, LIB_STB_IMAGE_LOAD_ERROR << path);
            return;
        }
        size_t data_length = th.width * th.height * n * sizeof(uint8_t);
        cout << "File:\t" << path;
        cout << "Image Data Size:\t" << data_length << "Bytes\n";

        switch (n)
        {
        case 1:
            th.gl_internal_format = th.gl_format = GL_ALPHA;
            break;
        case 2:
            th.gl_internal_format = th.gl_format = GL_RG;
            break;
        case 3:
            th.gl_internal_format = th.gl_format = GL_RGB;
            break;
        case 4:
            th.gl_internal_format = th.gl_format = GL_RGBA;
            break;
        default:
            ERROR("Resource", "图像通道数错误:" << n);
            return;
        }
        cout << "Image Cannels:\t" << th.gl_format << '\n';
        string p = path;
        p = p + ".texture";
        ofstream file;
        file.open(p, ios::binary | ios::out | ios::trunc);
        if (!file.is_open())
        {
            ERROR("Resource", "无法创建文件:",p);
            return;
        }

        uint64_t head = TEXTURE_HEADER;
        file.write((char *)&head, sizeof(uint64_t));

        head = compressBound(data_length + sizeof(TextureHead)); // head用作压缩后大小
        void *before = malloc(data_length + sizeof(TextureHead));
        void *compressdata = malloc(head);
        if (before == nullptr || compressdata == nullptr)
        {
            if (before!=nullptr)
            {
                free(before);
            }
            if (compressdata!=nullptr)
            {
                free(compressdata);
            }
            ERROR("Memory", "内存耗尽");
            return;
        }
        
        memcpy(before, &th, sizeof(TextureHead));
        memcpy((uint8_t *)before + sizeof(TextureHead), data, data_length);
        stbi_image_free(data);

        int res = compress2((Bytef *)compressdata, (uLongf *)&head, (Bytef *)before, (uLong)(data_length + sizeof(TextureHead)), COMPRESS_LEVEL); // head用作实际压缩后大小
        cout << "Before Compress Size:\t" << data_length + sizeof(TextureHead) << '\n';
        cout << "After Compress Size:\t" << head << endl;
        free(before);
        if (res == Z_MEM_ERROR || res == Z_BUF_ERROR || res == Z_DATA_ERROR)
        {
            free(compressdata);
            file.close();
            ERROR("ZLIB", "在压缩时出现错误:" << res);
            return;
        }
        file.seekp(sizeof(uint64_t), ios::cur);
        file.write((char *)compressdata, head);
        file.seekp(sizeof(uint64_t), ios::beg); // 移动到开头
        head = data_length + sizeof(TextureHead);
        file.write((char *)&head, sizeof(uint64_t)); // 写入压缩前大小
        free(compressdata);
        file.close();
        cout << "END FILE;" << endl;
    }
} // namespace Boundless::Resource