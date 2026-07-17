#include "MapEditorView.hpp"

#include "PlacementSystem.hpp"
#include <imgui.h>

sf::Vector2f isoToScreen(int x, int y, float z)
{
    const float ratio = 0.75f;
    float screenX = (x - y) * (TILE_WIDTH / 2.f);
    float screenY = (x + y) * (TILE_HEIGHT / 2.f) * ratio;
    screenY -= (z * 24.0f);

    return {screenX, screenY};
}

bool DrawMapViewWindow(bool allowWindowMove, const sf::View& mapView, int currentLayer, int hoveredX, int hoveredY,
                       const PlacementSystem& placementSystem)
{
    bool isMouseOverMapView = false;
    ImGuiWindowFlags mapViewFlags = allowWindowMove ? 0 : ImGuiWindowFlags_NoMove;
    if (ImGui::Begin("Map View", nullptr, mapViewFlags))
    {
        isMouseOverMapView = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

        float zoom = mapView.getSize().x / 1400.0f;
        ImGui::TextColored(ImVec4(0.85f, 0.9f, 1.0f, 1.0f), "Map View");
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.85f, 1.0f, 1.0f), "Layer: %d", currentLayer);
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.6f, 1.0f), "Zoom: %.2fx", zoom);
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 1.0f, 0.7f, 1.0f), "Cursor: (%d, %d)", hoveredX, hoveredY);
        ImGui::Separator();

        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.5f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.7f, 1.0f, 1.0f));

        ImGui::BeginChild("MapCanvas", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

        for (int y = 0; y < MAP_HEIGHT; ++y)
        {
            for (int x = 0; x < MAP_WIDTH; ++x)
            {
                sf::Vector2f pos = isoToScreen(x, y);
                pos.x += canvasPos.x + 400;
                pos.y += canvasPos.y + 100;

                sf::Color color = (currentLayer == 0)   ? sf::Color(80, 140, 80)
                                  : (currentLayer == 1) ? sf::Color(120, 100, 70)
                                  : (currentLayer == 2) ? sf::Color(180, 80, 80)
                                                        : sf::Color(100, 100, 120);

                if (x == hoveredX && y == hoveredY)
                {
                    color = sf::Color(255, 255, 100, 120);
                }

                ImVec2 p1 = {pos.x, pos.y};
                ImVec2 p2 = {pos.x + TILE_WIDTH / 2, pos.y + TILE_HEIGHT / 2};
                ImVec2 p3 = {pos.x, pos.y + TILE_HEIGHT};
                ImVec2 p4 = {pos.x - TILE_WIDTH / 2, pos.y + TILE_HEIGHT / 2};

                ImU32 col = IM_COL32(color.r, color.g, color.b, color.a);
                drawList->AddQuadFilled(p1, p2, p3, p4, col);
                drawList->AddQuad(p1, p2, p3, p4, IM_COL32(40, 40, 40, 255), 1.0f);
            }
        }

        for (const auto& obj : placementSystem.GetAllObjects())
        {
            sf::Vector2f pos = isoToScreen(obj.x, obj.y);
            pos.x += canvasPos.x + 400;
            pos.y += canvasPos.y + 100;

            sf::Color color;
            switch (obj.category)
            {
            case PlacementCategory::Wall:
                color = (obj.connectionMask > 0) ? sf::Color(70, 130, 180) : sf::Color(100, 100, 120);
                break;
            case PlacementCategory::Prop:
                color = sf::Color(139, 90, 43);
                break;
            default:
                color = sf::Color(80, 140, 80);
                break;
            }

            ImVec2 p1 = {pos.x, pos.y};
            ImVec2 p2 = {pos.x + TILE_WIDTH / 2, pos.y + TILE_HEIGHT / 2};
            ImVec2 p3 = {pos.x, pos.y + TILE_HEIGHT};
            ImVec2 p4 = {pos.x - TILE_WIDTH / 2, pos.y + TILE_HEIGHT / 2};

            ImU32 col = IM_COL32(color.r, color.g, color.b, color.a);
            drawList->AddQuadFilled(p1, p2, p3, p4, col);
            drawList->AddQuad(p1, p2, p3, p4, IM_COL32(0, 0, 0, 255), 1.5f);
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
    ImGui::End();

    return isMouseOverMapView;
}
