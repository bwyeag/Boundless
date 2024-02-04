#ifndef _RENDER_HPP_FILE_
#define _RENDER_HPP_FILE_
#include <iostream>
#include <unordered_map>

#define ERROR(type, info) std::cerr << "[ERROR][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl
#define ERRORINFO(info) std::cerr << "[ERROR]other:" << info << std::endl
#define WARNING(type, info) std::cerr << "[WARNING][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl
#define WARNINGINFO(info) std::cerr << "[WARNING]other:" << info << std::endl
#define INFO(type, info) std::cout << "[INFO][" << type << "]file:" << __FILE__ << ";line:" << __LINE__ << "|info:" << info << std::endl
#define INFODATA(info) std::cout << "[INFO]other:" << info << std::endl

namespace Boundless::Render
{

} // namespace Boundless::Render
#endif //!_RENDER_HPP_FILE_