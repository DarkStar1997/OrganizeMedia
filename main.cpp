#include <algorithm>
#include <cstdlib>
#include <fmt/core.h>
#include <filesystem>
#include <chrono>
#include <string>
#include <argh.h>
#include <iostream>
#include <vector>

std::vector<std::string> weekday = {"sun", "mon", "tue", "wed", "thur", "fri", "sat"};
std::vector<std::string> month = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};

std::filesystem::path get_output_dir(std::filesystem::file_time_type const& ftime, const std::filesystem::path output_path)
{
    std::time_t cftime = std::chrono::system_clock::to_time_t(
            std::chrono::file_clock::to_sys(ftime));
    std::tm * timestamp = std::localtime(&cftime);
    return output_path / std::to_string(1900 + timestamp->tm_year) / month[timestamp->tm_mon] / std::to_string(timestamp->tm_mday);
}

int main(int argc, char **argv)
{
    std::string help_text = "Usage:\n"
                            "./rmg_organize --input [input dir name] --output [output dir name]\n";
    argh::parser cmdl;
    cmdl.parse(argc, argv, argh::parser::Mode::PREFER_PARAM_FOR_UNREG_OPTION);
    if(cmdl["help"])
    {
        fmt::print("{}\n", help_text);
        exit(0);
    }
    std::string input, output;
    
    if(!(cmdl({"i", "input"}) >> input))
        fmt::print("Provide input directory with -i or --input\n");
    else
    {
        if(!std::filesystem::exists(input))
        {
            fmt::print("Input directory {} not present\n", input);
            exit(0);
        }
        fmt::print("Input directory set to {}\n", input);
    }
    
    if(!(cmdl({"o", "output"}) >> output))
        fmt::print("Provide output directory with -o or --output\n");
    else
        fmt::print("Output directory set to {}\n", output);

    std::vector<std::string> extensions = {".json", ".cpp", ".cache"};
    size_t count = 0, duplicates = 0;

    for(const auto& dir_entry : std::filesystem::recursive_directory_iterator(input))
    {
        if(std::find(extensions.begin(), extensions.end(), dir_entry.path().extension()) != extensions.end())
        {
            auto output_path = get_output_dir(std::filesystem::last_write_time(dir_entry), std::filesystem::path(output));
            if(!std::filesystem::exists(output_path))
                std::filesystem::create_directories(output_path);
            uint8_t isUnique = std::filesystem::copy_file(dir_entry.path(), output_path / dir_entry.path().filename(), std::filesystem::copy_options::skip_existing);
            count += isUnique;
            if(!isUnique)
                duplicates++;
        }
    }

    fmt::print("{} New files found\n {} Duplicate files\n", count, duplicates);
}
