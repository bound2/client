#include "test_framework.h"
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include "UserOption.h"
#include "KeyAccelerator.h"

namespace {
struct SettingsFixture {
    std::filesystem::path path = "user-option-test.tmp";
    KeyAccelerator keys;
    SettingsFixture() {
        keys.Init(4);
        keys.SetAcceleratorKey(1, DIK_I);
        g_pKeyAccelerator = &keys;
    }
    ~SettingsFixture() {
        g_pKeyAccelerator = nullptr;
        std::filesystem::remove(path);
    }
    std::string Read() {
        std::ifstream file(path, std::ios::binary);
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }
    void Write(const std::string& contents) {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);
        file.write(contents.data(), contents.size());
    }
};
}

TEST(UserOption, XbrzDefaultsOnAndBothChoicesSurviveReload)
{
    SettingsFixture fixture;
    UserOption options;
    CHECK_EQ(TRUE, options.UseXbrz);
    options.DrawFPS = TRUE;
    options.VolumeMusic = 7;
    for (int enabled : {0, 1}) {
        options.UseXbrz = enabled;
        options.SaveToFile(fixture.path.string().c_str());
        UserOption loaded;
        CHECK(loaded.LoadFromFile(fixture.path.string().c_str()));
        CHECK_EQ(enabled, loaded.UseXbrz);
        CHECK_EQ(TRUE, loaded.DrawFPS);
        CHECK_EQ(7, loaded.VolumeMusic);
        CHECK_EQ(DIK_I, fixture.keys.GetKey(1));
    }
}

TEST(UserOption, OldMissingAndInvalidSettingsDefaultXbrzOn)
{
    SettingsFixture fixture;
    UserOption options;
    options.UseXbrz = FALSE;
    options.SaveToFile(fixture.path.string().c_str());
    auto legacy = fixture.Read();
    const auto tail = legacy.rfind("0\tUseXbrz");
    CHECK(tail != std::string::npos);
    if (tail == std::string::npos) return;
    legacy.resize(tail);
    for (const char* extension : {"", "0\tUnknownOption\n", "2\tUseXbrz\n", "0"}) {
        fixture.Write(legacy + extension);
        options.UseXbrz = FALSE;
        CHECK(options.LoadFromFile(fixture.path.string().c_str()));
        CHECK_EQ(TRUE, options.UseXbrz);
    }
    std::filesystem::remove(fixture.path);
    options.UseXbrz = FALSE;
    CHECK(!options.LoadFromFile(fixture.path.string().c_str()));
    CHECK_EQ(TRUE, options.UseXbrz);
}
