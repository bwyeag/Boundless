#ifndef _BL_RENDER_HPP_FILE_
#define _BL_RENDER_HPP_FILE_
#include "boundless.hpp"

namespace Boundless {
struct LightProp {
    Vector3f ambient;  // true->锥光
    Vector3f color;    // 颜色
    Vector3f position;  // if (isLocal) 表示光的位置; else 表示光的方向
    Vector3f halfVector;  // 锥光的半程向量
    Vector3f coneDirection;
    bool enable;                 // 是否启用
    bool isLocal;                // true->点光源和锥光; false->方向光
    bool isSpot;                 // true->锥光
    bool edited;
    float spotCosCutoff;         // 聚光灯余弦截止
    float spotExponent;          // 聚光灯衰减系数
    float constantAttenuation;   // 光照衰减常量部分
    float linearAttenuation;     // 光照衰减线性部分
    float quadraticAttenuation;  // 光照衰减平方部分
};
struct MaterialProp {
    bool edited;
    Vector3f emission;  // 材质的照明
    Vector3f ambient;
    Vector3f diffuse;
    Vector3f specular;
    float shininess;
};
#define NUM_MAX_LIGHTS 2
#define NUM_MAX_MATERIALS 1

class LightBase {
   private:
    Program shader;
    GLuint uniform_buffer;

   public:
    LightProp lightdata[NUM_MAX_LIGHTS];
    MaterialProp materialdata[NUM_MAX_MATERIALS];
    LightBase() {}
    void Load() {
        glCreateBuffers(1, &uniform_buffer);
        glNamedBufferStorage(
            uniform_buffer,
            NUM_MAX_LIGHTS * 16 * 7 + NUM_MAX_MATERIALS * 16 * 4, nullptr,
            GL_DYNAMIC_STORAGE_BIT);
        shader.AddShader(".\\shader\\classic_shader_vertex.glsl",
                         GL_VERTEX_SHADER);
        shader.AddShader(".\\shader\\classic_shader_fragment.glsl",
                         GL_FRAGMENT_SHADER);
        shader.Link();
    }
    void UpdateUniformBuffer() {
        for (size_t i = 0; i < NUM_MAX_LIGHTS; i++)
        {
            if (lightdata[i].edited)
            {
                glNamedBufferSubData(uniform_buffer, 0 + 16*7*i, sizeof(Vector3f), &lightdata[i].ambient);
                glNamedBufferSubData(uniform_buffer, 16 + 16*7*i, sizeof(Vector3f), &lightdata[i].color);
                glNamedBufferSubData(uniform_buffer, 32 + 16*7*i, sizeof(Vector3f), &lightdata[i].position);
                glNamedBufferSubData(uniform_buffer, 48 + 16*7*i, sizeof(Vector3f), &lightdata[i].halfVector);
                glNamedBufferSubData(uniform_buffer, 64 + 16*7*i, sizeof(Vector3f), &lightdata[i].coneDirection);
                glNamedBufferSubData(uniform_buffer, 76 + 16*7*i, sizeof(Vector3f), &lightdata[i].enable);
                glNamedBufferSubData(uniform_buffer, 77 + 16*7*i, sizeof(Vector3f), &lightdata[i].isLocal);
                glNamedBufferSubData(uniform_buffer, 78 + 16*7*i, sizeof(Vector3f), &lightdata[i].isSpot);
                glNamedBufferSubData(uniform_buffer, 80 + 16*7*i, sizeof(float), &lightdata[i].spotCosCutoff);
                glNamedBufferSubData(uniform_buffer, 84 + 16*7*i, sizeof(float), &lightdata[i].spotExponent);
                glNamedBufferSubData(uniform_buffer, 88 + 16*7*i, sizeof(float), &lightdata[i].constantAttenuation);
                glNamedBufferSubData(uniform_buffer, 92 + 16*7*i, sizeof(float), &lightdata[i].linearAttenuation);
                glNamedBufferSubData(uniform_buffer, 96 + 16*7*i, sizeof(float), &lightdata[i].quadraticAttenuation);
            }
            
        }
        for (size_t i = 0; i < NUM_MAX_MATERIALS; i++)
        {
            if (materialdata[i].edited)
            {
                glNamedBufferSubData(uniform_buffer, 0 + 16*7*NUM_MAX_LIGHTS + 16 * 4*i, sizeof(Vector3f), &materialdata[i].emission);
                glNamedBufferSubData(uniform_buffer, 16 + 16*7*NUM_MAX_LIGHTS + 16 * 4*i, sizeof(Vector3f), &materialdata[i].ambient);
                glNamedBufferSubData(uniform_buffer, 32 + 16*7*NUM_MAX_LIGHTS + 16 * 4*i, sizeof(Vector3f), &materialdata[i].diffuse);
                glNamedBufferSubData(uniform_buffer, 48 + 16*7*NUM_MAX_LIGHTS + 16 * 4*i, sizeof(Vector3f), &materialdata[i].specular);
                glNamedBufferSubData(uniform_buffer, 60 + 16*7*NUM_MAX_LIGHTS + 16 * 4*i, sizeof(float), &materialdata[i].shininess);
            }
        }
    }
    ~LightBase() {}
};

class LightRender : public RenderObject {
   public:
    LightRender(LightBase* base) { this->data_ptr = base; }
    virtual void draw(const Matrix4f& mvp) { glBindVertexArray(mesh.GetVAO()); }
    ~LightRender();
};

}  // namespace Boundless
#endif  //!_BL_RENDER_HPP_FILE_