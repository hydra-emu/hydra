#pragma once
#ifndef TKP_SETTINGS_H
#define TKP_SETTINGS_H
#include "base_application.h"
#include "application_settings.h"
namespace TKPEmu::Applications {
    struct Settings : public IMApplication {
    private:
        AppSettings* current_settings_;
        float* sleep_time_ = nullptr;
        bool* rom_loaded_ = nullptr;
    public:
        Settings(AppSettings* current_settings, float* sleep_time, bool* rom_loaded) : current_settings_(current_settings), sleep_time_(sleep_time), rom_loaded_(rom_loaded) {}
        static auto GetFpsValue(int i) {
            static const int fps_values[] = { 30, 60, 120, 144, 240 };
            return fps_values[i];
        }
        void Draw(const char* title, bool* p_open = NULL) noexcept final override
        {
            if (!ImGui::Begin(title, p_open))
            {
                ImGui::End();
                return;
            }
            if (ImGui::CollapsingHeader("Video")) {
                ImGui::Text("General video settings:");
                ImGui::Checkbox("FPS Limit", (bool*)(&(current_settings_->limit_fps)));
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("Enable to save CPU cycles.");
                    ImGui::EndTooltip();
                }
                if (current_settings_->limit_fps) {
                    ImGui::SameLine();
                    static const char* fps_types[] = { "30", "60", "120", "144", "240" };
                    if (ImGui::Combo("", (int*)(&current_settings_->max_fps_index), fps_types, IM_ARRAYSIZE(fps_types))) {
                        *sleep_time_ = 1000.0f / GetFpsValue(current_settings_->max_fps_index);
                    }
                }
                ImGui::Separator();
            }
            if (ImGui::CollapsingHeader("Emulation")) {
                ImGui::Text("General emulation settings:");
                if (*rom_loaded_) {
                    ImGui::Text("Some settings require the emulation to be stopped");
                }
                else {
                    ImGui::Checkbox("Debug mode", (bool*)(&(current_settings_->debug_mode)));
                }
                ImGui::Separator();
            }
            if (ImGui::CollapsingHeader("Gameboy")) {
                ImGui::Text("Palette:");
                static float c1[3] = { (float)current_settings_->dmg_color0_r / 255, (float)current_settings_->dmg_color0_g / 255, (float)current_settings_->dmg_color0_b / 255 };
                static float c2[3] = { (float)current_settings_->dmg_color1_r / 255, (float)current_settings_->dmg_color1_g / 255, (float)current_settings_->dmg_color1_b / 255 };
                static float c3[3] = { (float)current_settings_->dmg_color2_r / 255, (float)current_settings_->dmg_color2_g / 255, (float)current_settings_->dmg_color2_b / 255 };
                static float c4[3] = { (float)current_settings_->dmg_color3_r / 255, (float)current_settings_->dmg_color3_g / 255, (float)current_settings_->dmg_color3_b / 255 };
                if (ImGui::ColorEdit3("Color 1", c1, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
                    current_settings_->dmg_color0_r = (int)(c1[0] * 255.0f);
                    current_settings_->dmg_color0_g = (int)(c1[1] * 255.0f);
                    current_settings_->dmg_color0_b = (int)(c1[2] * 255.0f);
                }
                ImGui::SameLine();
                if (ImGui::ColorEdit3("Color 2", c2, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
                    current_settings_->dmg_color1_r = (int)(c2[0] * 255.0f);
                    current_settings_->dmg_color1_g = (int)(c2[1] * 255.0f);
                    current_settings_->dmg_color1_b = (int)(c2[2] * 255.0f);
                }
                ImGui::SameLine();
                if (ImGui::ColorEdit3("Color 3", c3, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
                    current_settings_->dmg_color2_r = (int)(c3[0] * 255.0f);
                    current_settings_->dmg_color2_g = (int)(c3[1] * 255.0f);
                    current_settings_->dmg_color2_b = (int)(c3[2] * 255.0f);
                }
                ImGui::SameLine();
                if (ImGui::ColorEdit3("Color 4", c4, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
                    current_settings_->dmg_color3_r = (int)(c4[0] * 255.0f);
                    current_settings_->dmg_color3_g = (int)(c4[1] * 255.0f);
                    current_settings_->dmg_color3_b = (int)(c4[2] * 255.0f);
                }
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Click on a color to open the color picker.");
                }
                if (ImGui::Button("Reset")) {
                    static AppSettings orig;
                    current_settings_->dmg_color0_r = orig.dmg_color0_r;
                    current_settings_->dmg_color0_g = orig.dmg_color0_g;
                    current_settings_->dmg_color0_b = orig.dmg_color0_b;
                    current_settings_->dmg_color1_r = orig.dmg_color1_r;
                    current_settings_->dmg_color1_g = orig.dmg_color1_g;
                    current_settings_->dmg_color1_b = orig.dmg_color1_b;
                    current_settings_->dmg_color2_r = orig.dmg_color2_r;
                    current_settings_->dmg_color2_g = orig.dmg_color2_g;
                    current_settings_->dmg_color2_b = orig.dmg_color2_b;
                    current_settings_->dmg_color3_r = orig.dmg_color3_r;
                    current_settings_->dmg_color3_g = orig.dmg_color3_g;
                    current_settings_->dmg_color3_b = orig.dmg_color3_b;
                    c1[0] = (float)current_settings_->dmg_color0_r / 255;
                    c1[1] = (float)current_settings_->dmg_color0_g / 255;
                    c1[2] = (float)current_settings_->dmg_color0_b / 255;
                    c2[0] = (float)current_settings_->dmg_color1_r / 255;
                    c2[1] = (float)current_settings_->dmg_color1_g / 255;
                    c2[2] = (float)current_settings_->dmg_color1_b / 255;
                    c3[0] = (float)current_settings_->dmg_color2_r / 255;
                    c3[1] = (float)current_settings_->dmg_color2_g / 255;
                    c3[2] = (float)current_settings_->dmg_color2_b / 255;
                    c4[0] = (float)current_settings_->dmg_color3_r / 255;
                    c4[1] = (float)current_settings_->dmg_color3_g / 255;
                    c4[2] = (float)current_settings_->dmg_color3_b / 255;
                }
                ImGui::Separator();
            }
            ImGui::End();
        }
    };
}
#endif;