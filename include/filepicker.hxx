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
    FilePicker(const std::string& id, const std::string& name, const std::string& default_path,
               const FilePickerFilters& filters, std::function<void(const char*)> callback);
    ~FilePicker();
    void update(ImVec2 start, ImVec2 end);

private:
    std::string id;
    std::string imgui_id;
    std::string name;
    std::function<void(const char*)> callback;
    FilePickerFilters filters;

#ifdef HYDRA_WEB
    std::string html_id;
#endif
};