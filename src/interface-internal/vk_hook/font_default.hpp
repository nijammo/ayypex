#pragma once
#include "imgui.h"
#include "overlay_params.hpp"
#include <sys/stat.h>
#include "fontdata.hpp"

//const char* GetDefaultCompressedFontDataTTFBase85(void);
void create_fonts(ImFontAtlas* font_atlas, const overlay_params& params, ImFont*& small_font, ImFont*& text_font);