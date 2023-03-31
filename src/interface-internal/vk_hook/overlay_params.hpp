#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct overlay_params {
        int control;
        uint32_t fps_sampling_period; /* ns */
        std::vector<std::uint32_t> fps_limit;
        bool help;
        bool no_display;
        bool full;
        bool io_read, io_write, io_stats;
        unsigned width;
        unsigned height;
        int offset_x, offset_y;
        float round_corners;
        unsigned vsync;
        int gl_vsync;
        int gl_bind_framebuffer{-1};
        bool gl_dont_flip{false};
        int64_t log_duration, log_interval;
        unsigned cpu_color, gpu_color, vram_color, ram_color, engine_color,
            io_color, frametime_color, background_color, text_color, wine_color,
            battery_color;
        std::vector<unsigned> gpu_load_color;
        std::vector<unsigned> cpu_load_color;
        std::vector<unsigned> gpu_load_value;
        std::vector<unsigned> cpu_load_value;
        std::vector<unsigned> fps_color;
        std::vector<unsigned> fps_value;
        unsigned media_player_color;
        unsigned table_columns;
        bool no_small_font;
        float font_size, font_scale;
        float font_size_text;
        float font_scale_media_player;
        float background_alpha, alpha;
        float cellpadding_y;
        std::string time_format, output_folder, output_file;
        std::string pci_dev;
        std::string media_player_name;
        std::string cpu_text, gpu_text;
        std::vector<std::string> blacklist;
        unsigned autostart_log;
        std::vector<std::string> media_player_format;
        std::vector<std::string> benchmark_percentiles;
        std::string font_file, font_file_text;
        uint32_t font_glyph_ranges;
        std::string custom_text_center;
        std::string custom_text;
        std::string config_file_path;
        std::unordered_map<std::string, std::string> options;
        int permit_upload;
        int fsr_steam_sharpness;
        unsigned short fcat_screen_edge;
        unsigned short fcat_overlay_width;

        size_t font_params_hash;
};