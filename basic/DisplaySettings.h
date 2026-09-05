#pragma once

#include <string>
#include <string_view>

// Loaded before the window and assets, independently of the legacy user file.
// Display changes take effect on the next launch.
struct DisplaySettings {
    bool fullscreen = true;
    bool highResolution = false;

    bool Load(const char* filename = "UserSet/Display.ini");
    bool Save() const;
    std::string LaunchCommand(std::string_view argument);

private:
    std::string filename_ = "UserSet/Display.ini";
};

DisplaySettings& GetDisplaySettings();
