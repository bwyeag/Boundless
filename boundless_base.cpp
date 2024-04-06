#include "boundless_base.hpp"

namespace Boundless {
Byte* CompressData(const Byte* data, size_t* length, size_t space) {
    // 计算压缩容量
    size_t dlen = zlib::compressBound(*length) + sizeof(uint64) * 2;
    Byte* compress_data = (Byte*)malloc(space + dlen);
    if (compress_data == nullptr) {
        throw std::bad_alloc();
    }
    Byte* data_ptr = compress_data + space;
    *(uint64*)data_ptr = *length;    // 存入压缩前长度
    data_ptr += sizeof(uint64) * 2;  // 移动到压缩数据的开头
    *(uint64*)(data_ptr - sizeof(uint64)) =
        dlen - sizeof(uint64) * 2;  // 存入压缩后长度(作为缓冲区长度)
    int res =
        zlib::compress2(data_ptr, (zlib::uLong*)(data_ptr - sizeof(uint64)),
                        data, *length, compress_level);
    if (res != Z_OK) {
        free(compress_data);
        throw zlib::ZlibException(res);
    }
    *length = sizeof(uint64) * 2 + space +
              *(uint64*)(data_ptr - sizeof(uint64));  // 返回长度
    return compress_data;
}
Byte* UncompressData(const Byte* data, size_t* ret_length) {
    Byte* uncompress_data =
        (Byte*)malloc(*(uint64*)data);  // 按压缩前长度分配空间
    if (uncompress_data == nullptr)
        {throw std::bad_alloc();}
    *ret_length = *(uint64*)data;
    int res = zlib::uncompress2(uncompress_data, (zlib::uLong*)(ret_length),
                                data + sizeof(uint64) * 2,
                                (zlib::uLong*)(data + sizeof(uint64)));
    if (res != Z_OK) {
        free(uncompress_data);
        throw zlib::ZlibException(res);
    }
    return uncompress_data;
}
}  // namespace Boundless
