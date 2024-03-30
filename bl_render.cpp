#include "bl_render.hpp"

namespace Boundless {
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
virtual void ADSRender::draw(const Matrix4f& mvp_matrix,
                             const Matrix4f& model_matrix,
                             const Matrix4f& normal_matrix,
                             const Vector3f& eye_dir) {
    glBindVertexArray(mesh.GetVAO());
    shader.UseProgram();
    glProgramUniform1i(ADSBase::shader.GetID(), ads_materialindex_uniform,
                       ((ADSData*)data_ptr)->materialindex);
    glProgramUniform4fv(ADSBase::shader.GetID(), ads_vertcolor_uniform,
                        &((ADSData*)data_ptr)->vertexcolor.x());
    glProgramUniformMatrix4fv(ADSBase::shader.GetID(), ads_mvpmatrix_uniform, 1,
                              GL_FALSE, &mvp_matrix(0, 0));
    glProgramUniformMatrix4fv(ADSBase::shader.GetID(), ads_modelmatrix_uniform,
                              1, GL_FALSE, &model_matrix(0, 0));
    glProgramUniformMatrix4fv(ADSBase::shader.GetID(), ads_normalmatrix_uniform,
                              1, GL_FALSE, &normal_matrix(0, 0));
    glProgramUniform3fv(ADSBase::shader.GetID(), ads_eyedirection_uniform, 1,
                        &eye_dir.x());
    if (mesh.index_status == IndexStatus::NO_INDEX) {
        glDrawArrays(mesh.GetPrimitiveType(), 0, mesh.GetCount());
    } else if (mesh.index_status == IndexStatus::ONLY_INDEX) {
        glDrawElements(mesh.GetPrimitiveType(), mesh.GetCount(),
                       mesh.GetIndexType(), 0);
    } else if (mesh.index_status == IndexStatus::RESTART_INDEX) {
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(mesh.GetRestartIndex());
        glDrawElements(mesh.GetPrimitiveType(), mesh.GetCount(),
                       mesh.GetIndexType(), 0);
        glDisable(GL_PRIMITIVE_RESTART);
    }
}
}  // namespace Boundless
