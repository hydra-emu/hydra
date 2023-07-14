#include <emulator_factory.hxx>
#include <emulator_settings.hxx>

GeneralSettings EmulatorSettings::general_settings_{hydra::EmulatorFactory::GetSavePath() +
                                                    "settings.json"};
EmulatorDataMap EmulatorSettings::emulator_data_{};