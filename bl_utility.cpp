/*
 * BOUNDLESS Engine File
 * Create: 2023/12/2/18:15
 * OpenGL Render C++ File
 *
 */

#include "bl_utility.hpp"

namespace Boundless
{
    //////////////////////////////////////////////////////////////////
    //  OpenGL 类
    //  OpenGL Classes
    //
    Buffer::Buffer()
    {
        glCreateBuffers(1, &(this->buffer_id));
    }
    Buffer::Buffer(GLenum target)
    {
        glCreateBuffers(1, &(this->buffer_id));
        this->buffer_target = target;
    }
    Buffer::Buffer(GLenum target, GLsizeiptr size, GLbitfield flags, const void *data)
        : Buffer(target)
    {
        StoreData(size, flags, data);
    }
    Buffer::~Buffer()
    {
        glDeleteBuffers(1, &(this->buffer_id));
        this->buffer_id = 0;
    }

    void Buffer::Bind()
    {
        glBindBuffer(this->buffer_id, this->buffer_target);
    }
    void Buffer::Unbind()
    {
        glBindBuffer(0, this->buffer_target);
    }
    void Buffer::Bind(GLenum tar)
    {
        glBindBuffer(this->buffer_id, tar);
    }
    void Buffer::Unbind(GLenum tar)
    {
        glBindBuffer(0, tar);
    }
    void Buffer::SetTarget(GLenum tar)
    {
        this->buffer_target = tar;
    }

    void Buffer::StoreData(GLsizeiptr size, GLbitfield flags, const void *data)
    {
        buffer_size = size;
        buffer_storeflags = flags;
        glNamedBufferStorage(buffer_id, size, data, flags);
    }
    void Buffer::StoreSubData(const data_range &range, const void *data)
    {
        glNamedBufferSubData(buffer_id, range.offset, range.length, data);
    }
    void Buffer::FillData(GLenum internal_format, GLenum format, GLenum type, const void *data)
    {
        glClearNamedBufferData(buffer_id, internal_format, format, type, data);
    }
    void Buffer::FillSubData(const data_range &range, GLenum internal_format, GLenum format, GLenum type, const void *data)
    {
        glClearNamedBufferSubData(buffer_id, internal_format, range.offset, range.length, format, type, data);
    }

    void Buffer::CopyData(Buffer &read, Buffer &write, const data_range &read_range, GLintptr write_offset)
    {
        glCopyNamedBufferSubData(read.buffer_id, write.buffer_id, read_range.offset, read_range.length, write_offset);
    }

    void Buffer::GetData(const data_range &range, void *write_to)

    {
        glGetNamedBufferSubData(buffer_id, range.offset, range.length, write_to);
    }

    void *Buffer::Map(GLenum access)
    {
        map_access = access;
        map_range = {0, buffer_size};
        return glMapNamedBuffer(buffer_id, access);
    }
    void Buffer::Unmap()
    {
        glUnmapNamedBuffer(buffer_id);
    }
    void *Buffer::MapSub(const data_range &range, GLbitfield flags)
    {
        map_bitfield = flags;
        map_range = range;
        return glMapNamedBufferRange(buffer_id, range.offset, range.length, flags);
    }
    void Buffer::FlushMap(const data_range &range)
    {
        glFlushMappedNamedBufferRange(buffer_id, range.offset, range.length);
    }

    void Buffer::Invalidate()
    {
        buffer_size = 0;
        buffer_storeflags = 0;
        glInvalidateBufferData(buffer_id);
    }
    void Buffer::InvalidateSub(const data_range &range)
    {
        glInvalidateBufferSubData(buffer_id, range.offset, range.length);
    }
    Buffer::operator GLuint()
    {
        return buffer_id;
    }
    layout_format::layout_format(GLenum type, GLuint count, GLboolean normalize)
        : type(type), size(count * static_cast<GLuint>(openglTypeSize(type)))
    {
        this->info = 0x00000000;
        if (normalize == GL_TRUE)
        {
            this->info |= 0xFF000000;
        }
        this->info |= static_cast<GLubyte>(count);
    }
    layout_format &layout_format::operator=(layout_format &target)
    {
        if (&target == this)
        {
            return *this;
        }
        type = target.type;
        size = target.size;
        info = target.info;
        return *this;
    }
    layout_format &layout_format::operator=(layout_format &&target)
    {
        if (&target == this)
        {
            return *this;
        }
        type = target.type;
        size = target.size;
        info = target.info;
        return *this;
    }
    void layout_format::set_count(GLuint count) const
    {
#ifdef _DEBUG
        if (count > 0xFF)
        {
            WARNING("INPUT", "count过大")
        }
#endif
        this->info &= 0xFFFFFF00;
        this->info |= static_cast<GLubyte>(count);
    }

    GLuint layout_format::get_count() const
    {
        return this->info & 0x000000FF;
    }
    GLboolean layout_format::is_normalised() const
    {
        return (this->info & 0xFF000000) != 0;
    }
    void layout_format::set_normalised(bool enable) const
    {
        if (enable)
        {
            this->info |= 0xFF000000;
        }
        else
        {
            this->info &= 0x00FFFFFF;
        }
    }
    GLboolean layout_format::is_enable() const
    {
        return (this->info & 0x0000FF00) != 0;
    }
    void layout_format::set_enable(bool enable) const
    {
        if (enable)
        {
            this->info |= 0x0000FF00;
        }
        else
        {
            this->info &= 0xFFFF00FF;
        }
    }
    void layout_format::set_index(GLuint index) const
    {
#ifdef _DEBUG
        if (index > 0xFF)
        {
            WARNING("INPUT","index过大")
        } 
#endif      
        this->info |= (index << 16);
    }
    GLuint layout_format::get_index() const
    {
        return (this->info & 0x00FF0000) >> 16;
    }
    IndexBuffer::IndexBuffer()
        : Buffer(GL_ELEMENT_ARRAY_BUFFER)
    {
    }
    IndexBuffer::IndexBuffer(GLenum draw_type, GLenum index_type, const void *data, GLsizeiptr size, GLbitfield flags)
        : Buffer(GL_ELEMENT_ARRAY_BUFFER, size, flags, data), draw_type(draw_type), index_type(index_type),
          index_count(size / openglTypeSize(index_type))
    {
    }
    IndexBuffer::IndexBuffer(GLsizeiptr size, GLbitfield flags)
        : Buffer(GL_ELEMENT_ARRAY_BUFFER, size, flags)
    {
    }
    void IndexBuffer::SetData(GLenum draw_type, GLenum index_type)
    {
        this->SetDrawType(draw_type);
        this->SetDrawType(index_type);
    }
    void IndexBuffer::SetDrawType(GLenum draw_type)
    {
        this->draw_type = draw_type;
    }
    void IndexBuffer::SetDataType(GLenum index_type)
    {
        this->index_type = index_type;
        this->index_count = this->buffer_size / openglTypeSize(index_type);
    }
    GLenum IndexBuffer::GetDrawType() const
    {
        return this->draw_type;
    }
    GLenum IndexBuffer::GetIndexType() const
    {
        return this->index_type;
    }
    GLsizei IndexBuffer::GetIndexCount() const
    {
        return this->index_count;
    }
    IndexBuffer::operator GLuint() const
    {
        return this->buffer_id;
    }
    VertexBuffer::VertexBuffer(uint32_t reserve)
        : Buffer(GL_ARRAY_BUFFER)
    {
        this->vertex_layout.reserve(reserve);
    }
    VertexBuffer::VertexBuffer(GLsizeiptr size,
                               GLbitfield flags,
                               uint32_t reserve,
                               const void *data)
        : Buffer(GL_ARRAY_BUFFER, size, flags, data)
    {
        vertex_layout.reserve(reserve);
    }

    VertexBuffer &VertexBuffer::operator<<(const layout_format &data)
    {
        this->vertex_size += data.size;
        this->vertex_layout.push_back(data);
        return *this;
    }
    VertexBuffer::operator GLuint() const
    {
        return this->buffer_id;
    }
    GLsizei VertexBuffer::GetSize() const
    {
        return this->vertex_size;
    }
    const std::vector<layout_format> &VertexBuffer::GetLayout() const
    {
        return this->vertex_layout;
    }

    UniformBuffer::UniformBuffer()
        : Buffer(GL_UNIFORM_BUFFER)
    {
    }
    UniformBuffer::UniformBuffer(const void *data, GLsizeiptr size, GLbitfield flags)
        : Buffer(GL_UNIFORM_BUFFER, size, flags, data)
    {
    }
    UniformBuffer::UniformBuffer(GLsizeiptr size, GLbitfield flags)
        : Buffer(GL_UNIFORM_BUFFER, size, flags)
    {
    }
    void UniformBuffer::BindTo(GLuint index)
    {
        binding = index;
        glBindBufferBase(GL_UNIFORM_BUFFER, index, buffer_id);
    }

    ShaderStorageBuffer::ShaderStorageBuffer()
        : Buffer(GL_SHADER_STORAGE_BUFFER)
    {
    }
    ShaderStorageBuffer::ShaderStorageBuffer(const void *data, GLsizeiptr size, GLbitfield flags)
        : Buffer(GL_SHADER_STORAGE_BUFFER, size, flags, data)
    {
    }
    ShaderStorageBuffer::ShaderStorageBuffer(GLsizeiptr size, GLbitfield flags)
        : Buffer(GL_SHADER_STORAGE_BUFFER, size, flags)
    {
    }
    void ShaderStorageBuffer::BindTo(GLuint index)
    {
        binding = index;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffer_id);
    }
    VertexArray::VertexArray()
    {
        glCreateVertexArrays(1, &this->array_id);
    }

    void VertexArray::Use(VertexBuffer &target, const data_range &range, bool enable, bool set)
    {
        size_t offset = 0;
        for (GLuint i = range.offset; i < range.offset + range.length; i++)
        {
            const layout_format &val = target.GetLayout()[i];
            if ((val.is_normalised() == GL_FALSE) && openglIsInteger(val.type))
            {
                glVertexAttribIPointer(i, val.get_count(), val.type, target.GetSize(),
                                       (const void *)offset);
            }
            else if (val.type == GL_DOUBLE)
            {
                glVertexAttribLPointer(i, val.get_count(), val.type, target.GetSize(),
                                       (const void *)offset);
            }
            else
            {
                glVertexAttribPointer(i, val.get_count(), val.type, val.is_normalised(),
                                      target.GetSize(), (const void *)offset);
            }
            if (set)
            {
                val.set_index(layout_index);
                val.set_enable(enable);
            }
            if (enable)
            {
                glEnableVertexAttribArray(this->layout_index);
            }
            this->layout_index++;
            offset += val.size;
        }
    }
    inline void VertexArray::UseAll(VertexBuffer &target, bool enable, bool set)
    {
        this->Use(target, {0, GLsizei(target.GetLayout().size())}, enable,set);
    }
    inline void VertexArray::Enable(GLuint index)
    {
#ifdef _DEBUG
        assert(index <= layout_index);
#endif
        glEnableVertexAttribArray(index);
    }
    inline void VertexArray::Disable(GLuint index)
    {
#ifdef _DEBUG
        assert(index <= layout_index);
#endif
        glDisableVertexAttribArray(index);
    }
    inline void VertexArray::Bind() const
    {
        glBindVertexArray(this->array_id);
    }
    inline void VertexArray::Unbind() const
    {
        glBindVertexArray(0);
    }
    void VertexArray::SetStatic(GLuint index, const layout_format &val, const void *v)
    {
        if (openglIsInteger(val.type))
        {
            if ((val.is_normalised() == GL_FALSE))
            {
                if (val.type == GL_INT)
                {
                    switch (val.get_count())
                    {
                    case 1:
                        glVertexAttribI1iv(index, (const GLint *)v);
                        return;
                    case 2:
                        glVertexAttribI2iv(index, (const GLint *)v);
                        return;
                    case 3:
                        glVertexAttribI3iv(index, (const GLint *)v);
                        return;
                    case 4:
                        glVertexAttribI4iv(index, (const GLint *)v);
                        return;
                    default:
                        throw;
                    }
                }
                else if (GL_UNSIGNED_BYTE)
                {
                    switch (val.get_count())
                    {
                    case 1:
                        glVertexAttribI1uiv(index, (const GLuint *)v);
                        return;
                    case 2:
                        glVertexAttribI2uiv(index, (const GLuint *)v);
                        return;
                    case 3:
                        glVertexAttribI3uiv(index, (const GLuint *)v);
                        return;
                    case 4:
                        glVertexAttribI4uiv(index, (const GLuint *)v);
                        return;
                    default:
                        throw;
                    }
                }
                else if (val.get_count() == 4)
                {
                    switch (val.type)
                    {
                    case GL_BYTE:
                        glVertexAttribI4bv(index, (const GLbyte *)v);
                        return;
                    case GL_UNSIGNED_BYTE:
                        glVertexAttribI4ubv(index, (const GLubyte *)v);
                        return;
                    case GL_SHORT:
                        glVertexAttribI4sv(index, (const GLshort *)v);
                        return;
                    case GL_UNSIGNED_SHORT:
                        glVertexAttribI4usv(index, (const GLushort *)v);
                        return;
                    default:
                        throw;
                    }
                }
                else
                {
                    throw;
                }
            }
            else
            {
                switch (val.type)
                {
                case GL_BYTE:
                    glVertexAttrib4Nbv(index, (const GLbyte *)v);
                    return;
                case GL_UNSIGNED_BYTE:
                    glVertexAttrib4Nubv(index, (const GLubyte *)v);
                    return;
                case GL_SHORT:
                    glVertexAttrib4Nsv(index, (const GLshort *)v);
                    return;
                case GL_UNSIGNED_SHORT:
                    glVertexAttrib4Nusv(index, (const GLushort *)v);
                    return;
                case GL_INT:
                    glVertexAttrib4Niv(index, (const GLint *)v);
                    return;
                case GL_UNSIGNED_INT:
                    glVertexAttrib4Nuiv(index, (const GLuint *)v);
                    return;
                default:
                    break;
                }
            }
        }
        if (val.type == GL_DOUBLE)
        {
            switch (val.get_count())
            {
            case 1:
                glVertexAttribL1dv(index, (const GLdouble *)v);
                return;
            case 2:
                glVertexAttribL1dv(index, (const GLdouble *)v);
                return;
            case 3:
                glVertexAttribL3dv(index, (const GLdouble *)v);
                return;
            case 4:
                glVertexAttribL4dv(index, (const GLdouble *)v);
                return;
            default:
                break;
            }
        }
        switch (val.get_count())
        {
        case 1:
            switch (val.type)
            {
            case GL_FLOAT:
                glVertexAttrib1fv(index, (const GLfloat *)v);
                return;
            case GL_DOUBLE:
                glVertexAttrib1dv(index, (const GLdouble *)v);
                return;
            case GL_SHORT:
                glVertexAttrib1sv(index, (const GLshort *)v);
                return;
            default:
                throw;
            }
        case 2:
            switch (val.type)
            {
            case GL_FLOAT:
                glVertexAttrib2fv(index, (const GLfloat *)v);
                return;
            case GL_DOUBLE:
                glVertexAttrib2dv(index, (const GLdouble *)v);
                return;
            case GL_SHORT:
                glVertexAttrib2sv(index, (const GLshort *)v);
                return;
            default:
                throw;
            }
        case 3:
            switch (val.type)
            {
            case GL_FLOAT:
                glVertexAttrib3fv(index, (const GLfloat *)v);
                return;
            case GL_DOUBLE:
                glVertexAttrib3dv(index, (const GLdouble *)v);
                return;
            case GL_SHORT:
                glVertexAttrib3sv(index, (const GLshort *)v);
                return;
            default:
                throw;
            }
        case 4:
            switch (val.type)
            {
            case GL_FLOAT:
                glVertexAttrib4fv(index, (const GLfloat *)v);
                return;
            case GL_DOUBLE:
                glVertexAttrib4dv(index, (const GLdouble *)v);
                return;
            case GL_SHORT:
                glVertexAttrib4sv(index, (const GLshort *)v);
                return;
            case GL_BYTE:
                glVertexAttrib4bv(index, (const GLbyte *)v);
                return;
            case GL_UNSIGNED_BYTE:
                glVertexAttrib4ubv(index, (const GLubyte *)v);
                return;
            case GL_INT:
                glVertexAttrib4iv(index, (const GLint *)v);
                return;
            case GL_UNSIGNED_INT:
                glVertexAttrib4uiv(index, (const GLuint *)v);
                return;
            default:
                throw;
            }
        default:
            throw;
        }
    }
} // namespace Boundless