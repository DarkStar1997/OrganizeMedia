#include <algorithm>
#include <fmt/core.h>
#include <filesystem>
#include <chrono>
#include <string>
#include <argh.h>
#include <iostream>
#include <vector>

std::vector<std::string> weekday = {"sun", "mon", "tue", "wed", "thur", "fri", "sat"};
std::vector<std::string> month = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};

std::string formatted_last_write_time(std::filesystem::file_time_type const& ftime)
{
    std::time_t cftime = std::chrono::system_clock::to_time_t(
            std::chrono::file_clock::to_sys(ftime));
    //auto res = std::string(std::asctime(std::localtime(&cftime)));
    //return res.substr(0, res.length() - 1);
    std::tm * timestamp = std::localtime(&cftime);
    return fmt::format("{} {} {}", timestamp->tm_mday, month[timestamp->tm_mon], weekday[timestamp->tm_wday]);
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
        fmt::print("Input directory set to {}\n", input);
    
    if(!(cmdl({"o", "output"}) >> output))
        fmt::print("Provide output directory with -o or --output\n");
    else
        fmt::print("Output directory set to {}\n", output);

    std::vector<std::string> extensions = {".json", ".cpp", ".cache"};
    size_t count = 0;

    for(const auto& dir_entry : std::filesystem::recursive_directory_iterator(input))
    {
        if(std::find(extensions.begin(), extensions.end(), dir_entry.path().extension()) != extensions.end())
        {
            fmt::print("[{}] - {}\n", dir_entry.path().string(), formatted_last_write_time(std::filesystem::last_write_time(dir_entry)));
            count++;
        }
    }

    fmt::print("{} files found\n", count);
}
