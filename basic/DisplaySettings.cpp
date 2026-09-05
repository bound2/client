#include "DisplaySettings.h"
#include <filesystem>
#include <fstream>

DisplaySettings& GetDisplaySettings()
{
    static DisplaySettings settings;
    return settings;
}

bool DisplaySettings::Load(const char* filename)
{
    filename_ = filename;
    fullscreen = true;
    highResolution = false;
    std::ifstream file(filename_);
    if (!file) return false;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line == "Fullscreen=0") fullscreen = false;
        else if (line == "Fullscreen=1") fullscreen = true;
        else if (line == "Resolution=800x600") highResolution = false;
        else if (line == "Resolution=1024x768") highResolution = true;
    }
    return !file.bad();
}

bool DisplaySettings::Save() const
{
    std::error_code error;
    const auto parent = std::filesystem::path(filename_).parent_path();
    if (!parent.empty()) std::filesystem::create_directories(parent, error);
    if (error) return false;
    std::ofstream file(filename_, std::ios::trunc);
    file << "Fullscreen=" << (fullscreen ? 1 : 0) << '\n'
         << "Resolution=" << (highResolution ? "1024x768" : "800x600") << '\n';
    file.close();
    return !file.fail();
}

std::string DisplaySettings::LaunchCommand(std::string_view argument)
{
    if (argument.find_first_not_of(" \t\r\n") == std::string_view::npos) {
        const char mode = (highResolution ? '3' : '1') + (fullscreen ? 1 : 0);
        return std::string("000000000") + mode;
    }
    // Reflect old launcher overrides in Options; saving remains explicit.
    const char mode = argument.back();
    if (mode >= '1' && mode <= '4') {
        fullscreen = mode == '2' || mode == '4';
        highResolution = mode == '3' || mode == '4';
    }
    return std::string(argument);
}
