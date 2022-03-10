#include <fmt/core.h>
#include <filesystem>
#include <chrono>
#include <iterator>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include <exception>

std::vector<std::string> weekday = {"sun", "mon", "tue", "wed", "thur", "fri", "sat"};
std::vector<std::string> month = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};

std::filesystem::path get_output_dir(struct stat * result, const std::filesystem::path output_path)
{
    std::tm * timestamp = std::localtime(&result->st_mtime);
    return output_path / std::to_string(1900 + timestamp->tm_year) / month[timestamp->tm_mon] / std::to_string(timestamp->tm_mday);
}

int main()
{
    std::string config_file_name = "rmg_config.json";
    if(!std::filesystem::exists(config_file_name))
    {
        fmt::print("{} not present\n", config_file_name);
        exit(0);
    }
    nlohmann::json config_data;
    {
        std::ifstream input_config; input_config.open(config_file_name);
        config_data = nlohmann::json::parse(input_config, nullptr, true, true);
    }
    std::string input = config_data["input_dir"];
    std::string output = config_data["output_dir"];
    std::vector<std::string> extensions = config_data["extensions"];
    std::string mode = config_data["mode"];
    if(mode != "copy" && mode != "move")
        throw std::runtime_error("Incorrect option for mode. Available options: [copy, move]");
    size_t count = 0, duplicates = 0;

    for(const auto& dir_entry : std::filesystem::recursive_directory_iterator(input))
    {
        if(std::find(extensions.begin(), extensions.end(), dir_entry.path().extension()) != extensions.end())
        {
            struct stat result;
            if(stat(dir_entry.path().string().c_str(), &result) == 0)
            {
                auto output_path = get_output_dir(&result, std::filesystem::path(output));
                auto output_file = output_path / dir_entry.path().filename();
                if(!std::filesystem::exists(output_path))
                    std::filesystem::create_directories(output_path);
                if(std::filesystem::exists(output_file))
                {
                    duplicates++;
                    continue;
                }
                if(mode == "copy")
                    std::filesystem::copy_file(dir_entry.path(), output_file);
                else if(mode == "move")
                    std::filesystem::rename(dir_entry.path(), output_file);
                count++;
            }
        }
    }

    fmt::print("{} New files found\n {} Duplicate files\n", count, duplicates);
}
