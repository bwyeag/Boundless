#ifndef _BOUNDLESS_RENDER_HPP_FILE_
#define _BOUNDLESS_RENDER_HPP_FILE_
#include "boundless_base.hpp"
namespace Boundless {
class Program {
   public:
    struct ShaderInfo {
        GLenum type;
        GLuint id;
    };

   private:
    GLuint program_id;
    std::vector<ShaderInfo> program_shader;
    void PrintLog() const;

   public:
    Program();
    Program(size_t c);
    Program(const Program& target) = delete;
    Program(Program&& target) noexcept = default;
    Program& operator=(const Program& target) = delete;
    Program& operator=(Program&& target) noexcept = default;
    ShaderInfo& operator[](size_t index);
    ~Program();

    void AddShader(std::string_view path, GLenum type);
    void AddShaderByCode(std::string_view data, GLenum type);
    void Link() const;
    inline GLuint GetID() { return program_id; }

    static void UseProgram(Program& p);
    static void UnUseProgram();

    static GLuint LoadShader(std::string_view path, GLenum type);
    static GLuint LoadShader(const char* path, GLenum type);
    static GLuint ComplieShader(std::string_view code, GLenum type);
};
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
                      const Vector3f& eye_dir) = 0;
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
                         T** retobj,
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
        *retobj = (T*)rdo;
        return tfo;
    }
    template <typename T, typename... arguments>
    Transform* AddObjectUnder(Transform* parent,
                              const Vector3d& vp,
                              const Vector3d& vs,
                              const Quaterniond& qr,
                              T** retobj,
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
        *retobj = (T*)rdo;
        return tfo;
    }
    template <typename T, typename... arguments>
    Transform* AddObjectRight(Transform* brother,
                              const Vector3d& vp,
                              const Vector3d& vs,
                              const Quaterniond& qr,
                              T** retobj,
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
        *retobj = (T*)rdo;
        return tfo;
    }
    void DrawAll();
    ~Renderer();
};

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
#define NUM_MAX_LIGHTS 2
#define NUM_MAX_MATERIALS 1

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

class ADSBase {
   public:
    static Program shader;
    static GLuint uniform_buffer;

    static LightProp lightdata[NUM_MAX_LIGHTS];
    static MaterialProp materialdata[NUM_MAX_MATERIALS];

    static void InitMeshADS(Mesh& mesh);
    static void InitADS();
    static void UpdateUniformBuffer();
};

static const GLsizei starr[]{24};
class ADSRender : public RenderObject, public ADSBase {
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
}  // namespace Boundless

#endif  //!_BOUNDLESS_RENDER_HPP_FILE_