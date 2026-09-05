#pragma once
#include <SDL.h>
#include <cstdint>
#include <vector>

// All calls are on the render thread. Release BEFORE destroying the SDL renderer;
// SDL owns texture destruction as part of renderer teardown.
class FrameUpscaler {
public:
    bool Draw(SDL_Surface* source, SDL_Renderer* renderer, const SDL_Rect& destination);
    void Release(SDL_Renderer* renderer = nullptr);
    static int ScaleFactor(int sourceWidth, int sourceHeight, const SDL_Rect& destination);
    unsigned long long FilteredFrames() const { return filteredFrames_; }
private:
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
    int width_ = 0, height_ = 0, factor_ = 0;
    bool valid_ = false;
    unsigned long long filteredFrames_ = 0;
    std::vector<uint32_t> current_, previous_, output_;
};
