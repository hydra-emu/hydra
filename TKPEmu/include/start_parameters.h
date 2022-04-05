#ifndef TKPEMU_STARTPARAMETERS_H
#define TKPEMU_STARTPARAMETERS_H
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