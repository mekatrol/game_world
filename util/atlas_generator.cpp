#include <msdfgen.h>
#include <msdfgen-ext.h>
#include <filesystem>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <png.h>
#include <nlohmann/json.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <algorithm>

using json = nlohmann::json;

struct Glyph
{
    float advance;
    float bearingX;
    float bearingY;
    int w, h;
    int x, y; // atlas position in pixels
};

static void SavePNG(const char *path, int w, int h, const std::vector<unsigned char> &rgba)
{
    FILE *fp = fopen(path, "wb");
    if (!fp)
    {
        std::cerr << "Failed to open " << path << " for writing\n";
        return;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png)
    {
        fclose(fp);
        std::cerr << "png_create_write_struct failed\n";
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info)
    {
        png_destroy_write_struct(&png, nullptr);
        fclose(fp);
        std::cerr << "png_create_info_struct failed\n";
        return;
    }

    if (setjmp(png_jmpbuf(png)))
    {
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        std::cerr << "libpng write error\n";
        return;
    }

    png_init_io(png, fp);
    png_set_IHDR(
        png, info, w, h, 8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png, info);

    std::vector<png_bytep> rows(h);
    for (int y = 0; y < h; ++y)
        rows[y] = (png_bytep)&rgba[y * w * 4];

    png_write_image(png, rows.data());
    png_write_end(png, nullptr);

    png_destroy_write_struct(&png, &info);
    fclose(fp);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: atlas_generator font.ttf\n";
        return 1;
    }

    const int firstChar = 32;
    const int lastChar = 126;

    const int glyphPxHeight = 48; // target height in pixels (roughly EM height)
    const int rangePx = 6;        // SDF range in pixels
    const int atlasSize = 1024;

    // ---- FreeType for metrics (advance/bearing) ----
    FT_Library ftLib = nullptr;
    if (FT_Init_FreeType(&ftLib))
    {
        std::cerr << "FT_Init_FreeType failed\n";
        return 1;
    }

    FT_Face face = nullptr;
    if (FT_New_Face(ftLib, argv[1], 0, &face))
    {
        std::cerr << "FT_New_Face failed: " << argv[1] << "\n";
        FT_Done_FreeType(ftLib);
        return 1;
    }

    FT_Set_Pixel_Sizes(face, 0, glyphPxHeight);
    double pxPerUnit = 0.0;

    // ---- msdfgen for outlines ----
    msdfgen::FreetypeHandle *msft = msdfgen::initializeFreetype();
    msdfgen::FontHandle *font = msdfgen::loadFont(msft, argv[1]);
    if (!font)
    {
        std::cerr << "msdfgen::loadFont failed\n";
        FT_Done_Face(face);
        FT_Done_FreeType(ftLib);
        msdfgen::deinitializeFreetype(msft);
        return 1;
    }

    auto CalibrateScale = [&](int c) -> bool
    {
        if (FT_Load_Char(face, (FT_ULong)c, FT_LOAD_NO_BITMAP))
            return false;

        msdfgen::Shape shape;
        if (!msdfgen::loadGlyph(shape, font, c))
            return false;

        msdfgen::edgeColoringSimple(shape, 3.0);

        const auto b = shape.getBounds();
        const double msdfH = (b.t - b.b);

        // FreeType glyph height in pixels
        const double ftH = face->glyph->metrics.height / 64.0;

        if (!(msdfH > 0.0) || !(ftH > 0.0))
            return false;

        pxPerUnit = ftH / msdfH;
        std::cerr << "Calibrated pxPerUnit=" << pxPerUnit
                  << " (ftH=" << ftH << ", msdfH=" << msdfH << ")\n";
        return true;
    };

    if (!CalibrateScale('M') && !CalibrateScale('H') && !CalibrateScale('A'))
    {
        std::cerr << "Failed to calibrate scale\n";
        return 1;
    }

    std::unordered_map<int, Glyph> glyphs;
    std::vector<unsigned char> atlas(atlasSize * atlasSize * 4, 0);

    int penX = rangePx;
    int penY = rangePx;
    int rowH = 0;

    for (int c = firstChar; c <= lastChar; ++c)
    {
        // Load glyph in FreeType to get metrics
        if (FT_Load_Char(face, (FT_ULong)c, FT_LOAD_NO_BITMAP))
            continue;

        msdfgen::Shape shape;
        if (!msdfgen::loadGlyph(shape, font, c))
            continue;

        // shape.normalize();
        msdfgen::edgeColoringSimple(shape, 3.0);

        msdfgen::Shape::Bounds b = shape.getBounds();

        // Convert shape bounds (font units) -> pixels using pxPerUnit
        const double wPx = (b.r - b.l) * pxPerUnit;
        const double hPx = (b.t - b.b) * pxPerUnit;

        int gw = int(std::ceil(wPx)) + rangePx * 2;
        int gh = int(std::ceil(hPx)) + rangePx * 2;

        static int maxGW = 0, maxGH = 0;
        maxGW = std::max(maxGW, gw);
        maxGH = std::max(maxGH, gh);
        if (c == lastChar)
        {
            std::cerr << "maxGW=" << maxGW << " maxGH=" << maxGH << "\n";
        }

        if (gw <= 0 || gh <= 0)
            continue;

        if (penX + gw >= atlasSize)
        {
            penX = rangePx;
            penY += rowH + rangePx;
            rowH = 0;
        }

        if (penY + gh >= atlasSize)
        {
            std::cerr << "Atlas full; increase atlasSize\n";
            break;
        }

        msdfgen::Bitmap<float, 4> bmp(gw, gh);

        // Transform: shapeCoord * scale + translate = pixelCoord
        const msdfgen::Vector2 scale(pxPerUnit, pxPerUnit);
        const msdfgen::Vector2 translate(
            rangePx - b.l * pxPerUnit,
            rangePx - b.b * pxPerUnit);

        msdfgen::generateMTSDF(bmp, shape, rangePx, scale, translate);

        for (int y = 0; y < gh; ++y)
        {
            for (int x = 0; x < gw; ++x)
            {
                int dst = ((penY + y) * atlasSize + (penX + x)) * 4;
                for (int i = 0; i < 4; ++i)
                {
                    atlas[dst + i] = (unsigned char)std::clamp(
                        bmp(x, y)[i] * 255.0f, 0.0f, 255.0f);
                }
            }
        }

        FT_GlyphSlot slot = face->glyph;

        Glyph g{};
        g.advance = slot->advance.x / 64.0f;
        g.bearingX = slot->metrics.horiBearingX / 64.0f;
        g.bearingY = slot->metrics.horiBearingY / 64.0f;
        g.w = gw;
        g.h = gh;
        g.x = penX;
        g.y = penY;

        glyphs[c] = g;

        penX += gw + rangePx;
        rowH = std::max(rowH, gh);
    }

    std::filesystem::create_directories("assets/fonts");

    SavePNG("assets/fonts/font.png", atlasSize, atlasSize, atlas);

    json j;
    j["atlasSize"] = atlasSize;

    for (auto &[c, g] : glyphs)
    {
        j["glyphs"][std::to_string(c)] = {
            {"advance", g.advance},
            {"bearingX", g.bearingX},
            {"bearingY", g.bearingY},
            {"w", g.w},
            {"h", g.h},
            {"u0", float(g.x) / atlasSize},
            {"v0", float(g.y) / atlasSize},
            {"u1", float(g.x + g.w) / atlasSize},
            {"v1", float(g.y + g.h) / atlasSize}};
    }

    std::ofstream("assets/fonts/font.json") << j.dump(2);

    msdfgen::destroyFont(font);
    msdfgen::deinitializeFreetype(msft);

    FT_Done_Face(face);
    FT_Done_FreeType(ftLib);

    return 0;
}
