#pragma once

#include <filesystem>
#include <vector>
#include <unordered_set>
#include <string>
#include <mutex>
#include <condition_variable>

enum class Mode {
    Mode_COPY,
    Mode_MOVE,
    Mode_COPY_AND_DELETE
};

enum class State {
    TAKING_INPUT,
    COMPUTING_FILES,
    ORGANIZING,
    COMPLETED,
    ABORT
};

enum class OrgLevel {
    YEAR,
    MONTH,
    DATE
};

struct Renderer {

    Renderer()
    {
        checkbox_labels = std::vector<std::string>({"png", "jpg", "jpeg", "mp4", "avi", "others"});
        ext_checkboxes = std::vector<uint8_t>(checkbox_labels.size(), 0);
        selected_mode = Mode::Mode_COPY;
        state = State::TAKING_INPUT;
        selected_org_level = OrgLevel::DATE;
        renderer_lock = std::unique_lock<std::mutex>(mu);
        computed_file_count = 0;
        new_files_count = 0;
        organizing_progress = 0.0f;
    }

    void RenderUI();
    void ShowStartingOptions();
    std::filesystem::path get_selected_path();
    void run();
    void update_extensions();
    std::filesystem::path get_output_dir(std::tm * timestamp, const std::filesystem::path output_path);
    void generate_log(const std::vector<std::string> & duplicates,
            const std::vector<std::pair<std::string, std::string>> & renamed);

    Mode selected_mode;
    State state;
    OrgLevel selected_org_level;
    std::vector<uint8_t> ext_checkboxes;
    std::vector<std::string> checkbox_labels;
    std::unordered_set<std::string> extensions;
    std::string other_filters_str;
    std::filesystem::path source_path;
    std::filesystem::path dest_path;
    std::mutex mu;
    std::unique_lock<std::mutex> renderer_lock;
    std::condition_variable cond;
    size_t computed_file_count;
    size_t new_files_count;
    std::vector<std::string> duplicate_list;
    std::vector<std::pair<std::string, std::string>> renamed_list;
    float organizing_progress;
};
