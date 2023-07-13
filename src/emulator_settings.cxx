#include <include/emulator_settings.hxx>
#include <include/emulator_factory.h>

GeneralSettings EmulatorSettings::general_settings_ {
    hydra::EmulatorFactory::GetSavePath() + "settings.json"
};
EmulatorDataMap EmulatorSettings::emulator_data_ {};