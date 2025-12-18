#include "util/animation_library.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

// If you don't already have this in your project, add it via your dependency system
// (or vendor the single header).
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using nlohmann::json;

namespace util
{
    static AnimationDef parse_animation_def(const fs::path &path)
    {
        std::ifstream f(path);
        if (!f.is_open())
        {
            throw std::runtime_error("Failed to open animation json: " + path.string());
        }

        json j;
        f >> j;

        AnimationDef def;
        def.key = j.at("key").get<std::string>();
        def.asset_file = j.at("assetFile").get<std::string>();
        def.sprite_count_x = j.at("spriteCountX").get<int>();
        def.sprite_count_y = j.at("spriteCountY").get<int>();

        if (j.contains("assetMaskFile"))
        {
            def.asset_mask_file = j.at("assetMaskFile").get<std::string>();
        }

        if (j.contains("assetShadowFile"))
        {
            def.asset_shadow_file = j.at("assetShadowFile").get<std::string>();
        }

        const auto &seqs = j.at("frameSequences");
        if (!seqs.is_object())
        {
            throw std::runtime_error("frameSequences must be an object in: " + path.string());
        }

        for (auto it = seqs.begin(); it != seqs.end(); ++it)
        {
            const std::string seq_name = it.key();
            const auto &seq_obj = it.value();

            if (!seq_obj.is_object())
            {
                throw std::runtime_error(
                    "frameSequences['" + seq_name + "'] must be an object in: " + path.string());
            }

            FrameSequence seq;
            seq.seconds_per_frame = seq_obj.at("secondsPerFrame").get<double>();

            const auto &frames = seq_obj.at("frames");
            if (!frames.is_array())
            {
                throw std::runtime_error(
                    "frames must be an array in sequence '" + seq_name + "' in: " + path.string());
            }

            seq.frames.reserve(frames.size());
            for (const auto &v : frames)
            {
                seq.frames.push_back(v.get<unsigned int>());
            }

            def.sequences.emplace(seq_name, std::move(seq));
        }

        return def;
    }

    AnimationLibrary load_animation_library(const std::string &directory)
    {
        AnimationLibrary lib;

        const fs::path dir_path{directory};
        if (!fs::exists(dir_path) || !fs::is_directory(dir_path))
        {
            throw std::runtime_error("Animation directory not found: " + directory);
        }

        for (const auto &entry : fs::directory_iterator(dir_path))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            const auto &p = entry.path();
            if (p.extension() != ".json")
            {
                continue;
            }

            AnimationDef def = parse_animation_def(p);

            if (def.key.empty())
            {
                throw std::runtime_error("Animation json has empty key: " + p.string());
            }

            const auto [_, inserted] = lib.emplace(def.key, std::move(def));
            if (!inserted)
            {
                throw std::runtime_error("Duplicate animation key '" + lib[def.key].key + "' from: " + p.string());
            }
        }

        return lib;
    }
}
