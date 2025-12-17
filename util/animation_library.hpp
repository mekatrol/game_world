#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace util
{
    struct FrameSequence
    {
        std::vector<unsigned int> frames;
        double seconds_per_frame = 0.1;
    };

    struct AnimationDef
    {
        std::string key;
        std::string asset_file;
        int sprite_count_x = 0;
        int sprite_count_y = 0;

        // sequenceName -> sequence data
        std::unordered_map<std::string, FrameSequence> sequences;
    };

    using AnimationLibrary = std::unordered_map<std::string, AnimationDef>;

    AnimationLibrary load_animation_library(const std::string &directory);
}
