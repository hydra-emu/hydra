set(CMAKE_OBJC_COMPILER "/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/emcc")
set(CMAKE_OBJC_COMPILER_ARG1 "")
set(CMAKE_OBJC_COMPILER_ID "Clang")
set(CMAKE_OBJC_COMPILER_VERSION "18.0.0")
set(CMAKE_OBJC_COMPILER_VERSION_INTERNAL "")
set(CMAKE_OBJC_COMPILER_WRAPPER "")
set(CMAKE_OBJC_STANDARD_COMPUTED_DEFAULT "11")
set(CMAKE_OBJC_EXTENSIONS_COMPUTED_DEFAULT "ON")
set(CMAKE_OBJC_COMPILE_FEATURES "")
set(CMAKE_OBJC90_COMPILE_FEATURES "")
set(CMAKE_OBJC99_COMPILE_FEATURES "")
set(CMAKE_OBJC11_COMPILE_FEATURES "")
set(CMAKE_OBJC17_COMPILE_FEATURES "")
set(CMAKE_OBJC23_COMPILE_FEATURES "")

set(CMAKE_OBJC_PLATFORM_ID "")
set(CMAKE_OBJC_SIMULATE_ID "")
set(CMAKE_OBJC_COMPILER_FRONTEND_VARIANT "GNU")
set(CMAKE_OBJC_SIMULATE_VERSION "")


set(CMAKE_AR "/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/emar")
set(CMAKE_OBJC_COMPILER_AR "CMAKE_OBJC_COMPILER_AR-NOTFOUND")
set(CMAKE_RANLIB "/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/emranlib")
set(CMAKE_OBJC_COMPILER_RANLIB "CMAKE_OBJC_COMPILER_RANLIB-NOTFOUND")
set(CMAKE_LINKER "/usr/bin/ld")
set(CMAKE_MT "")
set(CMAKE_TAPI "CMAKE_TAPI-NOTFOUND")
set(CMAKE_COMPILER_IS_GNUOBJC )
set(CMAKE_OBJC_COMPILER_LOADED 1)
set(CMAKE_OBJC_COMPILER_WORKS TRUE)
set(CMAKE_OBJC_ABI_COMPILED TRUE)

set(CMAKE_OBJC_COMPILER_ENV_VAR "OBJC")

set(CMAKE_OBJC_COMPILER_ID_RUN 1)
set(CMAKE_OBJC_SOURCE_FILE_EXTENSIONS m)
set(CMAKE_OBJC_IGNORE_EXTENSIONS h;H;o;O)
set(CMAKE_OBJC_LINKER_PREFERENCE 5)
set(CMAKE_OBJC_LINKER_DEPFILE_SUPPORTED FALSE)

foreach (lang C CXX OBJCXX)
  foreach(extension IN LISTS CMAKE_OBJC_SOURCE_FILE_EXTENSIONS)
    if (CMAKE_${lang}_COMPILER_ID_RUN)
      list(REMOVE_ITEM CMAKE_${lang}_SOURCE_FILE_EXTENSIONS ${extension})
    endif()
  endforeach()
endforeach()

# Save compiler ABI information.
set(CMAKE_OBJC_SIZEOF_DATA_PTR "")
set(CMAKE_OBJC_COMPILER_ABI "")
set(CMAKE_OBJC_BYTE_ORDER "")
set(CMAKE_OBJC_LIBRARY_ARCHITECTURE "")

if(CMAKE_OBJC_SIZEOF_DATA_PTR)
  set(CMAKE_SIZEOF_VOID_P "${CMAKE_OBJC_SIZEOF_DATA_PTR}")
endif()

if(CMAKE_OBJC_COMPILER_ABI)
  set(CMAKE_INTERNAL_PLATFORM_ABI "${CMAKE_OBJC_COMPILER_ABI}")
endif()

if(CMAKE_OBJC_LIBRARY_ARCHITECTURE)
  set(CMAKE_LIBRARY_ARCHITECTURE "")
endif()





set(CMAKE_OBJC_IMPLICIT_INCLUDE_DIRECTORIES "/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/cache/sysroot/include/fakesdl;/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/cache/sysroot/include/compat;/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/lib/clang/18/include;/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/cache/sysroot/include")
set(CMAKE_OBJC_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_OBJC_IMPLICIT_LINK_DIRECTORIES "/home/runner/work/hydra/hydra/emsdk-cache/emsdk-main/upstream/emscripten/cache/sysroot/lib/wasm32-emscripten")
set(CMAKE_OBJC_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES "")
