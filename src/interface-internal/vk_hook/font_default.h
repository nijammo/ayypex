#pragma once
#include "imgui.h"
#include <sys/stat.h>
#include "fontdata.h"

//const char* GetDefaultCompressedFontDataTTFBase85(void);
void create_fonts(ImFontAtlas* font_atlas, ImFont*& small_font, ImFont*& text_font);