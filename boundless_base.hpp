#ifndef _BOUNDLESS_HPP_FILE_
#define _BOUNDLESS_HPP_FILE_

#include "glad/glad.h"
#include "glfw/glfw3.h"

#include "bl_data_struct.hpp"
#include "bl_log.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "Eigen/Core"
#include "Eigen/Geometry"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#define BL_MATH_PI 3.1415926535897932384626

namespace zlib {
#include <zlib.h>
class ZlibException : public std::exception {
   private:
    int code;

   public:
    ZlibException(int code) { this->code = code; }
    const char* what() const noexcept {
        if (code == Z_MEM_ERROR) {
            return "zlib memory error";
        } else if (code == Z_BUF_ERROR) {
            return "zlib buffer out off range";
        } else if (code == Z_DATA_ERROR) {
            return "zlib data corrupted";
        } else {
            return "unknown";
        }
    }
};
}  // namespace zlib

using Eigen::Matrix3d;
using Eigen::Matrix3f;
using Eigen::Matrix4d;
using Eigen::Matrix4f;
using Eigen::Quaterniond;
using Eigen::Quaternionf;
using Eigen::Vector3d;
using Eigen::Vector3f;
using Eigen::Vector4d;
using Eigen::Vector4f;

namespace Boundless {
// typedef
typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int6;
typedef std::uint8_t Byte;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

///////////////////////////////////////////////
// 压缩算法
//
const int32 compress_level = 7;
/* 数据结构:|压缩前长度8Byte|压缩后长度8Byte|压缩数据| */
// 压缩数据,data为压缩前数据指针,length为数据长度,结束后变为压缩后长度,space在开头预留space字节的空间
Byte* CompressData(const Byte* data, size_t* length, size_t space = 0ULL);
// 解压缩数据,data为压缩后数据指针,ret_length返回数据长度
Byte* UncompressData(const Byte* data, size_t* ret_length);
}  // namespace Boundless
#endif  //!_BOUNDLESS_HPP_FILE_