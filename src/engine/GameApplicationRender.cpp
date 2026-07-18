#include "engine/GameApplication.hpp"
#include "content/MaterialCatalog.hpp"
#include "core/Constants.hpp"
#include "core/IsoMath.hpp"
#include "render/GameRenderer.hpp"

#include <glad/glad.h>
#include <imgui.h>
#include <imgui-SFML.h>
#include <algorithm>
#include <cstdint>
#include <string>

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
        m_Renderer3D.applyWeather(m_Advanced.weather.state());
        m_Renderer3D.renderScene(m_Registry, m_Camera);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        m_Window.pushGLStates();
        m_Window.setView(m_Window.getDefaultView());

        GameRenderer::renderFloor(m_Window, m_GameState, m_TimeShift);
        GameRenderer::renderEntities(m_Window, m_GameState, m_TimeShift, m_HostileAI);
        GameRenderer::renderAdvancedWorld(m_Window, m_Advanced);
        renderInteractionHighlight();

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

        const float fadeAlpha = m_DoorTransition.fadeAlpha();
        if (fadeAlpha > 0.001f)
        {
            sf::RectangleShape fade({static_cast<float>(m_Window.getSize().x), static_cast<float>(m_Window.getSize().y)});
            fade.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(std::min(255.0f, fadeAlpha * 255.0f))));
            m_Window.draw(fade);
        }

        m_Window.popGLStates();
        m_Window.display();
    }

    void GameApplication::renderInteractionHighlight()
    {
        const auto& target = m_InteractionManager.highlightedTarget();
        if (!target || !target->hasLineOfSight)
        {
            return;
        }

        sf::Vector2f screenPos = IsoMath::worldToScreen(target->position);
        screenPos.y -= 18.0f;

        sf::CircleShape ring(13.0f, 32);
        ring.setOrigin({13.0f, 13.0f});
        ring.setPosition(screenPos);
        ring.setFillColor(sf::Color(0, 0, 0, 0));
        ring.setOutlineThickness(2.0f);
        ring.setOutlineColor(sf::Color(120, 255, 170, 220));
        m_Window.draw(ring);

        sf::CircleShape pointer(4.0f, 3);
        pointer.setOrigin({4.0f, 4.0f});
        pointer.setPosition({screenPos.x, screenPos.y + 19.0f});
        pointer.setFillColor(sf::Color(120, 255, 170, 230));
        m_Window.draw(pointer);

        if (!m_FontLoaded)
        {
            return;
        }

        std::string label = target->label.empty() ? "Interact" : target->label;
        if (label.size() > 28)
        {
            label = label.substr(0, 25) + "...";
        }

        sf::Text text(m_GlobalFont, "[E] " + label, 13);
        text.setFillColor(sf::Color(220, 255, 225));
        text.setOutlineThickness(1.0f);
        text.setOutlineColor(sf::Color(10, 25, 15, 220));

        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y});
        text.setPosition({screenPos.x, screenPos.y - 16.0f});
        m_Window.draw(text);
    }

    void GameApplication::renderMaterialDebugWindow()
    {
        if (!m_ShowMaterialDebug)
        {
            return;
        }

        ImGui::Begin("Material Catalog", &m_ShowMaterialDebug);
        int overlay = static_cast<int>(m_Renderer3D.debugOverlay());
        ImGui::Text("3D Debug Overlay");
        if (ImGui::RadioButton("None", overlay == static_cast<int>(RendererDebugOverlay::None)))
        {
            m_Renderer3D.setDebugOverlay(RendererDebugOverlay::None);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Normals", overlay == static_cast<int>(RendererDebugOverlay::Normals)))
        {
            m_Renderer3D.setDebugOverlay(RendererDebugOverlay::Normals);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Material IDs", overlay == static_cast<int>(RendererDebugOverlay::MaterialIds)))
        {
            m_Renderer3D.setDebugOverlay(RendererDebugOverlay::MaterialIds);
        }

        const glm::vec3 cameraPos = m_Camera.getPosition();
        const glm::vec3 cameraFront = m_Camera.getFront();
        constexpr float nearPlane = 0.1f;
        constexpr float farPlane = 2000.0f;
        const float aspectRatio = static_cast<float>(Config::SCREEN_WIDTH) / static_cast<float>(Config::SCREEN_HEIGHT);
        ImGui::Separator();
        ImGui::Text("Camera Frustum");
        ImGui::Text("pos %.2f %.2f %.2f | front %.2f %.2f %.2f",
                    cameraPos.x,
                    cameraPos.y,
                    cameraPos.z,
                    cameraFront.x,
                    cameraFront.y,
                    cameraFront.z);
        ImGui::Text("fov 45.00 | aspect %.3f | near %.2f | far %.1f", aspectRatio, nearPlane, farPlane);
        ImGui::Separator();

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
