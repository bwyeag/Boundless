#include "boundless.hpp"

namespace Boundless {
void MeshMaker::MakeCube(Mesh& mesh, float size, VertexData df) {
    size = abs(size);
    if (df == VertexData::POSITION) {
        mesh.primitive_type = GL_TRIANGLE_STRIP;
        mesh.restart_index = UINT16_MAX;
        mesh.index_type = GL_UNSIGNED_SHORT;
        mesh.mesh_count = 17;
        mesh.InitIndexStatus(IndexStatus::RESTART_INDEX);
        size /= 2.0f;
        const float vparr[24]{
            -size, size,  size,  size,  size,  size,  -size, size,
            -size, size,  size,  -size, -size, -size, -size, size,
            -size, -size, -size, -size, size,  size,  -size, size,
        };
        const uint16 viarr[17]{0, 1, 2, 3, 4, 5, 6, 7, UINT16_MAX,
                               2, 4, 0, 6, 1, 7, 3, 5};
        glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                             GL_STATIC_READ);
        glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
        glNamedBufferStorage(mesh.vertex_array, sizeof(vparr), vparr,
                             GL_STATIC_READ);
        glVertexArrayVertexBuffer(mesh.vertex_array, 0, mesh.vertex_buffer, 0,
                                  12);
        glEnableVertexArrayAttrib(mesh.vertex_array, 0);
        glVertexArrayAttribFormat(mesh.vertex_array, 0, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribBinding(mesh.vertex_array, 0, 0);
    } else if (df == VertexData::NORMAL) {
        mesh.primitive_type = GL_TRIANGLE_STRIP;
        mesh.restart_index = UINT32_MAX;
        mesh.index_type = GL_UNSIGNED_SHORT;
        mesh.mesh_count = 17;
        mesh.InitIndexStatus(IndexStatus::RESTART_INDEX);
        size /= 2.0f;
        const float vparr[144]{
            -size, size,  size,  0.0f,  1.0f,  0.0f,  size,  size,
            size,  0.0f,  1.0f,  0.0f,  -size, size,  -size, 0.0f,
            1.0f,  0.0f,  size,  size,  -size, 0.0f,  1.0f,  0.0f,

            -size, size,  -size, 0.0f,  0.0f,  -1.0f, size,  size,
            -size, 0.0f,  0.0f,  -1.0f, -size, -size, -size, 0.0f,
            0.0f,  -1.0f, size,  -size, -size, 0.0f,  0.0f,  -1.0f,

            -size, -size, -size, 0.0f,  -1.0f, 0.0f,  size,  -size,
            -size, 0.0f,  -1.0f, 0.0f,  -size, -size, size,  0.0f,
            -1.0f, 0.0f,  size,  -size, size,  0.0f,  -1.0f, 0.0f,

            -size, size,  size,  0.0f,  0.0f,  1.0f,  size,  size,
            size,  0.0f,  0.0f,  1.0f,  -size, -size, size,  0.0f,
            0.0f,  1.0f,  size,  -size, size,  0.0f,  0.0f,  1.0f,

            -size, size,  size,  -1.0f, 0.0f,  0.0f,  -size, -size,
            size,  -1.0f, 0.0f,  0.0f,  -size, size,  -size, -1.0f,
            0.0f,  0.0f,  -size, -size, -size, -1.0f, 0.0f,  0.0f,

            size,  size,  size,  1.0f,  0.0f,  0.0f,  size,  -size,
            size,  1.0f,  0.0f,  0.0f,  size,  size,  -size, 1.0f,
            0.0f,  0.0f,  size,  -size, -size, 1.0f,  0.0f,  0.0f};
        const uint16 viarr[]{
            0,  1,  2,  3,  UINT16_MAX, 4,  5,  6,  7,  UINT16_MAX,
            8,  9,  10, 11, UINT16_MAX, 12, 13, 14, 15, UINT16_MAX,
            16, 17, 18, 19, UINT16_MAX, 20, 21, 22, 23, UINT16_MAX};
        glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                             GL_STATIC_READ);
        glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
        glNamedBufferStorage(mesh.vertex_array, sizeof(vparr), vparr,
                             GL_STATIC_READ);
        glVertexArrayVertexBuffer(mesh.vertex_array, 0, mesh.vertex_buffer, 0,
                                  24);
        glEnableVertexArrayAttrib(mesh.vertex_array, 0);
        glEnableVertexArrayAttrib(mesh.vertex_array, 1);
        glVertexArrayAttribFormat(mesh.vertex_array, 0, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribFormat(mesh.vertex_array, 1, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribBinding(mesh.vertex_array, 0, 0);
        glVertexArrayAttribBinding(mesh.vertex_array, 1, 0);
    }
}
// static void MeshMaker::MakePlane(Mesh& mesh,
//                             float size,
//                             int xdiv,
//                             int ydiv,
//                             VertexData df) {}
void MeshMaker::MakeSphere(Mesh& mesh, float r, int rdiv, int hdiv, VertexData df) {
    r = std::abs(r);
    rdiv = std::max(rdiv, 8);
    hdiv = std::max(hdiv, 8);
    if (df == VertexData::POSITION) {
        mesh.primitive_type = GL_TRIANGLE_STRIP;
        mesh.restart_index = UINT32_MAX;
        mesh.InitIndexStatus(IndexStatus::RESTART_INDEX);
        int cnt = rdiv * hdiv + 2;
        float *vparr = (float*)malloc(sizeof(float) * 3 * cnt), *cur = vparr;
        if (!vparr)
            throw std::bad_alloc();
        *(cur++) = 0.0f;
        *(cur++) = 0.0f;
        *(cur++) = r;
        *(cur++) = 0.0f;
        *(cur++) = 0.0f;
        *(cur++) = -r;
        for (int i = 0; i < rdiv; i++) {
            float t = 2 * BL_MATH_PI / rdiv * i;
            Vector3f rvec(cosf(t), sinf(t), 0.0f);
            Vector3f zvec(0.0f, 0.0f, 1.0f);
            for (int j = 1; j < hdiv + 1; j++) {
                t = -BL_MATH_PI / 2 + BL_MATH_PI / (rdiv + 1) * j;
                Vector3f pos = r * (rvec * cosf(t) + zvec * sinf(t));
                *(cur++) = pos.x();
                *(cur++) = pos.y();
                *(cur++) = pos.z();
            }
        }
        glNamedBufferStorage(mesh.vertex_array, sizeof(vparr), vparr,
                             GL_STATIC_READ);
        glVertexArrayVertexBuffer(mesh.vertex_array, 0, mesh.vertex_buffer, 0,
                                  12);
        glEnableVertexArrayAttrib(mesh.vertex_array, 0);
        glVertexArrayAttribFormat(mesh.vertex_array, 0, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribBinding(mesh.vertex_array, 0, 0);
        if (cnt < UINT16_MAX) {
            mesh.index_type = GL_UNSIGNED_SHORT;
            mesh.mesh_count =
                rdiv * (hdiv * 2 + 1) + 2 * (rdiv / 2 * 5 + rdiv % 2 * 3);
            uint16 *viarr = (uint16*)malloc(sizeof(uint16) * mesh.mesh_count),
                   *cur = viarr;
            if (!viarr)
                throw std::bad_alloc();
            for (int i = 0; i < rdiv - 1; i++) {
                for (int j = 0; j < hdiv - 1; j++) {
                    int k = 2 + j + i * hdiv;
                    *(cur++) = static_cast<uint16>(k);
                    *(cur++) = static_cast<uint16>(k + hdiv);
                    *(cur++) = static_cast<uint16>(k + 1);
                }
                int k = 2 + hdiv - 1 + (rdiv - 1) * hdiv;
                *(cur++) = static_cast<uint16>(k);
                *(cur++) = static_cast<uint16>(k + hdiv);
                *(cur++) = UINT16_MAX;
            }
            int offset = (rdiv - 1) * (hdiv * 2 + 1);
            for (int j = 0; j < hdiv - 1; j++) {
                *(cur++) = static_cast<uint16>(j + 2 + offset);
                *(cur++) = static_cast<uint16>(j + 2);
                *(cur++) = static_cast<uint16>(j + 2 + 1);
            }
            *(cur++) = static_cast<uint16>(hdiv - 1 + 2 + offset);
            *(cur++) = static_cast<uint16>(hdiv - 1 + 2);
            *(cur++) = UINT16_MAX;

            int i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint16>(2 + (i - 2) * hdiv);
                *(cur++) = 1;
                *(cur++) = static_cast<uint16>(2 + (i - 1) * hdiv);
                *(cur++) = static_cast<uint16>(2 + i * hdiv);
                *(cur++) = UINT16_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) = static_cast<uint16>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
                *(cur++) = static_cast<uint16>(2 + hdiv);
            } else {
                *(cur++) = static_cast<uint16>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
            }

            i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint16>(2 + (i - 2) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint16>(2 + (i - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint16>(2 + i * hdiv + hdiv - 1);
                *(cur++) = UINT16_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) =
                    static_cast<uint16>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint16>(2 + hdiv - 1);
                *(cur++) = static_cast<uint16>(2 + 2 * hdiv - 1);
            } else {
                *(cur++) = 0;
                *(cur++) =
                    static_cast<uint16>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint16>(2 + hdiv - 1);
            }
            glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                                 GL_STATIC_READ);
            glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
            free(viarr);
        } else {
            mesh.index_type = GL_UNSIGNED_INT;
            mesh.mesh_count =
                rdiv * (hdiv * 2 + 1) + 2 * (rdiv / 2 * 5 + rdiv % 2 * 3);
            uint32 *viarr = (uint32*)malloc(sizeof(uint32) * mesh.mesh_count),
                   *cur = viarr;
            if (!viarr)
                throw std::bad_alloc();
            for (int i = 0; i < rdiv - 1; i++) {
                for (int j = 0; j < hdiv - 1; j++) {
                    int k = 2 + j + i * hdiv;
                    *(cur++) = static_cast<uint32>(k);
                    *(cur++) = static_cast<uint32>(k + hdiv);
                    *(cur++) = static_cast<uint32>(k + 1);
                }
                int k = 2 + hdiv - 1 + (rdiv - 1) * hdiv;
                *(cur++) = static_cast<uint32>(k);
                *(cur++) = static_cast<uint32>(k + hdiv);
                *(cur++) = UINT32_MAX;
            }
            int offset = (rdiv - 1) * (hdiv * 2 + 1);
            for (int j = 0; j < hdiv - 1; j++) {
                *(cur++) = static_cast<uint32>(j + 2 + offset);
                *(cur++) = static_cast<uint32>(j + 2);
                *(cur++) = static_cast<uint32>(j + 2 + 1);
            }
            *(cur++) = static_cast<uint32>(hdiv - 1 + 2 + offset);
            *(cur++) = static_cast<uint32>(hdiv - 1 + 2);
            *(cur++) = UINT32_MAX;

            int i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint32>(2 + (i - 2) * hdiv);
                *(cur++) = 1;
                *(cur++) = static_cast<uint32>(2 + (i - 1) * hdiv);
                *(cur++) = static_cast<uint32>(2 + i * hdiv);
                *(cur++) = UINT32_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) = static_cast<uint32>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
                *(cur++) = static_cast<uint32>(2 + hdiv);
            } else {
                *(cur++) = static_cast<uint32>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
            }

            i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint32>(2 + (i - 2) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint32>(2 + (i - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint32>(2 + i * hdiv + hdiv - 1);
                *(cur++) = UINT32_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) =
                    static_cast<uint32>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint32>(2 + hdiv - 1);
                *(cur++) = static_cast<uint32>(2 + 2 * hdiv - 1);
            } else {
                *(cur++) = 0;
                *(cur++) =
                    static_cast<uint32>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint32>(2 + hdiv - 1);
            }
            glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                                 GL_STATIC_READ);
            glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
            free(viarr);
        }
        free(vparr);
    } else if (df == VertexData::NORMAL) {
        mesh.primitive_type = GL_TRIANGLE_STRIP;
        mesh.restart_index = UINT32_MAX;
        mesh.InitIndexStatus(IndexStatus::RESTART_INDEX);
        int cnt = rdiv * hdiv + 2;
        float *vparr = (float*)malloc(sizeof(float) * 3 * 2 * cnt),
              *cur = vparr;
        if (!vparr)
            throw std::bad_alloc();
        *(cur++) = 0.0f;
        *(cur++) = 0.0f;
        *(cur++) = r;
        *(cur++) = 0.0f;
        *(cur++) = 0.0f;
        *(cur++) = -r;
        for (int i = 0; i < rdiv; i++) {
            float t = 2 * BL_MATH_PI / rdiv * i;
            Vector3f rvec(cosf(t), sinf(t), 0.0f);
            Vector3f zvec(0.0f, 0.0f, 1.0f);
            for (int j = 1; j < hdiv + 1; j++) {
                t = -BL_MATH_PI / 2 + BL_MATH_PI / (rdiv + 1) * j;
                Vector3f pos = rvec * cosf(t) + zvec * sinf(t);
                cur += 3;
                *(cur++) = pos.x();
                *(cur++) = pos.y();
                *(cur++) = pos.z();
                cur -= 6;
                pos *= r;
                *(cur++) = pos.x();
                *(cur++) = pos.y();
                *(cur++) = pos.z();
            }
        }
        glNamedBufferStorage(mesh.vertex_array, sizeof(vparr), vparr,
                             GL_STATIC_READ);
        glVertexArrayVertexBuffer(mesh.vertex_array, 0, mesh.vertex_buffer, 0,
                                  12);
        glEnableVertexArrayAttrib(mesh.vertex_array, 0);
        glEnableVertexArrayAttrib(mesh.vertex_array, 1);
        glVertexArrayAttribFormat(mesh.vertex_array, 0, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribFormat(mesh.vertex_array, 1, 3, GL_FLOAT, GL_FALSE,
                                  12);
        glVertexArrayAttribBinding(mesh.vertex_array, 0, 0);
        glVertexArrayAttribBinding(mesh.vertex_array, 1, 0);
        if (cnt < UINT16_MAX) {
            mesh.index_type = GL_UNSIGNED_SHORT;
            mesh.mesh_count =
                rdiv * (hdiv * 2 + 1) + 2 * (rdiv / 2 * 5 + rdiv % 2 * 3);
            uint16 *viarr = (uint16*)malloc(sizeof(uint16) * mesh.mesh_count),
                   *cur = viarr;
            if (!viarr)
                throw std::bad_alloc();
            for (int i = 0; i < rdiv - 1; i++) {
                for (int j = 0; j < hdiv - 1; j++) {
                    int k = 2 + j + i * hdiv;
                    *(cur++) = static_cast<uint16>(k);
                    *(cur++) = static_cast<uint16>(k + hdiv);
                    *(cur++) = static_cast<uint16>(k + 1);
                }
                int k = 2 + hdiv - 1 + (rdiv - 1) * hdiv;
                *(cur++) = static_cast<uint16>(k);
                *(cur++) = static_cast<uint16>(k + hdiv);
                *(cur++) = UINT16_MAX;
            }
            int offset = (rdiv - 1) * (hdiv * 2 + 1);
            for (int j = 0; j < hdiv - 1; j++) {
                *(cur++) = static_cast<uint16>(j + 2 + offset);
                *(cur++) = static_cast<uint16>(j + 2);
                *(cur++) = static_cast<uint16>(j + 2 + 1);
            }
            *(cur++) = static_cast<uint16>(hdiv - 1 + 2 + offset);
            *(cur++) = static_cast<uint16>(hdiv - 1 + 2);
            *(cur++) = UINT16_MAX;

            int i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint16>(2 + (i - 2) * hdiv);
                *(cur++) = 1;
                *(cur++) = static_cast<uint16>(2 + (i - 1) * hdiv);
                *(cur++) = static_cast<uint16>(2 + i * hdiv);
                *(cur++) = UINT16_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) = static_cast<uint16>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
                *(cur++) = static_cast<uint16>(2 + hdiv);
            } else {
                *(cur++) = static_cast<uint16>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
            }

            i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint16>(2 + (i - 2) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint16>(2 + (i - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint16>(2 + i * hdiv + hdiv - 1);
                *(cur++) = UINT16_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) =
                    static_cast<uint16>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint16>(2 + hdiv - 1);
                *(cur++) = static_cast<uint16>(2 + 2 * hdiv - 1);
            } else {
                *(cur++) = 0;
                *(cur++) =
                    static_cast<uint16>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint16>(2 + hdiv - 1);
            }
            glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                                 GL_STATIC_READ);
            glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
            free(viarr);
        } else {
            mesh.index_type = GL_UNSIGNED_INT;
            uint32 *viarr = (uint32*)malloc(
                       sizeof(uint32) * (rdiv * (hdiv * 2 + 1) +
                                         2 * (rdiv / 2 * 5 + rdiv % 2 * 3))),
                   *cur = viarr;
            if (!viarr)
                throw std::bad_alloc();
            for (int i = 0; i < rdiv - 1; i++) {
                for (int j = 0; j < hdiv - 1; j++) {
                    int k = 2 + j + i * hdiv;
                    *(cur++) = static_cast<uint32>(k);
                    *(cur++) = static_cast<uint32>(k + hdiv);
                    *(cur++) = static_cast<uint32>(k + 1);
                }
                int k = 2 + hdiv - 1 + (rdiv - 1) * hdiv;
                *(cur++) = static_cast<uint32>(k);
                *(cur++) = static_cast<uint32>(k + hdiv);
                *(cur++) = UINT32_MAX;
            }
            int offset = (rdiv - 1) * (hdiv * 2 + 1);
            for (int j = 0; j < hdiv - 1; j++) {
                *(cur++) = static_cast<uint32>(j + 2 + offset);
                *(cur++) = static_cast<uint32>(j + 2);
                *(cur++) = static_cast<uint32>(j + 2 + 1);
            }
            *(cur++) = static_cast<uint32>(hdiv - 1 + 2 + offset);
            *(cur++) = static_cast<uint32>(hdiv - 1 + 2);
            *(cur++) = UINT32_MAX;

            int i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint32>(2 + (i - 2) * hdiv);
                *(cur++) = 1;
                *(cur++) = static_cast<uint32>(2 + (i - 1) * hdiv);
                *(cur++) = static_cast<uint32>(2 + i * hdiv);
                *(cur++) = UINT32_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) = static_cast<uint32>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
                *(cur++) = static_cast<uint32>(2 + hdiv);
            } else {
                *(cur++) = static_cast<uint32>(2 + (rdiv - 1) * hdiv);
                *(cur++) = 1;
                *(cur++) = 2;
            }

            i = rdiv - 1;
            while (i > 1) {
                *(cur++) = static_cast<uint32>(2 + (i - 2) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint32>(2 + (i - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint32>(2 + i * hdiv + hdiv - 1);
                *(cur++) = UINT32_MAX;
                i -= 2;
            }
            if (i == 1) {
                *(cur++) =
                    static_cast<uint32>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = 0;
                *(cur++) = static_cast<uint32>(2 + hdiv - 1);
                *(cur++) = static_cast<uint32>(2 + 2 * hdiv - 1);
            } else {
                *(cur++) = 0;
                *(cur++) =
                    static_cast<uint32>(2 + (rdiv - 1) * hdiv + hdiv - 1);
                *(cur++) = static_cast<uint32>(2 + hdiv - 1);
            }
            glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                                 GL_STATIC_READ);
            glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
            free(viarr);
        }
        free(vparr);
    }
}
// void MeshMaker::MakeTorus(Mesh& mesh,
//                             float r,
//                             float d,
//                             int rdiv,
//                             int hdiv,
//                             VertexData df) {}
void MeshMaker::MakeCubord(Mesh& mesh, float a, float b, float c, VertexData df) {
    if (df == VertexData::POSITION) {
        mesh.primitive_type = GL_TRIANGLE_STRIP;
        mesh.restart_index = UINT16_MAX;
        mesh.index_type = GL_UNSIGNED_SHORT;
        mesh.mesh_count = 17;
        mesh.InitIndexStatus(IndexStatus::RESTART_INDEX);
        a /= 2.0f;
        b /= 2.0f;
        c /= 2.0f;
        const float vparr[24]{
            -a, b,  c,  a, b,  c,  -a, b,  -c, a, b,  -c,
            -a, -b, -c, a, -b, -c, -a, -b, c,  a, -b, c,
        };
        const uint16 viarr[17]{0, 1, 2, 3, 4, 5, 6, 7, UINT16_MAX,
                               2, 4, 0, 6, 1, 7, 3, 5};
        glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                             GL_STATIC_READ);
        glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
        glNamedBufferStorage(mesh.vertex_array, sizeof(vparr), vparr,
                             GL_STATIC_READ);
        glVertexArrayVertexBuffer(mesh.vertex_array, 0, mesh.vertex_buffer, 0,
                                  12);
        glEnableVertexArrayAttrib(mesh.vertex_array, 0);
        glVertexArrayAttribFormat(mesh.vertex_array, 0, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribBinding(mesh.vertex_array, 0, 0);
    } else if (df == VertexData::NORMAL) {
        mesh.primitive_type = GL_TRIANGLE_STRIP;
        mesh.restart_index = UINT16_MAX;
        mesh.index_type = GL_UNSIGNED_SHORT;
        mesh.mesh_count = 17;
        mesh.InitIndexStatus(IndexStatus::RESTART_INDEX);
        a /= 2.0f;
        b /= 2.0f;
        c /= 2.0f;
        const float vparr[144]{
            -a, b,  c,  0.0f,  1.0f,  0.0f,  a,  b,  c,  0.0f,  1.0f,  0.0f,
            -a, b,  -c, 0.0f,  1.0f,  0.0f,  a,  b,  -c, 0.0f,  1.0f,  0.0f,

            -a, b,  -c, 0.0f,  0.0f,  -1.0f, a,  b,  -c, 0.0f,  0.0f,  -1.0f,
            -a, -b, -c, 0.0f,  0.0f,  -1.0f, a,  -b, -c, 0.0f,  0.0f,  -1.0f,

            -a, -b, -c, 0.0f,  -1.0f, 0.0f,  a,  -b, -c, 0.0f,  -1.0f, 0.0f,
            -a, -b, c,  0.0f,  -1.0f, 0.0f,  a,  -b, c,  0.0f,  -1.0f, 0.0f,

            -a, b,  c,  0.0f,  0.0f,  1.0f,  a,  b,  c,  0.0f,  0.0f,  1.0f,
            -a, -b, c,  0.0f,  0.0f,  1.0f,  a,  -b, c,  0.0f,  0.0f,  1.0f,

            -a, b,  c,  -1.0f, 0.0f,  0.0f,  -a, -b, c,  -1.0f, 0.0f,  0.0f,
            -a, b,  -c, -1.0f, 0.0f,  0.0f,  -a, -b, -c, -1.0f, 0.0f,  0.0f,

            a,  b,  c,  1.0f,  0.0f,  0.0f,  a,  -b, c,  1.0f,  0.0f,  0.0f,
            a,  b,  -c, 1.0f,  0.0f,  0.0f,  a,  -b, -c, 1.0f,  0.0f,  0.0f};
        const uint16 viarr[]{
            0,  1,  2,  3,  UINT16_MAX, 4,  5,  6,  7,  UINT16_MAX,
            8,  9,  10, 11, UINT16_MAX, 12, 13, 14, 15, UINT16_MAX,
            16, 17, 18, 19, UINT16_MAX, 20, 21, 22, 23, UINT16_MAX};
        glNamedBufferStorage(mesh.index_buffer, sizeof(viarr), viarr,
                             GL_STATIC_READ);
        glVertexArrayElementBuffer(mesh.vertex_array, mesh.index_buffer);
        glNamedBufferStorage(mesh.vertex_array, sizeof(vparr), vparr,
                             GL_STATIC_READ);
        glVertexArrayVertexBuffer(mesh.vertex_array, 0, mesh.vertex_buffer, 0,
                                  24);
        glEnableVertexArrayAttrib(mesh.vertex_array, 0);
        glEnableVertexArrayAttrib(mesh.vertex_array, 1);
        glVertexArrayAttribFormat(mesh.vertex_array, 0, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribFormat(mesh.vertex_array, 1, 3, GL_FLOAT, GL_FALSE,
                                  0);
        glVertexArrayAttribBinding(mesh.vertex_array, 0, 0);
        glVertexArrayAttribBinding(mesh.vertex_array, 1, 0);
    }
}
}