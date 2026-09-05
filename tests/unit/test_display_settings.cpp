#include "test_framework.h"
#include "DisplaySettings.h"
#include <filesystem>
#include <fstream>

namespace {
struct DisplayFile {
    const char* path = "display_settings_test.bin";
    ~DisplayFile() { std::filesystem::remove(path); }
};
}

TEST(DisplaySettings, NoArgumentsDefaultToScaledFullscreen)
{
    DisplaySettings settings;
    CHECK(settings.fullscreen);
    CHECK(!settings.highResolution);
    CHECK(settings.LaunchCommand("") == "0000000002");
    CHECK(settings.LaunchCommand(" \t") == "0000000002");
}

TEST(DisplaySettings, AllFourSavedModesLaunchWithoutArguments)
{
    DisplayFile file;
    for (int mode = 1; mode <= 4; ++mode) {
        DisplaySettings saved;
        saved.Load(file.path);
        saved.fullscreen = mode == 2 || mode == 4;
        saved.highResolution = mode >= 3;
        CHECK(saved.Save());
        DisplaySettings loaded;
        CHECK(loaded.Load(file.path));
        CHECK_EQ(saved.fullscreen, loaded.fullscreen);
        CHECK_EQ(saved.highResolution, loaded.highResolution);
        CHECK(loaded.LaunchCommand("") == "000000000" + std::to_string(mode));
    }
}

TEST(DisplaySettings, LegacyOverridesDoNotOverwriteSavedPreferences)
{
    DisplayFile file;
    DisplaySettings saved;
    saved.Load(file.path);
    saved.fullscreen = true;
    saved.highResolution = false;
    CHECK(saved.Save());
    for (int mode = 1; mode <= 4; ++mode) {
        DisplaySettings launch;
        CHECK(launch.Load(file.path));
        const auto argument = "000000000" + std::to_string(mode);
        CHECK(launch.LaunchCommand(argument) == argument);
        CHECK_EQ(mode == 2 || mode == 4, launch.fullscreen);
        CHECK_EQ(mode >= 3, launch.highResolution);
        DisplaySettings next;
        CHECK(next.Load(file.path));
        CHECK(next.LaunchCommand("") == "0000000002");
    }
}

TEST(DisplaySettings, MissingAndInvalidPreferencesUseDefaults)
{
    DisplayFile file;
    std::filesystem::remove(file.path);
    DisplaySettings settings;
    settings.fullscreen = false;
    settings.highResolution = true;
    CHECK(!settings.Load(file.path));
    CHECK(settings.LaunchCommand("") == "0000000002");
    {
        std::ofstream output(file.path, std::ios::binary);
        output << "Fullscreen=invalid\r\nResolution=99999x99999\r\nUnknown=1\r\n";
    }
    CHECK(settings.Load(file.path));
    CHECK(settings.LaunchCommand("") == "0000000002");
    {
        std::ofstream output(file.path, std::ios::binary);
        output << "Fullscreen=0\r\nResolution=1024x768\r\n";
    }
    CHECK(settings.Load(file.path));
    CHECK(settings.LaunchCommand("") == "0000000003");
}
