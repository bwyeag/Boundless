#include "bl_render.hpp"

namespace Boundless
{
    void LightBase::Load() {
        glCreateBuffers(1, &uniform_buffer);
        glNamedBufferStorage(
            uniform_buffer,
            NUM_MAX_LIGHTS * 16 * 7 + NUM_MAX_MATERIALS * 16 * 4, nullptr,
            GL_DYNAMIC_STORAGE_BIT);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);
        shader.AddShader(".\\shader\\classic_shader_vertex.glsl",
                         GL_VERTEX_SHADER);
        shader.AddShader(".\\shader\\classic_shader_fragment.glsl",
                         GL_FRAGMENT_SHADER);
        shader.Link();
    }
    void LightBase::UpdateUniformBuffer() {
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
    void LightRender::init(GLsizei count = 0) {
        mesh.InitMesh(starr);
        glEnableVertexArrayAttrib(mesh.GetVAO(), 4);
        glEnableVertexArrayAttrib(mesh.GetVAO(), 5);
        glVertexArrayAttribFormat(mesh.GetVAO(), 4, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribFormat(mesh.GetVAO(), 5, 3, GL_FLOAT, GL_FALSE, 12);
        glVertexArrayAttribBinding(mesh.GetVAO(), 4, 0);
        glVertexArrayAttribBinding(mesh.GetVAO(), 5, 0);
        data_ptr = static_cast<void*>(count);
    }
    virtual void LightRender::draw(const Matrix4f& mvp_matrix,
                      const Matrix4f& model_matrix,
                      const Matrix4f& normal_matrix) {
        glBindVertexArray(mesh.GetVAO());
        shader.UseProgram();
        if (mesh.index_status == IndexStatus::NO_INDEX)
        {
            glDrawArrays(GL_TRIANGLES, 0,static_cast<GLsizei>(data_ptr));
        }
        else if (mesh.index_status == IndexStatus::ONLY_INDEX)
        {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(data_ptr), GL_UNSIGNED_INT, 0);
        }
        else if (mesh.index_status == IndexStatus::RESTART_INDEX) {
            glEnable(GL_PRIMITIVE_RESTART);
            glPrimitiveRestartIndex(mesh.restart_index);
            glDrawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(data_ptr), GL_UNSIGNED_INT, 0);
            glDisable(GL_PRIMITIVE_RESTART);
        }
        
        
    }
} // namespace Boundless
