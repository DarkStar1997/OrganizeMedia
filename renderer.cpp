#include "renderer.hpp"
#include <imgui.h>
#include <nfd.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <vector>
#include "imspinner.h"

#define IM_CLAMP(V, MN, MX)     ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))

static bool source_button_val = false;
static bool dest_button_val = false;
static char other_filters[256];
static bool start_button_val = false;
static double x_offset = 0;
static bool organize_button_val = false;

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

void Renderer::ShowStartingOptions()
{
    ImGui::SetCursorPosX(x_offset);
    if (ImGui::RadioButton("Copy", selected_mode == Mode::Mode_COPY)) { selected_mode = Mode::Mode_COPY; }
    ImGui::SameLine(); HelpMarker("Copy files after organizing from source to destination");

    ImGui::SetCursorPosX(x_offset);
    if (ImGui::RadioButton("Move", selected_mode == Mode::Mode_MOVE)) { selected_mode = Mode::Mode_MOVE; }
    ImGui::SameLine(); HelpMarker("Move files after organizing from source to destination");

    ImGui::SetCursorPosX(x_offset);
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
        state |= COMPUTING_FILES;
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

    if(state & TAKING_INPUT)
        ShowStartingOptions();

    if(state & COMPUTING_FILES) {
        ImGui::SetCursorPosX(x_offset);
        ImSpinner::SpinnerTwinAng360("SpinnerTwinAng360", 16, 11, 4, ImColor(255, 255, 255), ImColor(255, 0, 0), 4);
        ImGui::SameLine();
        ImGui::Text("Computing number of potential files");
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::SetCursorPosX(x_offset);
        organize_button_val = ImGui::Button("Organize");
        if(organize_button_val) {
            state = state & ~COMPUTING_FILES;
            state |= ORGANIZING;
        }
    }

    if(state & ORGANIZING) {
        static float progress = 0.0f, progress_dir = 1.0f;
        progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
        if (progress >= +1.1f) { progress = +1.1f; progress_dir *= -1.0f; }
        if (progress <= -0.1f) { progress = -0.1f; progress_dir *= -1.0f; }

        ImGui::SetCursorPosX(x_offset);
        ImGui::Text("Computed 1000 potential new files");
        ImGui::SetCursorPosX(x_offset);
        ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));

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
        fmt::print("{}\n", outPath);
        selectedPath = outPath;
        NFD_FreePath(outPath);
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
