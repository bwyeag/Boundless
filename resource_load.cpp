#include "resource_load.hpp"

namespace Boundless::Resource
{
    //模型加载函数,将模型数据加载到OpenGL
    ModelLoader::ModelLoader(const char* path, ModelLoadFunction f)
    {
        LoadFile(path,f);
    }
    void ModelLoader::LoadFile(const char* path, ModelLoadFunction f)
    {
        model_file.open(path,ios::binary|ios::in);
        {
            if (!model_file.is_open())
            {
                ERROR("Resource","无法打开模型文件:"<<path);
                exit(-1);
            }
            uint64_t head;
            model_file.read((char*)&head,sizeof(uint64_t));
            if (head!=MODEL_HEADER)
            {
                ERROR("Resource","模型文件类型错误,文件头代码:"<<head);
                exit(-1);
            }
        }

        size_t tarlength;//读取压缩前大小
        model_file.read((char*)&tarlength,sizeof(size_t));

        size_t start,srclength;//读取压缩后文件长度
        start=model_file.tellg();
        model_file.seekg(ios::end);
        srclength=model_file.tellg();
        srclength=srclength-start;
        model_file.seekg(start);

        //分配解压缩所用的内存
        void* src=malloc(srclength);
        void* tar = malloc(tarlength);
        if (src==nullptr||tar==nullptr)
        {
            ERROR("Memory","内存耗尽");
            exit(-1);
        }
        model_file.read(src,srclength);//读取压缩前内容
        int res = uncompress2(tar,&tarlength,src,srclength);//解压缩
        free(src);
        if (res==Z_MEM_ERROR || res==Z_BUF_ERROR||res==Z_DATA_ERROR)
        {
            ERROR("ZLIB","在解压时出现错误:"<<res);
            exit(-1);
        }
        ModelHead head_data;
        memcpy(&head_data,tar,sizeof(ModelHead));//复制纹理头数据

        GLsizei bcnt = head_data.buffer_count;
        has_other_buffers=(bcnt==0)?false:true;
        if (head_data.index_start==UINT64_MAX)
            has_index=false,bcnt+=1;
        else
            has_index=true,bcnt+=2;
        restart_index=head_data.restart_index;

        buffers.resize(bcnt);
        glCreateBuffers(bcnt,&buffers[0]);

        BufferInfoData bufferdata[head_data.buffer_count];
        if (head_data.buffer_count>0)
        {
            memcpy(&bufferdata,tar+sizeof(ModelHead),head_data.buffer_count*sizeof(BufferInfoData));
        }

        if (f==nullptr)
        {
            glNamedBufferStorage(buffers[0],head_data.vertex_length,nullptr,GL_MAP_WRITE_BIT);
            char* wdata = (char*)glMapNamedBuffer(buffers[0],GL_WRITE_ONLY);
            memcpy(wdata,tar+head_data.vertex_start,head_data.vertex_length);
            glUnmapNamedBuffer(buffers[0]);

            if (has_index)
            {
                glNamedBufferStorage(buffers[1],head_data.index_length,nullptr,GL_MAP_WRITE_BIT);
                char* wdata = (char*)glMapNamedBuffer(buffers[1],GL_WRITE_ONLY);
                memcpy(wdata,tar+head_data.index_start,head_data.index_length);
                glUnmapNamedBuffer(buffers[1]);
            }

            GLuint* bptr=&buffers[has_index?2:1];
            for (uint32_t i = 0; i < head_data.buffer_count; i++)
            {
                glNamedBufferStorage(bptr[i],bufferdata[i].length,nullptr,GL_MAP_WRITE_BIT);
                char* wdata = (char*)glMapNamedBuffer(bptr[i],GL_WRITE_ONLY);
                memcpy(wdata,tar+bufferdata[i].start,bufferdata[i].length);
                glUnmapNamedBuffer(bptr[i]);
            }
            
        }
        else
        {
            f(tar,&head_data,bufferdata);
        }
        model_file.close();
    }

    TextureLoader::TextureLoader(const char* path, TextureLoadFunction f)
    {
        LoadFile(path,f);
    }
    void TextureLoader::LoadFile(const char* path, TextureLoadFunction f)
    {
        texture_file.open(path,ios::binary|ios::in);
        {
            if (!texture_file.is_open())
            {
                ERROR("Resource","无法打开纹理文件:"<<path);
                exit(-1);
            }
            uint64_t head;
            texture_file.read((char*)&head,sizeof(uint64_t));
            if (head!=TEXTURE_HEADER)
            {
                ERROR("Resource","纹理文件类型错误,文件头代码:"<<head);
                exit(-1);
            }
        }
        size_t tarlength;//读取压缩前大小
        texture_file.read((char*)&tarlength,sizeof(size_t));

        size_t start,srclength;//读取压缩后文件长度
        start=texture_file.tellg();
        texture_file.seekg(ios::end);
        srclength=texture_file.tellg();
        srclength=srclength-start;
        texture_file.seekg(start);

        //分配解压缩所用的内存
        void* src=malloc(srclength);
        void* tar = malloc(tarlength);
        if (src==nullptr||tar==nullptr)
        {
            ERROR("Memory","内存耗尽");
            exit(-1);
        }
        texture_file.read(src,srclength);//读取压缩前内容
        int res = uncompress2(tar,&tarlength,src,srclength);//解压缩
        if (res==Z_MEM_ERROR || res==Z_BUF_ERROR||res==Z_DATA_ERROR)
        {
            ERROR("ZLIB","在解压时出现错误:"<<res);
            exit(-1);
        }
        memcpy(&texture_info,tar,sizeof(TextureHead));//复制纹理头数据
        glCreateTextures(texture_info.gl_target,1,&texture);
        int count = 1+(texture_info.height==0)?0:1+(texture_info.depth==0)?0:1;
        switch (count)//加载纹理数据
        {
        case 1:
            glTextureStorage1D(texture,texture_info.mipmap_level,texture_info.gl_internal_format,texture_info.width);
            glTextureSubImage1D(texture,0,0,texture_info.width,texture_info.gl_format,texture_info.gl_type,tar+sizeof(TextureHead));
            break;
        case 2:
            glTextureStorage2D(texture,texture_info.mipmap_level,texture_info.gl_internal_format,texture_info.width,texture_info.height);
            glTextureSubImage2D(texture,0,0,0,texture_info.width,texture_info.height,texture_info.gl_format,texture_info.gl_type,tar+sizeof(TextureHead));
            break;
        case 3:
            glTextureStorage2D(texture,texture_info.mipmap_level,texture_info.gl_internal_format,texture_info.width,texture_info.height,texture_info.depth);
            glTextureSubImage3D(texture,0,0,0,0,texture_info.width,texture_info.height,texture_info.depth,texture_info.gl_format,texture_info.gl_type,tar+sizeof(TextureHead));
            break;
        default:
            ERROR("Resource","无法识别的纹理长宽高格式:"<<count);
            exit(-1);
            break;
        }
        free(src);free(tar);
        if (f!=nullptr)//允许进行额外处理
        {
            f(this,texture,&texture_info);
        }
        texture_file.close();
    }

    void GenerateInitialize()
    {
        stbi_set_flip_vertically_on_load(true);
    }
    void GenerateModelFile(const char* path)
    {
        
    }
    void GenerateTextureFile2D(const char* path)
    {
        TextureHead th;
        th.gl_target=GL_TEXTURE_2D;
        th.gl_type=GL_UNSIGNED_BYTE;
        th.depth=0;
        int n;
        uint8_t data = stbi_load(path, th.width,th.height,&n,0);
        if (data==nullptr)
        {
            ERROR("STB_IMAGE","加载图像失败");
            exit(-1);
        }
        switch (n)
        {
        case 1:
            th.gl_format=GL_ALPHA;
            break;
        case 2:
            th.gl_format=GL_RG;
            break;
        case 3:
            th.gl_format=GL_RGB;
            break;
        case 4:
            th.gl_format=GL_RGBA;
            break;
        default:
            ERROR("Resource","图像通道数错误:"<<n);
            break;
        }
        th.gl_internal_format=th.gl_format;
        size_t len = strlen(path);
        char p[len+5];
        strcpy(p,path);
        strcpy(p+len,".tex");
        ofstream file;
        file.open(p,ios::binary|ios::out|ios::trunc);
        uint64_t head = TEXTURE_HEADER;
        //----------todo----------
    }

} // namespace Boundless::Resource