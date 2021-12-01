#ifndef TKPEMU_STARTPARAMETERS_H
#define TKPEMU_STARTPARAMETERS_H
namespace TKPEmu {
    struct StartParameters {
       unsigned long long ScreenshotTime = 0;
       std::string ScreenshotDir;
       std::string RomFile;
    };
}
#endif