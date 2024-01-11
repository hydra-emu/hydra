#include <filepicker.hxx>
#include <imgui/imgui.h>
#include <nfd.h>

#ifdef HYDRA_WEB
int html_id_last = 0;
#include <emscripten.h>
#endif

FilePicker::FilePicker(const std::string& id, const std::string& name,
                       const std::string& default_path, const FilePickerFilters& filters,
                       std::function<void(const char*)> callback)
    : id(id), name(name), filters(filters), callback(callback)
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

void FilePicker::update(ImVec2 start, ImVec2 end)
{
    bool hovered = ImGui::IsMouseHoveringRect(start, end);
    if (hovered && ImGui::IsMouseClicked(0))
    {
#ifdef HYDRA_DESKTOP
        NFD_Init();
        nfdchar_t* path = nullptr;
        nfdfilteritem_t* filter = new nfdfilteritem_t[filters.size()];
        for (size_t i = 0; i < filters.size(); ++i)
        {
            filter[i].name = filters[i].first.c_str();
            filter[i].spec = filters[i].second.c_str();
        }
        nfdresult_t result = NFD_OpenDialog(&path, filter, filters.size(), NULL);
        if (result == NFD_OKAY)
        {
            callback(path);
            NFD_FreePath(path);
        }
        else if (result != NFD_CANCEL)
        {
            printf("Error: %s\n", NFD_GetError());
        }
        NFD_Quit();
#endif
    }
}
