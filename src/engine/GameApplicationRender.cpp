#include "engine/GameApplication.hpp"
#include "content/MaterialCatalog.hpp"
#include "render/GameRenderer.hpp"

#include <glad/glad.h>
#include <imgui.h>
#include <imgui-SFML.h>

namespace bunker
{
    void GameApplication::renderMapFrame(float dt)
    {
        m_MapScreen.updatePan(dt);

        m_Window.clear(sf::Color(20, 20, 22));
        m_Window.setView(m_Window.getDefaultView());
        m_MapScreen.render(m_Window, m_GameState);
        m_Window.display();
    }

    void GameApplication::renderGameplayFrame()
    {
        glClearColor(0.06f, 0.07f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        m_Renderer3D.renderScene(m_Registry, m_Camera);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        m_Window.pushGLStates();
        m_Window.setView(m_Window.getDefaultView());

        GameRenderer::renderFloor(m_Window, m_GameState, m_TimeShift);
        GameRenderer::renderEntities(m_Window, m_GameState, m_TimeShift, m_HostileAI);
        GameRenderer::renderAdvancedWorld(m_Window, m_Advanced);

        m_Hud.render(m_Window, m_GameState, m_PlayerController, m_Tactics, m_TitanAI, m_VehicleManager, m_Inventory);

        if (m_TimeShift.isInitialized())
        {
            m_TimeShift.renderHUD(m_Window, m_FontLoaded ? &m_GlobalFont : nullptr);
            m_TimeShift.renderTransitionEffect(m_Window);
        }

        GameRenderer::renderAdvancedHUD(m_Window, m_Advanced, m_FontLoaded ? &m_GlobalFont : nullptr);
        m_Audio.renderSubtitlesHUD(m_Window, m_FontLoaded ? &m_GlobalFont : nullptr);
        m_PipPad.renderTablet(m_Window, m_GameState, m_Inventory, m_Advanced, m_FontLoaded ? &m_GlobalFont : nullptr);

        if (m_TerminalUI.isOpen())
        {
            m_TerminalUI.render(m_Window, m_GameState);
        }

        if (m_ImGuiInitialized)
        {
            renderMaterialDebugWindow();
            ImGui::SFML::Render(m_Window);
        }

        m_Window.popGLStates();
        m_Window.display();
    }

    void GameApplication::renderMaterialDebugWindow()
    {
        if (!m_ShowMaterialDebug)
        {
            return;
        }

        ImGui::Begin("Material Catalog", &m_ShowMaterialDebug);
        for (const auto& material : getMaterialCatalog())
        {
            ImGui::Text("ID %u | %s | %s | #%06X | rough %.2f | metal %.2f",
                        material.id,
                        material.name,
                        materialCategoryName(material.category),
                        material.hex,
                        material.roughness,
                        material.metallic);
        }
        ImGui::End();
    }

} // namespace bunker
