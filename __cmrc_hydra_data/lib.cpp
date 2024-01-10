        #include <cmrc/cmrc.hpp>
#include <map>
#include <utility>

namespace cmrc {
namespace hydra {

namespace res_chars {
// These are the files which are available in this resource library
// Pointers to data/images/hydra.png
extern const char* const f_7da7_data_images_hydra_png_begin;
extern const char* const f_7da7_data_images_hydra_png_end;
// Pointers to data/cacert.pem
extern const char* const f_f6f8_data_cacert_pem_begin;
extern const char* const f_f6f8_data_cacert_pem_end;
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
    static auto f_8d77_data_dir = root_directory_dir.directory.add_subdir("data");
    root_index.emplace("data", &f_8d77_data_dir.index_entry);
    static auto f_5d4b_data_images_dir = f_8d77_data_dir.directory.add_subdir("images");
    root_index.emplace("data/images", &f_5d4b_data_images_dir.index_entry);
    root_index.emplace(
        "data/images/hydra.png",
        f_5d4b_data_images_dir.directory.add_file(
            "hydra.png",
            res_chars::f_7da7_data_images_hydra_png_begin,
            res_chars::f_7da7_data_images_hydra_png_end
        )
    );
    root_index.emplace(
        "data/cacert.pem",
        f_8d77_data_dir.directory.add_file(
            "cacert.pem",
            res_chars::f_f6f8_data_cacert_pem_begin,
            res_chars::f_f6f8_data_cacert_pem_end
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
    