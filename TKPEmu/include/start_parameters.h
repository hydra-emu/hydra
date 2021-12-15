#ifndef TKPEMU_STARTPARAMETERS_H
#define TKPEMU_STARTPARAMETERS_H
namespace TKPEmu {
    struct StartParameters {
       std::string RomDir;
       std::string RomFile;
       bool Recursive = false;
       bool Parallel = false;
    };
}
#endif