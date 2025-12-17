#pragma once

#include "util/sprite_sheet.hpp"
#include <glm/vec4.hpp>
#include <nlohmann/json.hpp>

#include <fstream>
#include <string>
#include <unordered_map>
#include <algorithm>

namespace util
{
    struct MsdfGlyph
    {
        float advance = 0.0f;
        float bearingX = 0.0f;
        float bearingY = 0.0f;
        float w = 0.0f; // pixel size in atlas
        float h = 0.0f;
        glm::vec4 uv{0, 0, 1, 1}; // u0,v0,u1,v1
    };

    class MsdfFont
    {
    public:
        bool load(const std::string &json_path, const std::string &png_path)
        {
            using json = nlohmann::json;

            std::ifstream f(json_path);
            if (!f.is_open())
                return false;

            json j;
            f >> j;

            m_atlas_size = j.value("atlasSize", 0);
            if (m_atlas_size <= 0)
                return false;

            // Load the atlas as a 1x1 "sheet" so SpriteRenderer can use its texture.
            // This passes SpriteSheet's validation requirements. :contentReference[oaicite:1]{index=1}
            if (!m_sheet.load_from_file(png_path, m_atlas_size, m_atlas_size, false))
                return false;

            m_glyphs.clear();
            m_line_height = 0.0f;

            const auto &glyphs = j["glyphs"];
            for (auto it = glyphs.begin(); it != glyphs.end(); ++it)
            {
                const int codepoint = std::stoi(it.key());
                const auto &g = it.value();

                MsdfGlyph out{};
                out.advance = g.value("advance", 0.0f);
                out.bearingX = g.value("bearingX", 0.0f);
                out.bearingY = g.value("bearingY", 0.0f);
                out.w = (float)g.value("w", 0);
                out.h = (float)g.value("h", 0);
                
                out.uv = {
                    g.value("u0", 0.0f),
                    g.value("v0", 0.0f),
                    g.value("u1", 1.0f),
                    g.value("v1", 1.0f),
                };

                m_line_height = std::max(m_line_height, out.bearingY);
                m_glyphs.emplace(codepoint, out);
            }

            // Fallback if bearingY was missing/0 for some reason
            if (m_line_height <= 0.0f)
                m_line_height = 48.0f;

            return true;
        }

        const SpriteSheet &sheet() const noexcept { return m_sheet; }
        SpriteSheet &sheet() noexcept { return m_sheet; }

        const MsdfGlyph *glyph(int codepoint) const
        {
            auto it = m_glyphs.find(codepoint);
            if (it == m_glyphs.end())
                return nullptr;
            return &it->second;
        }

        float line_height() const noexcept { return m_line_height; }

    private:
        SpriteSheet m_sheet{};
        int m_atlas_size = 0;
        float m_line_height = 0.0f;
        std::unordered_map<int, MsdfGlyph> m_glyphs{};
    };
}
