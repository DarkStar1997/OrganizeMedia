find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
    MESSAGE("CCACHE support enabled")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CCACHE_PROGRAM}")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "/MT")
else()
    set(CMAKE_CXX_FLAGS "-static")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

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
    nlohmann
    GIT_REPOSITORY "https://github.com/nlohmann/json"
    GIT_TAG 700b95f4473cac1d900b807541daad0ca3d98f52
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

set(SOURCES
    main.cpp
    )

set(EXE_NAME "rmg_organize")
add_executable(${EXE_NAME} ${SOURCES})
target_link_libraries(${EXE_NAME} fmt::fmt)
