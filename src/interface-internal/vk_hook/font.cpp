#include "font_default.h"
#include "fontdata.h"
#include "imgui.h"
#include <string>

bool file_exists(const std::string &path) {
    struct stat s;
    return !stat(path.c_str(), &s) && !S_ISDIR(s.st_mode);
}

void create_fonts(ImFontAtlas *font_atlas, ImFont *&small_font, ImFont *&text_font) {
    auto &io = ImGui::GetIO();
    if (!font_atlas) {
        font_atlas = io.Fonts;
    }
    font_atlas->Clear();

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
    
    ImVector<ImWchar> glyph_ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(font_atlas->GetGlyphRangesDefault());

    builder.BuildRanges(&glyph_ranges);
    // font_atlas->AddFontDefault();

    font_atlas->AddFontFromMemoryCompressedBase85TTF(
        ttf_rubik_compressed_data_base85, 16, nullptr,
        default_range);

    text_font = font_atlas->Fonts[0];

    font_atlas->Build();
}