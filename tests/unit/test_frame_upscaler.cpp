#include "test_framework.h"
#include "FrameUpscaler.h"
#include "third_party/xbrz/xbrz.h"
#include "SpriteLibBackendSDL.h"
#include <stdexcept>
#include <vector>

namespace {
struct FrameFixture {
    SDL_Surface* target = SDL_CreateRGBSurfaceWithFormat(0, 80, 64, 32, SDL_PIXELFORMAT_ARGB8888);
    SDL_Renderer* renderer = target ? SDL_CreateSoftwareRenderer(target) : nullptr;
    spritectl_surface_s source{};
    FrameFixture() {
        source.surface = SDL_CreateRGBSurfaceWithFormat(0, 16, 16, 16, SDL_PIXELFORMAT_RGB565);
        if (!renderer || !source.surface) throw std::runtime_error(SDL_GetError());
        for (int y = 0; y < 16; ++y) {
            auto* row = reinterpret_cast<uint16_t*>(static_cast<unsigned char*>(source.surface->pixels) + y * source.surface->pitch);
            for (int x = 0; x < 16; ++x) row[x] = x >= y ? 0xffff : 0;
        }
        spritectl_set_xbrz_enabled(0);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    }
    ~FrameFixture() {
        spritectl_set_xbrz_enabled(0);
        spritectl_release_present_resources(renderer);
        if (source.texture) SDL_DestroyTexture(source.texture);
        SDL_FreeSurface(source.surface);
        SDL_DestroyRenderer(renderer);
        SDL_FreeSurface(target);
    }
    std::vector<uint32_t> Draw() {
        SDL_RenderClear(renderer);
        if (spritectl_present_surface(&source, renderer) != 0) throw std::runtime_error(SDL_GetError());
        std::vector<uint32_t> pixels(80 * 64);
        if (SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_ARGB8888, pixels.data(), 80 * 4) != 0)
            throw std::runtime_error(SDL_GetError());
        return pixels;
    }
};
}

TEST(FrameUpscaler, ToggleChangesPixelsAndRestoresOriginalWithoutMovingMouse)
{
    FrameFixture frame;
    const auto original = frame.Draw();
    int x = 40, y = 32;
    spritectl_window_to_game_coords(&x, &y);
    CHECK_EQ(8, x); CHECK_EQ(8, y);
    spritectl_set_xbrz_enabled(1);
    const auto filtered = frame.Draw();
    CHECK(original != filtered);
    CHECK_EQ(1, spritectl_get_xbrz_enabled());
    x = 40; y = 32;
    spritectl_window_to_game_coords(&x, &y);
    CHECK_EQ(8, x); CHECK_EQ(8, y);
    for (int row = 0; row < 64; ++row) for (int column = 0; column < 8; ++column)
        CHECK_EQ(0, filtered[row * 80 + column] & 0xffffff);
    spritectl_set_xbrz_enabled(0);
    CHECK(original == frame.Draw());
}

TEST(FrameUpscaler, ReusesUnchangedFrameAndInvalidatesOnChangeResizeAndRelease)
{
    FrameFixture frame;
    FrameUpscaler upscaler;
    SDL_Rect destination{8, 0, 64, 64};
    CHECK(upscaler.Draw(frame.source.surface, frame.renderer, destination));
    CHECK(upscaler.Draw(frame.source.surface, frame.renderer, destination));
    CHECK_EQ(1, upscaler.FilteredFrames());
    SDL_FillRect(frame.source.surface, nullptr, 0xf800);
    CHECK(upscaler.Draw(frame.source.surface, frame.renderer, destination));
    CHECK_EQ(2, upscaler.FilteredFrames());
    destination.w = destination.h = 32;
    CHECK(upscaler.Draw(frame.source.surface, frame.renderer, destination));
    CHECK_EQ(3, upscaler.FilteredFrames());
    upscaler.Release(frame.renderer);
    upscaler.Release(frame.renderer);
    CHECK(upscaler.Draw(frame.source.surface, frame.renderer, destination));
    CHECK_EQ(4, upscaler.FilteredFrames());
    CHECK(!upscaler.Draw(nullptr, frame.renderer, destination));
    upscaler.Release();
}

TEST(FrameUpscaler, ConvertsPadded565And555RowsWithoutColorBleed)
{
    FrameFixture frame;
    for (const auto format : {SDL_PIXELFORMAT_RGB565, SDL_PIXELFORMAT_RGB555}) {
        SDL_Surface* source = SDL_CreateRGBSurfaceWithFormat(0, 5, 4, 16, format);
        CHECK(source != nullptr);
        if (!source) continue;
        CHECK(source->pitch > 5 * 2);
        // Poison the row padding, then paint the visible image red.
        SDL_memset(source->pixels, 0xff, source->pitch * source->h);
        SDL_FillRect(source, nullptr, SDL_MapRGB(source->format, 255, 0, 0));
        FrameUpscaler upscaler;
        CHECK(upscaler.Draw(source, frame.renderer, SDL_Rect{0, 0, 20, 16}));
        std::vector<uint32_t> pixels(20 * 16);
        SDL_Rect read{0, 0, 20, 16};
        CHECK_EQ(0, SDL_RenderReadPixels(frame.renderer, &read, SDL_PIXELFORMAT_ARGB8888, pixels.data(), 20 * 4));
        for (auto pixel : pixels) CHECK_EQ(0xff0000, pixel & 0xffffff);
        upscaler.Release();
        SDL_FreeSurface(source);
    }
    CHECK_EQ(2, FrameUpscaler::ScaleFactor(1024, 768, SDL_Rect{0, 0, 1440, 1080}));
    CHECK_EQ(3, FrameUpscaler::ScaleFactor(800, 600, SDL_Rect{0, 0, 1920, 1440}));
    CHECK_EQ(4, FrameUpscaler::ScaleFactor(800, 600, SDL_Rect{0, 0, 7680, 4320}));
    CHECK_EQ(0, FrameUpscaler::ScaleFactor(0, 600, SDL_Rect{0, 0, 1920, 1080}));
}

TEST(FrameUpscaler, ParallelRowsMatchSingleThreadedReferenceIncludingSliceBoundaries)
{
    const int width = 257, height = 257, factor = 2;
    SDL_Surface* source = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGB888);
    SDL_Surface* target = SDL_CreateRGBSurfaceWithFormat(0, width * factor, height * factor, 32, SDL_PIXELFORMAT_RGB888);
    SDL_Renderer* renderer = target ? SDL_CreateSoftwareRenderer(target) : nullptr;
    if (!source || !renderer) throw std::runtime_error(SDL_GetError());
    std::vector<uint32_t> pixels(width * height), expected(width * height * factor * factor), actual(expected.size());
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x)
        pixels[y * width + x] = ((x * 9871u) ^ (y * 32573u)) & 0xffffff;
    CHECK_EQ(0, SDL_ConvertPixels(width, height, SDL_PIXELFORMAT_RGB888, pixels.data(), width * 4,
        source->format->format, source->pixels, source->pitch));
    xbrz::scale(factor, pixels.data(), expected.data(), width, height, xbrz::ColorFormat::RGB);
    FrameUpscaler filter;
    CHECK(filter.Draw(source, renderer, SDL_Rect{0, 0, width * factor, height * factor}));
    CHECK_EQ(0, SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGB888, actual.data(), width * factor * 4));
    for (size_t i = 0; i < expected.size(); ++i) CHECK_EQ(expected[i] & 0xffffff, actual[i] & 0xffffff);
    filter.Release();
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(target);
    SDL_FreeSurface(source);
}
