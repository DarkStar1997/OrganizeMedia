#include "renderer.hpp"
#include <imgui.h>
#include <nfd.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <vector>
#include <thread>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <md5.h>
#include "imspinner.h"

static bool source_button_val = false;
static bool dest_button_val = false;
static char other_filters[256];
static bool start_button_val = false;
static bool stop_button_val = false;
static double x_offset = 0;
static bool organize_button_val = false;
static std::vector<std::string> month = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};
static std::unordered_map<std::string, std::string> hash_data; // map of [hash - filename]

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

const auto computeHash = [](auto hash, const std::string &filename) -> std::string
{
    std::unique_ptr<std::istream, void (*)(std::istream *)> inStream {nullptr, [](auto) {}};
    inStream = {new std::ifstream(filename, (std::ios_base::in | std::ios_base::binary)), [](std::istream *p) {delete p;}};

    const int bufSize = 1024 * 1024;
    auto buf = std::make_unique<char[]>(bufSize);
    while (inStream->good())
    {
        inStream->read(buf.get(), bufSize);
        hash.addData(buf.get(), inStream->gcount());
    }
    return hash.finalize().toString();
};

void Renderer::ShowStartingOptions()
{
    ImGui::SetCursorPosX(x_offset);
    if (ImGui::RadioButton("Copy", selected_mode == Mode::Mode_COPY)) { selected_mode = Mode::Mode_COPY; }
    ImGui::SameLine(); HelpMarker("Copy files after organizing from source to destination"); ImGui::SameLine();

    if (ImGui::RadioButton("Move", selected_mode == Mode::Mode_MOVE)) { selected_mode = Mode::Mode_MOVE; }
    ImGui::SameLine(); HelpMarker("Move files after organizing from source to destination"); ImGui::SameLine();

    if (ImGui::RadioButton("Copy then Delete", selected_mode == Mode::Mode_COPY_AND_DELETE)) { selected_mode = Mode::Mode_COPY_AND_DELETE; }
    ImGui::SameLine(); HelpMarker("Copy files after organizing from source to destination. Then delete the source files.");

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    for(size_t index = 0; index < checkbox_labels.size(); index++) {
        ImGui::SetCursorPosX(x_offset);
        ImGui::Checkbox(checkbox_labels[index].c_str(), (bool *)&ext_checkboxes[index]);
    }

    ImGui::SameLine();
    HelpMarker("Enter comma separated filters like: mp4,avi,jpg,jpeg,png");

    if(ext_checkboxes.back()) {
        ImGui::SetCursorPosX(x_offset);
        ImGui::InputText("other extensions", &other_filters[0], IM_ARRAYSIZE(other_filters));
        other_filters_str = other_filters;
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f)); ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::SetCursorPosX(x_offset);
    source_button_val = ImGui::Button("Select Source");

    if(source_button_val)
        source_path = get_selected_path();

    if(source_path.string().length())
    {
        ImGui::SameLine();
        ImGui::Text("Source: %s", source_path.string().c_str());
    }

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::SetCursorPosX(x_offset);
    dest_button_val = ImGui::Button("Select Destination");

    if(dest_button_val)
        dest_path = get_selected_path();

    if(dest_path.string().length())
    {
        ImGui::SameLine();
        ImGui::Text("Destination: %s", dest_path.string().c_str());
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f)); ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::SetCursorPosX(x_offset);
    start_button_val = ImGui::Button("Start");
    ImGui::Dummy(ImVec2(0.0f, 5.0f)); ImGui::Dummy(ImVec2(0.0f, 20.0f));

    if(start_button_val)
    {
        if(state == State::TAKING_INPUT) {
            update_extensions();
        }
        state = State::COMPUTING_FILES;
        renderer_lock.unlock();
        cond.notify_one();
    }
}

void Renderer::RenderUI()
{
    x_offset = ImGui::GetWindowSize().x / 10.0f;
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    ImGui::Begin("Media Organizer");

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    {
        std::string message = "A Tool To Organize Your Media Effortlessly";
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - message.size()) / 3);
        ImGui::Text(message.c_str());
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f)); ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 5.0f)); ImGui::Dummy(ImVec2(0.0f, 20.0f));

    if(state >= State::TAKING_INPUT)
    {
        ImGui::BeginDisabled(state > State::TAKING_INPUT);
        ShowStartingOptions();
        ImGui::EndDisabled();
    }

    if(state == State::COMPUTING_FILES) {
        ImGui::SetCursorPosX(x_offset);
        ImSpinner::SpinnerTwinAng360("SpinnerTwinAng360", 16, 11, 4, ImColor(255, 255, 255), ImColor(255, 0, 0), 4);
        ImGui::SameLine();
        ImGui::Text("Computing number of potential files");
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
    }

    if(state >= State::ORGANIZING) {

        ImGui::SetCursorPosX(x_offset);
        ImGui::Text("Computed %lu potential new files", computed_file_count);
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::SetCursorPosX(x_offset);
        ImGui::ProgressBar(organizing_progress, ImVec2(0.0f, 0.0f));
        ImGui::Dummy(ImVec2(0.0f, 5.0f)); ImGui::Dummy(ImVec2(0.0f, 20.0f));

    }

    if(state == State::COMPLETED) {
        ImGui::SetCursorPosX(x_offset);
        ImGui::Text("New Files: %lu", new_files_count);
        ImGui::SetCursorPosX(x_offset);
        ImGui::Text("Duplicate Files: %lu", duplicate_list.size());
        ImGui::SetCursorPosX(x_offset);
        ImGui::Text("Log generated in rmg_log.txt");
        ImGui::Dummy(ImVec2(0.0f, 5.0f)); ImGui::Dummy(ImVec2(0.0f, 20.0f));
    }

    ImGui::End();
}

std::filesystem::path Renderer::get_selected_path() {

    std::filesystem::path selectedPath;
    NFD_Init();

    nfdnchar_t *outPath;
    const nfdnchar_t *default_path;
    nfdresult_t result = NFD_PickFolderN(&outPath, default_path);
    if (result == NFD_OKAY)
    {
        fmt::print("Success!\n");
        selectedPath = outPath;
        NFD_FreePath((nfdu8char_t *)outPath);
    }
    else if (result == NFD_CANCEL)
    {
        fmt::print("User pressed cancel.\n");
    }
    else
    {
        fmt::print("Error: {}\n", NFD_GetError());
    }

    NFD_Quit();
    return selectedPath;
}

void Renderer::run() {
    std::unique_lock<std::mutex> locker(mu);
    cond.wait(locker, [this](){return (state > State::TAKING_INPUT);});
    locker.unlock();

    if(state == State::ABORT)
        return;

    for(const auto& dir_entry : std::filesystem::recursive_directory_iterator(source_path))
        if(state != State::ABORT && std::filesystem::is_regular_file(dir_entry.path()) && extensions.find(dir_entry.path().extension().string()) != extensions.end())
            computed_file_count++;

    if(computed_file_count == 0) {
        state = State::COMPLETED;
        return;
    }

    state = State::ORGANIZING;

    size_t count = 0, duplicates = 0;

    duplicate_list.reserve(computed_file_count);
    renamed_list.reserve(computed_file_count);

    for(const auto& dir_entry : std::filesystem::recursive_directory_iterator(source_path))
    {
        if(state == State::ABORT)
            return;
        if(std::filesystem::is_regular_file(dir_entry.path()) && std::find(extensions.begin(), extensions.end(), dir_entry.path().extension()) != extensions.end())
        {
            struct stat result;
            if(stat(dir_entry.path().string().c_str(), &result) == 0)
            {
                std::tm * timestamp = std::localtime(&result.st_mtime);
                auto output_path = get_output_dir(timestamp, std::filesystem::path(dest_path));
                auto output_file = output_path / dir_entry.path().filename();
                if(!std::filesystem::exists(output_path))
                    std::filesystem::create_directories(output_path);
                if(std::filesystem::exists(output_file))
                {
                    std::string output_file_hash = computeHash(Chocobo1::MD5(), output_file.string());
                    if(hash_data.find(output_file_hash) == hash_data.end())
                        hash_data.insert({output_file_hash, output_file.filename().string()});
                    std::string input_file_hash = computeHash(Chocobo1::MD5(), dir_entry.path().string());

                    if(input_file_hash == output_file_hash)
                    {
                        duplicates++;
                        organizing_progress = ((count + duplicates) / (float)computed_file_count);
                        duplicate_list.push_back(dir_entry.path().string());
                        continue;
                    }

                    auto data = hash_data.find(input_file_hash);
                    if(data != hash_data.end())
                    {
                        duplicates++;
                        organizing_progress = ((count + duplicates) / (float)computed_file_count);
                        duplicate_list.push_back(dir_entry.path().string());
                        continue;
                    }
                    hash_data.insert({input_file_hash, dir_entry.path().filename().string()});

                    std::string new_output_filename; int tmp_count = 1;
                    do
                    {
                        new_output_filename = output_file.stem().string() + std::to_string(tmp_count) + output_file.extension().string();
                        tmp_count++;
                    }while(std::filesystem::exists(output_path / std::filesystem::path(new_output_filename)));
                    output_file = output_path / std::filesystem::path(new_output_filename);
                    renamed_list.push_back({dir_entry.path().string(), output_file.string()});
                }
                if(selected_mode == Mode::Mode_COPY)
                    std::filesystem::copy_file(dir_entry.path(), output_file);
                else if(selected_mode == Mode::Mode_MOVE)
                    std::filesystem::rename(dir_entry.path(), output_file);
                else if(selected_mode == Mode::Mode_COPY_AND_DELETE)
                {
                    std::filesystem::copy_file(dir_entry.path(), output_file);
                    std::filesystem::remove(dir_entry.path());
                }
                count++;
            }
        }
        organizing_progress = ((count + duplicates) / (float)computed_file_count);
    }
    new_files_count = count;
    generate_log(duplicate_list, renamed_list);
    state = State::COMPLETED;
}

void Renderer::update_extensions() {
    for(size_t i = 0; i < checkbox_labels.size() - 1; i++) { // ignore the other filters
        if(ext_checkboxes[i]) {
            extensions.insert("." + checkbox_labels[i]);
        }
    }

    other_filters_str += ',';
    std::string cur_ext = "";
    for(size_t i = 0; i < other_filters_str.length(); i++) {
        const char ch = other_filters_str[i];
        if(std::isalnum(ch)) {
            cur_ext += ch;
        }
        else if(ch == ',') {
            if(cur_ext.length() > 0)
                extensions.insert("." + cur_ext);
            cur_ext = "";
        }
    }
}

std::filesystem::path Renderer::get_output_dir(std::tm * timestamp, const std::filesystem::path output_path)
{
    return output_path / std::to_string(1900 + timestamp->tm_year) / month[timestamp->tm_mon] / std::to_string(timestamp->tm_mday);
}

void Renderer::generate_log(const std::vector<std::string> & duplicates,
        const std::vector<std::pair<std::string, std::string>> & renamed)
{
    std::ofstream rmg_log("rmg_log.txt");
    if(duplicates.size() > 0)
    {
        rmg_log << fmt::format("\n\n{:=^100}\n\n", " Duplicate Files ");
        for(size_t i = 0; i < duplicates.size(); i++)
            rmg_log << fmt::format("{}) {}\n", i + 1, duplicates[i]);
    }
    if(renamed.size() > 0)
    {
        rmg_log << fmt::format("\n\n{:=^100}\n\n", " Renamed Files ");
        for(size_t i = 0; i < renamed.size(); i++)
            rmg_log << fmt::format("{}) {} renamed to {}\n", i + 1, renamed[i].first, renamed[i].second);
    }
}
