#pragma once

#include <filesystem>
#include <vector>
#include <unordered_set>

enum class Mode {
    Mode_COPY,
    Mode_MOVE,
    Mode_COPY_AND_DELETE
};

struct Renderer {

    Renderer() : ext_checkboxes(6, 0), selected_mode(Mode::Mode_COPY) {}

    void RenderUI();
    std::filesystem::path get_selected_path();

    Mode selected_mode;
    std::vector<uint8_t> ext_checkboxes;
    std::unordered_set<std::string> extensions;
    std::string other_filters_str;
    std::filesystem::path source_path;
    std::filesystem::path dest_path;
};
