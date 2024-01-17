#include "IconsMaterialDesign.h"
#include <filepicker.hxx>
#include <imgui/imgui.h>
#include <settings.hxx>

#ifdef HYDRA_WEB
int html_id_last = 0;
#include <emscripten.h>
std::vector<FilePicker*> pickers;
EM_JS(void, filepicker_move, (const char* id, int x, int y, int x2, int y2), {
    var element = document.getElementById(UTF8ToString(id));
    element.style.left = x + 'px';
    element.style.top = y + 'px';
    element.style.width = (x2 - x) + 'px';
    element.style.height = (y2 - y) + 'px';
    element.style.visibility = 'visible';
});

extern "C" void filepicker_callback(void* id, const char* path)
{
    FilePicker* picker = (FilePicker*)(id);
    picker->callback(path);
}
#else
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#ifdef HYDRA_NATIVE_FILE_DIALOG
#include <nfd.h>
bool nfd_errors = false;
#else
bool nfd_errors = true;
#endif
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
        input.id = UTF8ToString($0);
        input.value = '';
        input.type = 'file';
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
    pickers.push_back(this);
#endif
}

FilePicker::~FilePicker()
{
#ifdef HYDRA_WEB
    EM_ASM({
        var element = document.getElementById($0);
        element.parentNode.removeChild(element);
    }, html_id.c_str());
    pickers.erase(std::remove(pickers.begin(), pickers.end(), this), pickers.end());
#endif
}

void FilePicker::hideAll()
{
#ifdef HYDRA_WEB
    for (auto picker : pickers)
    {
        EM_ASM({
            var element = document.getElementById(UTF8ToString($0));
            element.style.visibility = 'hidden';
        }, picker->html_id.c_str());
    }
#endif
}

void FilePicker::update(ImVec2 start, ImVec2 end, const FilePickerFilters& filters)
{
#ifdef HYDRA_WEB
    filepicker_move(html_id.c_str(), start.x, start.y, end.x, end.y);
    EM_ASM({
        var element = document.getElementById(UTF8ToString($0));
        if (element.value !== '') {
            var reader = new FileReader();
            var file = element.files[0];
            function print_file(e){
                var result = reader.result;
                var filename = file.name;
                const uint8_view = new Uint8Array(result);
                var out_file = '/hydra/cache/' + filename;
                if(FS.analyzePath(out_file)["exists"])FS.unlink(out_file);
                FS.writeFile(out_file, uint8_view);
                FS.syncfs(function (err) {
                    Module.ccall('filepicker_callback', 'void', ['number', 'string'], [$1, out_file]);
                });
            }
            reader.addEventListener('loadend', print_file);
            reader.readAsArrayBuffer(file);
            element.value = '';
        }
    }, html_id.c_str(), this);
#endif

    bool hovered = ImGui::IsMouseHoveringRect(start, end);
    if (hovered && ImGui::IsMouseClicked(0))
    {
#ifdef HYDRA_DESKTOP
        std::string default_path = Settings::Get("last_path");
        if (default_path.empty())
        {
            default_path = ".";
            Settings::Set("last_path", default_path);
        }
#ifdef HYDRA_NATIVE_FILE_DIALOG
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
#endif
        {
        imgui_replacement:
            std::string filter_str;
            if (filters[0].second != "*.*")
            {
                for (size_t i = 0; i < filters.size(); ++i)
                {
                    filter_str += filters[i].first + '(' + filters[i].second + ") {" +
                                  filters[i].second + "},";
                }
                filter_str.pop_back();
            }
            else
            {
            }

            IGFD::FileDialogConfig config = {
                .path = default_path,
                .countSelectionMax = 1,
                .flags = ImGuiFileDialogFlags_Modal,
            };
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
