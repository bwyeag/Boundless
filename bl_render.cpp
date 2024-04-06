#include "bl_render.hpp"

namespace Boundless
{
inline Program::Program() {}
inline Program::Program(size_t c) {
    program_id = glCreateProgram();
    program_shader.reserve(c);
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

void ADSBase::InitMeshADS(Mesh& mesh) {
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
void ADSBase::InitADS() {
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
    glProgramUniformMatrix4fv(shader.GetID(), ads_modelmatrix_uniform, 1,
                              GL_FALSE, &model_matrix(0, 0));
    glProgramUniformMatrix4fv(shader.GetID(), ads_normalmatrix_uniform, 1,
                              GL_FALSE, &normal_matrix(0, 0));
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
} // namespace Boundless
