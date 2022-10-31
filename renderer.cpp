#include "renderer.hpp"
#include <imgui.h>
#include <nfd.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <vector>

static bool source_button_val = false;
static bool dest_button_val = false;
static char other_filters[256];

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

void Renderer::RenderUI()
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    ImGui::Begin("Media Organizer");

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::Text("\tA Tool To Organize Your Media Effortlessly");

    ImGui::Dummy(ImVec2(0.0f, 5.0f)); ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0.0f, 5.0f)); ImGui::Dummy(ImVec2(0.0f, 20.0f));

    if (ImGui::RadioButton("Copy", selected_mode == Mode::Mode_COPY)) { selected_mode = Mode::Mode_COPY; }
    ImGui::SameLine(); HelpMarker("Copy files after organizing from source to destination");

    if (ImGui::RadioButton("Move", selected_mode == Mode::Mode_MOVE)) { selected_mode = Mode::Mode_MOVE; }
    ImGui::SameLine(); HelpMarker("Move files after organizing from source to destination");

    if (ImGui::RadioButton("Copy then Delete", selected_mode == Mode::Mode_COPY_AND_DELETE)) { selected_mode = Mode::Mode_COPY_AND_DELETE; }
    ImGui::SameLine(); HelpMarker("Copy files after organizing from source to destination. Then delete the source files.");

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    ImGui::Checkbox(".png", (bool *)&ext_checkboxes[0]);
    ImGui::Checkbox(".jpg", (bool *)&ext_checkboxes[1]);
    ImGui::Checkbox(".jpeg", (bool *)&ext_checkboxes[2]);
    ImGui::Checkbox(".mp4", (bool *)&ext_checkboxes[3]);
    ImGui::Checkbox(".avi", (bool *)&ext_checkboxes[4]);
    ImGui::Checkbox("others", (bool *)&ext_checkboxes[5]);
    ImGui::SameLine();
    HelpMarker("Enter comma separated filters like: mp4,avi,jpg,jpeg,png");

    if(ext_checkboxes[5]) {
        ImGui::InputText("other extensions", &other_filters[0], IM_ARRAYSIZE(other_filters));
        other_filters_str = other_filters;
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f)); ImGui::Dummy(ImVec2(0.0f, 20.0f));
    source_button_val = ImGui::Button("Select Source");

    if(source_button_val)
        source_path = get_selected_path();

    if(source_path.string().length())
    {
        ImGui::SameLine();
        ImGui::Text("Source: %s", source_path.string().c_str());
    }

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    dest_button_val = ImGui::Button("Select Destination");

    if(dest_button_val)
        dest_path = get_selected_path();

    if(dest_path.string().length())
    {
        ImGui::SameLine();
        ImGui::Text("Destination: %s", dest_path.string().c_str());
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
