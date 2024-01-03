        #include <cmrc/cmrc.hpp>
#include <map>
#include <utility>

namespace cmrc {
namespace hydra {

namespace res_chars {
// These are the files which are available in this resource library
// Pointers to data/images/hydra.png
extern const char* const f_23a6_hydra_png_begin;
extern const char* const f_23a6_hydra_png_end;
}

namespace {

const cmrc::detail::index_type&
get_root_index() {
    static cmrc::detail::directory root_directory_;
    static cmrc::detail::file_or_directory root_directory_fod{root_directory_};
    static cmrc::detail::index_type root_index;
    root_index.emplace("", &root_directory_fod);
    struct dir_inl {
        class cmrc::detail::directory& directory;
    };
    dir_inl root_directory_dir{root_directory_};
    (void)root_directory_dir;
    static auto f_59b5_images_dir = root_directory_dir.directory.add_subdir("images");
    root_index.emplace("images", &f_59b5_images_dir.index_entry);
    root_index.emplace(
        "images/hydra.png",
        f_59b5_images_dir.directory.add_file(
            "hydra.png",
            res_chars::f_23a6_hydra_png_begin,
            res_chars::f_23a6_hydra_png_end
        )
    );
    return root_index;
}

}

cmrc::embedded_filesystem get_filesystem() {
    static auto& index = get_root_index();
    return cmrc::embedded_filesystem{index};
}

} // hydra
} // cmrc
    