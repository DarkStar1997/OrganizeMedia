#pragma once

#include <filesystem>
#include <vector>
#include <unordered_set>
#include <string>

enum class Mode {
    Mode_COPY,
    Mode_MOVE,
    Mode_COPY_AND_DELETE
};

static const uint8_t TAKING_INPUT = 0x01;
static const uint8_t COMPUTING_FILES = 0x02;
static const uint8_t ORGANIZING = 0x04;

struct Renderer {

    Renderer()
    {
        checkbox_labels = std::vector<std::string>({"png", "jpg", "jpeg", "mp4", "avi", "others"});
        ext_checkboxes = std::vector<uint8_t>(checkbox_labels.size(), 0);
        selected_mode = Mode::Mode_COPY;
        state = TAKING_INPUT;
    }

    void RenderUI();
    void ShowStartingOptions();
    std::filesystem::path get_selected_path();

    Mode selected_mode;
    uint8_t state;
    std::vector<uint8_t> ext_checkboxes;
    std::vector<std::string> checkbox_labels;
    std::unordered_set<std::string> extensions;
    std::string other_filters_str;
    std::filesystem::path source_path;
    std::filesystem::path dest_path;
};
