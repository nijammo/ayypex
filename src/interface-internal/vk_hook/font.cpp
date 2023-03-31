#include "font_default.hpp"
#include "fontdata.hpp"
#include "imgui.h"

bool file_exists(const std::string &path) {
    struct stat s;
    return !stat(path.c_str(), &s) && !S_ISDIR(s.st_mode);
}

void create_fonts(ImFontAtlas *font_atlas, const overlay_params &params,
                  ImFont *&small_font, ImFont *&text_font) {
    auto &io = ImGui::GetIO();
    if (!font_atlas) {
        font_atlas = io.Fonts;
    }
    font_atlas->Clear();

    ImGui::GetIO().FontGlobalScale = params.font_scale;
    static const ImWchar default_range[] = {
        0x0020,
        0x00FF, // Basic Latin + Latin Supplement
        0x2018,
        0x201F, // Bunch of quotation marks
        // 0x0100, 0x017F, // Latin Extended-A
        // 0x2103, 0x2103, // Degree Celsius
        // 0x2109, 0x2109, // Degree Fahrenheit
        0,
    };
    /*
    float font_size = params.font_size;
    if (font_size < FLT_EPSILON)
      font_size = 24;

    float font_size_text = params.font_size_text;
    if (font_size_text < FLT_EPSILON)
      font_size_text = font_size;
    static const ImWchar default_range[] = {
        0x0020,
        0x00FF, // Basic Latin + Latin Supplement
        0x2018,
        0x201F, // Bunch of quotation marks
        // 0x0100, 0x017F, // Latin Extended-A
        // 0x2103, 0x2103, // Degree Celsius
        // 0x2109, 0x2109, // Degree Fahrenheit
        0,
    };
    ImFontConfig* config = new ImFontConfig();
    config->MergeMode = true;
    static const ImWchar icon_ranges[] = {ICON_MIN_FK, ICON_MAX_FK, 0};
    */

    ImVector<ImWchar> glyph_ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(font_atlas->GetGlyphRangesDefault());

    builder.BuildRanges(&glyph_ranges);
    // font_atlas->AddFontDefault();

    font_atlas->AddFontFromMemoryCompressedBase85TTF(
        ttf_rubik_compressed_data_base85, params.font_size, nullptr,
        default_range);

    text_font = font_atlas->Fonts[0];

    font_atlas->Build();
}