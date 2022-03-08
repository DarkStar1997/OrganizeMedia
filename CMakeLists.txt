find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
    MESSAGE("CCACHE support enabled")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()

if(MSVC)
    MESSAGE("Compiler is MSVC. Sad")
    set(CMAKE_CXX_FLAGS "/MT")
else()
    MESSAGE("Compiler isn't MSVC. Good for you")
    set(CMAKE_CXX_FLAGS "-static")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_BUILD_TYPE "RELEASE")

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
    fmtlib
    GIT_REPOSITORY "https://github.com/fmtlib/fmt"
    GIT_TAG 5c0d65640118e7e0b1350f28fab3243fd6ce35a0
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fmtlib)

FetchContent_Declare(
    argh
    GIT_REPOSITORY "https://github.com/adishavit/argh"
    GIT_TAG 01b6a4a3b0991dc2f29e4ab55eb7da90c8189f80
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(argh)

FetchContent_Declare(
    nlohmann
    GIT_REPOSITORY "https://github.com/nlohmann/json"
    GIT_TAG 700b95f4473cac1d900b807541daad0ca3d98f52
    GIT_PROGRESS TRUE
)
FetchContent_Populate(nlohmann)
include_directories(${nlohmann_SOURCE_DIR}/include)

set(SOURCES
    main.cpp
    )

set(EXE_NAME "rmg_organize")
add_executable(${EXE_NAME} ${SOURCES})
target_link_libraries(${EXE_NAME} fmt::fmt)
target_include_directories(${EXE_NAME} PRIVATE ${argh_SOURCE_DIR})
