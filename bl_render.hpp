#ifndef _BL_RENDER_HPP_FILE_
#define _BL_RENDER_HPP_FILE_
#include "boundless.hpp"

namespace Boundless {
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
class MeshFunctions {
   public:
    void InitMeshADS(Mesh& mesh) {
        mesh.InitMesh();
        glEnableVertexArrayAttrib(mesh.GetVAO(), ads_vertpos_attrib);
        glEnableVertexArrayAttrib(mesh.GetVAO(), ads_vertnormal_attrib);
        glVertexArrayAttribFormat(mesh.GetVAO(), ads_vertpos_attrib, 3,
                                  GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribFormat(mesh.GetVAO(), ads_vertnormal_attrib, 3,
                                  GL_FLOAT, GL_FALSE, 12);
        glVertexArrayAttribBinding(mesh.GetVAO(), ads_vertpos_attrib, 0);
        glVertexArrayAttribBinding(mesh.GetVAO(), ads_vertnormal_attrib, 0);
    }
} struct LightProp {
    Vector3f ambient;  // true->锥光
    Vector3f color;    // 颜色
    Vector3f position;  // if (isLocal) 表示光的位置; else 表示光的方向
    Vector3f halfVector;  // 锥光的半程向量
    Vector3f coneDirection;
    bool enable;   // 是否启用
    bool isLocal;  // true->点光源和锥光; false->方向光
    bool isSpot;   // true->锥光
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

class ADSBase {
   public:
    static Program shader;
    static GLuint uniform_buffer;

    static LightProp lightdata[NUM_MAX_LIGHTS];
    static MaterialProp materialdata[NUM_MAX_MATERIALS];
    static void Init();
    static void UpdateUniformBuffer();
};

static const GLsizei starr[]{24};
class ADSRender : public RenderObject {
   public:
    struct ADSData {
        uint32 materialindex;
        Vector4f vertexcolor;
    };

    ADSRender(ADSData* base) { this->data_ptr = base; }
    virtual void draw(const Matrix4f& mvp_matrix,
                      const Matrix4f& model_matrix,
                      const Matrix4f& normal_matrix,
                      const Vector3f& eye_dir);
    ~ADSRender();
};

}  // namespace Boundless
#endif  //!_BL_RENDER_HPP_FILE_