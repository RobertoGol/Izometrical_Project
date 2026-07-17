#include "MapEditorPanels.hpp"

#include "MapEditorConsole.hpp"
#include <cstring>
#include <imgui.h>

void DrawConsoleWindow(bool showConsole, bool allowWindowMove, MapEditorConsole& console)
{
    if (!showConsole)
    {
        return;
    }

    ImGuiWindowFlags consoleFlags = allowWindowMove ? 0 : ImGuiWindowFlags_NoMove;
    if (ImGui::Begin("Map Editor Console", nullptr, consoleFlags))
    {
        static char inputBuffer[256] = "";

        bool reclaim_focus = false;
        if (ImGui::InputText("##cmd", inputBuffer, sizeof(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (strlen(inputBuffer) > 0)
            {
                console.Execute(inputBuffer);
                inputBuffer[0] = '\0';
                reclaim_focus = true;
            }
        }
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
        {
            ImGui::SetKeyboardFocusHere(-1);
        }

        ImGui::SameLine();
        if (ImGui::Button("Execute") && strlen(inputBuffer) > 0)
        {
            console.Execute(inputBuffer);
            inputBuffer[0] = '\0';
        }

        ImGui::BeginChild("History", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& line : console.history)
        {
            ImGui::TextUnformatted(line.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void DrawInstructionsWindow(bool showInstructions)
{
    if (!showInstructions)
    {
        return;
    }

    if (ImGui::Begin("Instructions"))
    {
        if (ImGui::CollapsingHeader("Р“РѕСЂСЏС‡РёРµ РєР»Р°РІРёС€Рё", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("WASD         вЂ” РџРµСЂРµРјРµС‰РµРЅРёРµ РєР°РјРµСЂС‹ (РєРѕРіРґР° РјС‹С€СЊ РЅР° РєР°СЂС‚Рµ)");
            ImGui::Text("NumPad +/-   вЂ” Р—СѓРј");
            ImGui::Text("Delete       вЂ” РЈРґР°Р»РёС‚СЊ РѕР±СЉРµРєС‚ РїРѕРґ РєСѓСЂСЃРѕСЂРѕРј");
            ImGui::Text("Р›РљРњ          вЂ” Р Р°Р·РјРµСЃС‚РёС‚СЊ РѕР±СЉРµРєС‚/СЃС‚РµРЅСѓ/С‚Р°Р№Р»");
        }
        if (ImGui::CollapsingHeader(
                "РџСЂРёРјРµСЂС‹ РїСЂРѕРјРїС‚РѕРІ РґР»СЏ РР"))
        {
            ImGui::TextWrapped("вЂў РЎРѕР·РґР°Р№ РІРѕРµРЅРЅСѓСЋ Р±Р°Р·Сѓ СЃ 3 Р·РґР°РЅРёСЏРјРё Рё 2 С‚СѓСЂРµР»СЏРјРё.");
            ImGui::TextWrapped("вЂў РќРµР±РѕР»СЊС€Р°СЏ РґРµСЂРµРІРЅСЏ СЃ 5 РґРѕРјР°РјРё Рё РєРѕР»РѕРґС†РµРј.");
            ImGui::TextWrapped("вЂў РџСЂРѕРјС‹С€Р»РµРЅРЅР°СЏ Р·РѕРЅР° СЃ РєРѕРЅРІРµР№РµСЂР°РјРё Рё СЃРєР»Р°РґР°РјРё.");
        }
    }
    ImGui::End();
}
