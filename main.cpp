#include <fmt/core.h>
#include <filesystem>
#include <chrono>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

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
        input_config >> config_data;
    }
    std::string input = config_data["input_dir"];
    std::string output = config_data["output_dir"];
    std::vector<std::string> extensions = config_data["extensions"];
    size_t count = 0, duplicates = 0;

    for(const auto& dir_entry : std::filesystem::recursive_directory_iterator(input))
    {
        if(std::find(extensions.begin(), extensions.end(), dir_entry.path().extension()) != extensions.end())
        {
            struct stat result;
            if(stat((const char*)dir_entry.path().c_str(), &result) == 0)
            {
                auto output_path = get_output_dir(&result, std::filesystem::path(output));
                if(!std::filesystem::exists(output_path))
                    std::filesystem::create_directories(output_path);
                uint8_t isUnique = std::filesystem::copy_file(dir_entry.path(), output_path / dir_entry.path().filename(), std::filesystem::copy_options::skip_existing);
                count += isUnique;
                if(!isUnique)
                    duplicates++;
            }
        }
    }

    fmt::print("{} New files found\n {} Duplicate files\n", count, duplicates);
}
