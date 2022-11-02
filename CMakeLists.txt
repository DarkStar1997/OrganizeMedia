find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
    MESSAGE("CCACHE support enabled")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
get_filename_component(fc_cache "../.fetchcontent_cache"
    REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(FETCHCONTENT_BASE_DIR ${fc_cache})

FetchContent_Declare(
    fmtlib
    GIT_REPOSITORY "https://github.com/fmtlib/fmt"
    GIT_TAG 9.1.0
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fmtlib)

FetchContent_Declare(
    nlohmann
    GIT_REPOSITORY "https://github.com/nlohmann/json"
    GIT_TAG v3.11.2
    GIT_PROGRESS TRUE
)
FetchContent_Populate(nlohmann)
include_directories(${nlohmann_SOURCE_DIR}/include)

FetchContent_Declare(
    hash
    GIT_REPOSITORY "https://github.com/Chocobo1/Hash"
    GIT_TAG 8ccfa5b92425be9bc7942c21743eeb7d9e188834
    GIT_PROGRESS TRUE
)
FetchContent_Populate(hash)
include_directories(${hash_SOURCE_DIR}/src)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui"
    GIT_TAG f87e891f1818d02f55028fe042a323bb27c41d66
    GIT_PROGRESS TRUE
)
FetchContent_Populate(imgui)

FetchContent_Declare(
    nativefiledialog
    GIT_REPOSITORY "https://github.com/btzy/nativefiledialog-extended"
    GIT_TAG 74923e7c0cc19fec1b789e78c033c211300e4a01
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(nativefiledialog)

set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
FetchContent_Declare(
    glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw"
    GIT_TAG 3.3.8
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(glfw)

set(SOURCES
    main.cpp
    )

set(EXE_NAME "rmg_organize")
add_executable(${EXE_NAME} ${SOURCES})
target_link_libraries(${EXE_NAME} fmt::fmt)


find_package(OpenGL REQUIRED)

set(GUI_SOURCE_LIST
    main_gui.cpp
    renderer.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    )

if(UNIX OR WIN32)
    set(BACKEND_LIST
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        )
    list(APPEND GUI_SOURCE_LIST main_gui.cpp ${BACKEND_LIST})
endif()

add_executable(run ${GUI_SOURCE_LIST})
target_include_directories(run PRIVATE
                           ${imgui_SOURCE_DIR}
                           ${imgui_SOURCE_DIR}/backends
                           #${nativefiledialog_SOURCE_DIR}
                           )

if(UNIX)
    target_link_libraries(run PRIVATE pthread glfw OpenGL::GL fmt::fmt nfd)
elseif(WIN32)
    target_link_libraries(run PRIVATE glfw OpenGL::GL fmt::fmt nfd)
endif()

add_custom_target(copy_fonts
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_LIST_DIR}/fonts/OpenSans-Regular.ttf
    ${CMAKE_CURRENT_BINARY_DIR}
)
add_dependencies(run copy_fonts)
