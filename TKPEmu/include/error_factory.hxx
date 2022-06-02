#ifndef TKPEMU_ERROR_FACTORY_H
#define TKPEMU_ERROR_FACTORY_H
#include <stdexcept>
#include <sstream>

namespace ErrorFactory {
    static std::runtime_error generate_exception(std::string func_name, int line_num, std::string desc) {
        std::stringstream ss;
        ss << "ERROR - " << func_name << ", line " << line_num << ":\n";
        ss << desc;
        ss << "\n";
        return std::runtime_error(ss.str());
    }
}
#endif