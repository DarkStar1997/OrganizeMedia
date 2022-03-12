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
#include <md5.h>

static std::vector<std::string> month = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};
static std::unordered_map<std::string, std::string> hash_data; // map of [hash - filename]

std::filesystem::path get_output_dir(std::tm * timestamp, const std::filesystem::path output_path)
{
    return output_path / std::to_string(1900 + timestamp->tm_year) / month[timestamp->tm_mon] / std::to_string(timestamp->tm_mday);
}

void generate_log(const std::vector<std::string> & duplicates,
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

namespace status
{
    void update_bar(int percent)
    {
        int total = 100;
        int done = (percent / 100.0) * total;
        fmt::print("\r[");
        fmt::print("{:>>{}}", "", done);
        fmt::print("{:->{}}", "", total - done);
        fmt::print("]");
        fmt::print(" {:>2}%", percent);
        fflush(stdout);
    }
};

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
    if(mode != "copy" && mode != "move" && mode != "copy&delete")
        throw std::runtime_error("Incorrect option for mode. Available options: [copy, move, copy&delete]");
    fmt::print("Selected mode: {}\n", mode);
    
    size_t count = 0, duplicates = 0;
    uint64_t file_count = 0;
    
    fmt::print("Computing total number of potential files\n");
    for(const auto& dir_entry : std::filesystem::recursive_directory_iterator(input))
        if(std::filesystem::is_regular_file(dir_entry.path()) && std::find(extensions.begin(), extensions.end(), dir_entry.path().extension()) != extensions.end())
            file_count++;
    fmt::print("Files to be operated: {}\n", file_count);

    if(file_count == 0)
        exit(0);
    
    std::vector<std::string> duplicate_list; duplicate_list.reserve(file_count);
    std::vector<std::pair<std::string, std::string>> renamed_list; renamed_list.reserve(file_count);

    for(const auto& dir_entry : std::filesystem::recursive_directory_iterator(input))
    {
        if(std::filesystem::is_regular_file(dir_entry.path()) && std::find(extensions.begin(), extensions.end(), dir_entry.path().extension()) != extensions.end())
        {
            struct stat result;
            if(stat(dir_entry.path().string().c_str(), &result) == 0)
            {
                std::tm * timestamp = std::localtime(&result.st_mtime);
                auto output_path = get_output_dir(timestamp, std::filesystem::path(output));
                auto output_file = output_path / dir_entry.path().filename();
                if(!std::filesystem::exists(output_path))
                    std::filesystem::create_directories(output_path);
                if(std::filesystem::exists(output_file))
                {
                    //fmt::print("Computing hash for potential duplicate output file {}\n", output_file.string());
                    std::string output_file_hash = computeHash(Chocobo1::MD5(), output_file.string());
                    if(hash_data.find(output_file_hash) == hash_data.end())
                        hash_data.insert({output_file_hash, output_file.filename().string()});
                    //fmt::print("Computing hash for potential duplicate input file {}\n", dir_entry.path().string());
                    std::string input_file_hash = computeHash(Chocobo1::MD5(), dir_entry.path().string());
                    
                    if(input_file_hash == output_file_hash)
                    {
                        //fmt::print("{} {} are identical\n", dir_entry.path().string(), output_file.string());
                        duplicates++;
                        status::update_bar(((count + duplicates) / (double)file_count) * 100.0);
                        duplicate_list.push_back(dir_entry.path().string());
                        continue;
                    }
                    
                    auto data = hash_data.find(input_file_hash);
                    if(data != hash_data.end())
                    {
                        //fmt::print("{} with hash {} already present with filename {}\n", dir_entry.path().string(), data->first, data->second);
                        duplicates++;
                        status::update_bar(((count + duplicates) / (double)file_count) * 100.0);
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
                if(mode == "copy")
                    std::filesystem::copy_file(dir_entry.path(), output_file);
                else if(mode == "move")
                    std::filesystem::rename(dir_entry.path(), output_file);
                else if(mode == "copy&delete")
                {
                    std::filesystem::copy_file(dir_entry.path(), output_file);
                    std::filesystem::remove(dir_entry.path());
                }
                count++;
            }
        }
        status::update_bar(((count + duplicates) / (double)file_count) * 100.0);
    }

    fmt::print("\n");
    fmt::print("{} New files found\n {} Duplicate files\n", count, duplicates);
    fmt::print("Generating log\n");
    generate_log(duplicate_list, renamed_list);
}
