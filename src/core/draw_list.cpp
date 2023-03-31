#include "draw_list.h"

#include "imgui.h"
#include "math.h"

void DrawList::add_line(vec2 start, vec2 end, vec3 color) {
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(start.x, start.y), ImVec2(end.x, end.y), ImColor(color.x, color.y, color.z), line_width);
}

void DrawList::add_circle(vec2 center, float radius, vec3 color) {
    ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(center.x, center.y), radius, ImColor(color.x, color.y, color.z), 0, line_width);
}

void DrawList::add_circle_filled(vec2 center, float radius, vec3 color) {
    ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(center.x, center.y), radius, ImColor(color.x, color.y, color.z), 0);
}

void DrawList::add_rect(vec2 start, vec2 end, vec3 color) {
    ImGui::GetBackgroundDrawList()->AddRect(ImVec2(start.x, start.y), ImVec2(end.x, end.y), ImColor(color.x, color.y, color.z), 0, 0, line_width);
}

void DrawList::add_rect_filled(vec2 start, vec2 end, vec3 color) {
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(start.x, start.y), ImVec2(end.x, end.y), ImColor(color.x, color.y, color.z), 0, 0);
}

void DrawList::add_text(vec2 pos, const char* text, vec3 color) {
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(pos.x, pos.y), ImColor(color.x, color.y, color.z), text);
}

void DrawList::add_text_centered(vec2 pos, const char* text, vec3 color) {
    ImFont* font = ImGui::GetFont();
    ImVec2 text_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, text);
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(pos.x - text_size.x / 2, pos.y - text_size.y / 2), ImColor(color.x, color.y, color.z), text);
}

// x_space and y_space are percentages of how much of the rectangle should be empty
void DrawList::add_rect_corners(vec2 start, vec2 end, float x_space, float y_space, vec3 color) {
    vec2 size = end - start;

    vec3 outline_color = vec3(0,0,0);

    vec2 x_space_size = vec2(size.x * x_space / 2, 0);
    vec2 y_space_size = vec2(0, size.y * y_space / 2);

    vec2 corner1(start.x, start.y);
    vec2 corner2(end.x, start.y);
    vec2 corner3(start.x, end.y);
    vec2 corner4(end.x, end.y);

    vec2 x = vec2(1, 0);
    vec2 y = vec2(0, 1);

    line_width = x_space_size.x > 5 ? 4 : 2;

    add_line(corner1 - x, corner1 + x_space_size + x, outline_color);
    add_line(corner1 - y, corner1 + y_space_size + y, outline_color);

    add_line(corner2 + x, corner2 - x_space_size - x, outline_color);
    add_line(corner2 - y, corner2 + y_space_size + y, outline_color);

    add_line(corner3 - x, corner3 + x_space_size + x, outline_color);
    add_line(corner3 + y, corner3 - y_space_size - y, outline_color);
    
    add_line(corner4 + x, corner4 - x_space_size - x, outline_color);
    add_line(corner4 + y, corner4 - y_space_size - y, outline_color);

    line_width /= 2;

    add_line(corner1, corner1 + x_space_size, color);
    add_line(corner1, corner1 + y_space_size, color);

    add_line(corner2, corner2 - x_space_size, color);
    add_line(corner2, corner2 + y_space_size, color);

    add_line(corner3, corner3 + x_space_size, color);
    add_line(corner3, corner3 - y_space_size, color);
    
    add_line(corner4, corner4 - x_space_size, color);
    add_line(corner4, corner4 - y_space_size, color);
}

DrawList draw_list;