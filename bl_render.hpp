#ifndef _BOUNDLESS_RENDER_HPP_FILE_
#define _BOUNDLESS_RENDER_HPP_FILE_
#include "boundless_base.hpp"
namespace Boundless
{
class RenderObject;
class Transform {
   private:
    friend class Renderer;
    Transform *parent, *next_brother, *child_head;

   public:
    RenderObject* render_obj;
    Vector3d position;
    Vector3d scale;
    Quaterniond rotate;
    Matrix4f model;
    bool edited, roenble, enable;

    const Matrix4f& get_model();
};

class RenderObject {
   public:
    Transform* base_transform;
    void* data_ptr;
    Mesh mesh;

    RenderObject() {}
    void enable();
    void disable();
    virtual void draw(const Matrix4f& mvp_matrix,
                      const Matrix4f& model_matrix,
                      const Matrix4f& normal_matrix,
                      const Vector3f& eye_dir) {}
    virtual ~RenderObject() {}
};

class Camera {
   private:
    Matrix4f projection;
    Matrix4f view;
    Matrix4f vp_matrix;

   public:
    bool editedProj;
    bool isFrustum;
    double zfar, znear, width, height;

    bool editedView;
    Vector3d position, forword, up;

    Camera();
    const Matrix4f& get_proj();
    const Matrix4f& get_view();
    const Matrix4f& get_viewproj_matrix();
};

class Renderer {
   private:
    object_pool<Transform, 64> tr_pool;
    object_pool<RenderObject, 64> ro_pool;
    Transform* transform_head;
    std::stack<Matrix4f> mat_stack;
    std::stack<Transform*> draw_ptrstack;

   public:
    Camera camera;

    Renderer();
    Transform* AddTransformNode(const Vector3d& vp,
                                const Vector3d& vs,
                                const Quaterniond& qr);
    Transform* AddTransformNodeUnder(Transform* parent,
                                     const Vector3d& vp,
                                     const Vector3d& vs,
                                     const Quaterniond& qr);
    Transform* AddTransformNodeRight(Transform* brother,
                                     const Vector3d& vp,
                                     const Vector3d& vs,
                                     const Quaterniond& qr);
    template <typename T, typename... arguments>
    Transform* AddObject(const Vector3d& vp,
                         const Vector3d& vs,
                         const Quaterniond& qr,
                         arguments&&... args) {
        static_assert(std::is_base_of<RenderObject, T>::value,
                      "Type must be derived from RenderObject.");
        static_assert(
            sizeof(T) == sizeof(RenderObject),
            "Type must have the same size compare with RenderObject.");
        Transform* tfo = tr_pool.allocate();
        RenderObject* rdo = ro_pool.allocate();
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
        tfo->render_obj = rdo;
        new ((T*)rdo) T(std::forward<arguments>(args)...);
        return tfo;
    }
    template <typename T, typename... arguments>
    Transform* AddObjectUnder(Transform* parent,
                              const Vector3d& vp,
                              const Vector3d& vs,
                              const Quaterniond& qr,
                              arguments&&... args) {
        static_assert(std::is_base_of<RenderObject, T>::value,
                      "Type must be derived from RenderObject.");
        static_assert(
            sizeof(T) == sizeof(RenderObject),
            "Type must have the same size compare with RenderObject.");
        Transform* tfo = tr_pool.allocate();
        Renderer* rdo = ro_pool.allocate();
        tfo->parent = parent;
        tfo->child_head = nullptr;
        tfo->next_brother = parent->child_head;
        parent->child_head = tfo;

        tfo->roenble = false;
        tfo->edited = true;
        tfo->position = vp;
        tfo->scale = vs;
        tfo->rotate = qr;
        tfo->render_obj = rdo;
        new ((T*)rdo) T(std::forward<arguments>(args)...);
        return tfo;
    }
    template <typename T, typename... arguments>
    Transform* AddObjectRight(Transform* brother,
                              const Vector3d& vp,
                              const Vector3d& vs,
                              const Quaterniond& qr,
                              arguments&&... args) {
        static_assert(std::is_base_of<RenderObject, T>::value,
                      "Type must be derived from RenderObject.");
        static_assert(
            sizeof(T) == sizeof(RenderObject),
            "Type must have the same size compare with RenderObject.");
        Transform* tfo = tr_pool.allocate();
        Renderer* rdo = ro_pool.allocate();
        tfo->parent = brother->parent;
        tfo->child_head = nullptr;
        tfo->next_brother = brother->next_brother;
        brother->next_brother = tfo;

        tfo->roenble = false;
        tfo->edited = true;
        tfo->position = vp;
        tfo->scale = vs;
        tfo->rotate = qr;
        tfo->render_obj = rdo;
        new ((T*)rdo) T(std::forward<arguments>(args)...);
        return tfo;
    }
    void DrawAll();
    ~Renderer();
};

extern const GLuint ads_vertpos_attrib;
extern const GLuint ads_vertnormal_attrib;
extern const GLuint ads_mvpmatrix_uniform;
extern const GLuint ads_modelmatrix_uniform;
extern const GLuint ads_normalmatrix_uniform;
extern const GLuint ads_vertcolor_uniform;
extern const GLuint ads_eyedirection_uniform;
extern const GLuint ads_materialindex_uniform;
extern const char* ads_vertshader_path;
extern const char* ads_fragshader_path;
extern const GLsizei ads_stride_array[];
class MeshFunctions {
   public:
    void InitMeshADS(Mesh& mesh);
};
struct LightProp {
    Vector3f ambient;
    Vector3f color;     // 颜色
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
extern Program shader;
extern GLuint uniform_buffer;

extern LightProp lightdata[];
extern MaterialProp materialdata[];

class ADSBase {
   public:
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
    void draw(const Matrix4f& mvp_matrix,
              const Matrix4f& model_matrix,
              const Matrix4f& normal_matrix,
              const Vector3f& eye_dir);
    ~ADSRender();
};
} // namespace Boundless

#endif //!_BOUNDLESS_RENDER_HPP_FILE_