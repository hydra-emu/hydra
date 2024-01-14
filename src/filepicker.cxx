#include "IconsMaterialDesign.h"
#include <filepicker.hxx>
#include <imgui/imgui.h>
#include <settings.hxx>

#ifdef HYDRA_WEB
int html_id_last = 0;
#include <emscripten.h>
#else
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <nfd.h>
bool nfd_errors = false;
#endif

FilePicker::FilePicker(const std::string& id, const std::string& name,
                       std::function<void(const char*)> callback)
    : id(id), name(name), callback(callback)
{
    imgui_id = this->name + "##" + this->id;
#ifdef HYDRA_WEB
    html_id = "filepicker_" + std::to_string(html_id_last++);
    EM_ASM({
        var input = document.createElement('input');
        input.type = 'file';
        input.id = $0;
        input.value = '';
        document.body.appendChild(input);
        input.onmousemove =  input.onmouseover =  function(e) {
            const mouseMoveEvent = new MouseEvent('mousemove', {
                bubbles: true,
                cancelable: true,
                clientX: event.clientX,
                clientY: event.clientY
            });
            document.getElementById('canvas').dispatchEvent(mouseMoveEvent);
        };
    }, html_id.c_str());
#endif
}

FilePicker::~FilePicker()
{
#ifdef HYDRA_WEB
    EM_ASM({
        var element = document.getElementById($0);
        element.parentNode.removeChild(element);
    }, html_id.c_str());
#endif
}

void FilePicker::update(ImVec2 start, ImVec2 end, const FilePickerFilters& filters)
{
    bool hovered = ImGui::IsMouseHoveringRect(start, end);
    if (hovered && ImGui::IsMouseClicked(0))
    {
        std::string default_path = Settings::Get("last_path");
        if (default_path.empty())
        {
            default_path = ".";
            Settings::Set("last_path", default_path);
        }
#ifdef HYDRA_DESKTOP
        if (!nfd_errors)
        {
            NFD_Init();
            auto filters_copy = filters;
            nfdchar_t* path = nullptr;
            nfdfilteritem_t* filter = new nfdfilteritem_t[filters.size()];
            for (size_t i = 0; i < filters.size(); ++i)
            {
                filter[i].name = filters[i].first.c_str();
                std::erase(filters_copy[i].second, '.');
                filter[i].spec = filters_copy[i].second.c_str();
            }
            nfdresult_t result =
                NFD_OpenDialog(&path, filter, filters.size(), default_path.c_str());
            if (result == NFD_OKAY)
            {
                callback(path);
                NFD_FreePath(path);
            }
            else if (result != NFD_CANCEL)
            {
                // This can happen if the user doesn't have an xdg-desktop-portal backend installed
                // for example
                nfd_errors = true;
                printf("Error: %s\nUsing imgui replacement", NFD_GetError());
                goto imgui_replacement;
            }
            NFD_Quit();
        }
        else
        {
        imgui_replacement:
            std::string filter_str;
            for (size_t i = 0; i < filters.size(); ++i)
            {
                filter_str +=
                    filters[i].first + '(' + filters[i].second + ") {" + filters[i].second + "},";
            }
            filter_str.pop_back();
            IGFD::FileDialogConfig config;
            config.path = default_path;
            config.countSelectionMax = 1;
            config.flags = ImGuiFileDialogFlags_Modal;
            std::string sname = ICON_MD_FOLDER_OPEN + name;
            ImGuiFileDialog::Instance()->OpenDialog("dialog", sname, filter_str.c_str(), config);
        }
#endif
    }

#ifdef HYDRA_DESKTOP
    if (nfd_errors)
    {
        ImGui::SetNextWindowSize(
            ImVec2(ImGui::GetIO().DisplaySize.x * 0.8f, ImGui::GetIO().DisplaySize.y * 0.8f));
        if (ImGuiFileDialog::Instance()->Display("dialog"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                Settings::Set("last_path", ImGuiFileDialog::Instance()->GetCurrentPath());
                callback(ImGuiFileDialog::Instance()->GetFilePathName().c_str());
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
#endif
}
