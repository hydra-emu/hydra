#pragma once

#include <functional>
#include <imgui/imgui.h>
#include <string>
#include <utility>
#include <vector>

using FilePickerFilters = std::vector<std::pair<std::string, std::string>>;

class FilePicker
{
public:
    FilePicker(const std::string& id, const std::string& name,
               std::function<void(const char*)> callback);
    ~FilePicker();
    void update(ImVec2 start, ImVec2 end, const FilePickerFilters& filters);
    void hide();
    std::function<void(const char*)> callback;

private:
    std::string id;
    std::string imgui_id;
    std::string name;

#ifdef HYDRA_WEB
    std::string html_id;
#endif
};