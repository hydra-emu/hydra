#ifndef TKP_STARTPARAMETERS_H
#define TKP_STARTPARAMETERS_H
#include <string>
namespace TKPEmu {
    struct StartParameters {
       std::string RomDir;
       std::string RomFile;
       std::string GenerationPath;
       bool Recursive = false;
       bool Parallel = false;
       bool Verbose = false;
       bool GenerateTestResults = false;
       bool Webserver = false;
       bool Screenshot = false;
    };
}
#endif