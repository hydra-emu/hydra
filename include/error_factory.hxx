#ifndef TKP_ERROR_FACTORY_H
#define TKP_ERROR_FACTORY_H
#include <stdexcept>
#include <sstream>

namespace ErrorFactory {
    static std::runtime_error generate_exception(const std::string& func_name, int line_num, const std::string& desc) {
        std::stringstream ss;
        ss << "ERROR - " << func_name << ", line " << line_num << ":\n";
        ss << desc;
        ss << "\n";
        return std::runtime_error(ss.str());
    }
}
#endif