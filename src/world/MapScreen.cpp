#include "world/MapScreen.hpp"
#include "core/IsoMath.hpp"
#include <cmath>
#include <algorithm>

namespace bunker
{

    void MapScreen::toggle()
    {
        m_IsOpen = !m_IsOpen;
        if (m_IsOpen)
        {
            m_PanOffset = {0.0f, 0.0f};
            m_Zoom = 1.0f;
        }
    }

    void MapScreen::handleInput(const sf::Event &event, float)
    {
        if (!m_IsOpen)
            return;

        if (event.type == sf::Event::MouseWheelScrolled)
        {
            m_Zoom += event.mouseWheelScroll.delta * 0.15f;
            m_Zoom = std::clamp(m_Zoom, m_MinZoom, m_MaxZoom);
        }

        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Escape ||
                event.key.code == sf::Keyboard::M)
            {
                close();
            }
        }
    }

    void MapScreen::updatePan(float dt)
    {
        if (!m_IsOpen)
            return;

        float panSpeed = 300.0f * dt / m_Zoom;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            m_PanOffset.y += panSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            m_PanOffset.y -= panSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            m_PanOffset.x += panSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            m_PanOffset.x -= panSpeed;
    }

    void MapScreen::render(sf::RenderWindow &window, const GameState &gs)
    {
        if (!m_IsOpen)
            return;

        m_PulseTimer += 0.05f;
        if (m_PulseTimer > 6.28f)
            m_PulseTimer = 0.0f;

        float W = static_cast<float>(Config::SCREEN_WIDTH);
        float H = static_cast<float>(Config::SCREEN_HEIGHT);

        sf::RectangleShape overlay({W, H});
        overlay.setFillColor(sf::Color(8, 12, 8, 220));
        window.draw(overlay);

        float margin = 60.0f;
        m_MapScreenSize = std::min(W, H) - margin * 2.0f;
        m_MapX = (W - m_MapScreenSize) / 2.0f;
        m_MapY = (H - m_MapScreenSize) / 2.0f + 15.0f;
        m_TileSize = (m_MapScreenSize * m_Zoom) / static_cast<float>(Config::MAP_WIDTH);

        sf::RectangleShape border({m_MapScreenSize, m_MapScreenSize});
        border.setPosition(m_MapX, m_MapY);
        border.setFillColor(sf::Color(15, 25, 15));
        border.setOutlineThickness(2.0f);
        border.setOutlineColor(sf::Color(50, 180, 50));
        window.draw(border);

        collectMarkers(gs);
        renderErosion(window, gs);
        renderGrid(window, gs);
        renderMarkers(window);

        drawText(window, "VAULT 17 — TACTICAL MAP", m_MapX, m_MapY - 25, 16, sf::Color(50, 220, 50));
        std::string posStr = "SEC: [" + std::to_string(static_cast<int>(gs.playerPos.x)) +
                             ", " + std::to_string(static_cast<int>(gs.playerPos.y)) + "]";
        drawText(window, posStr, m_MapX + m_MapScreenSize - 100, m_MapY - 22, 12, sf::Color(150, 200, 150));

        renderLegend(window, W, H);
    }

    void MapScreen::collectMarkers(const GameState &gs)
    {
        m_Markers.clear();

        m_Markers.push_back({gs.playerPos, "You", sf::Color(50, 255, 50),
                             MapMarker::Shape::Triangle, 7.0f, false});

        m_Markers.push_back({gs.titan.position, "BT-7274", sf::Color(230, 115, 25),
                             MapMarker::Shape::Diamond, 8.0f, gs.titan.health < 300.0f});

        m_Markers.push_back({gs.towerPosition, "Relay Tower", sf::Color::White,
                             MapMarker::Shape::Square, 6.0f, false});

        if (!gs.bunkerProgression.hasFoundPipPad)
        {
            m_Markers.push_back({gs.bunkerProgression.pipPadSpawnPos, "Pip-Pad",
                                 sf::Color(255, 255, 50), MapMarker::Shape::Diamond, 6.0f, true});
        }

        for (const auto &e : gs.enemies)
        {
            if (!e.isAlive)
                continue;
            m_Markers.push_back({e.position, "", sf::Color(255, 50, 50, 200),
                                 MapMarker::Shape::Circle, 4.0f, false});
        }

        for (const auto &c : gs.lootContainers)
        {
            if (c.isOpened)
                continue;
            m_Markers.push_back({c.position, "", sf::Color(180, 140, 60),
                                 MapMarker::Shape::Square, 4.0f, false});
        }
    }

    sf::Vector2f MapScreen::worldToMapPixel(const Vector3D &wPos) const
    {
        float normX = wPos.x / static_cast<float>(Config::MAP_WIDTH);
        float normY = wPos.y / static_cast<float>(Config::MAP_HEIGHT);

        float px = m_MapX + m_PanOffset.x + normX * m_MapScreenSize * m_Zoom;
        float py = m_MapY + m_PanOffset.y + normY * m_MapScreenSize * m_Zoom;
        return {px, py};
    }

    void MapScreen::renderGrid(sf::RenderWindow &window, const GameState &gs)
    {
        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                if (gs.sectorMap[x][y] == 1)
                {
                    sf::Vector2f p1 = worldToMapPixel({static_cast<float>(x), static_cast<float>(y), 0});
                    sf::Vector2f p2 = worldToMapPixel({static_cast<float>(x + 1), static_cast<float>(y + 1), 0});

                    if (p2.x >= m_MapX && p1.x <= m_MapX + m_MapScreenSize &&
                        p2.y >= m_MapY && p1.y <= m_MapY + m_MapScreenSize)
                    {
                        sf::RectangleShape block({p2.x - p1.x, p2.y - p1.y});
                        block.setPosition(std::max(p1.x, m_MapX), std::max(p1.y, m_MapY));
                        window.draw(block);
                    }
                }
            }
        }
    }

    void MapScreen::renderErosion(sf::RenderWindow &window, const GameState &gs)
    {
        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                float er = gs.etherErosionMap[x][y];
                if (er > 5.0f)
                {
                    sf::Vector2f p1 = worldToMapPixel({static_cast<float>(x), static_cast<float>(y), 0});
                    sf::Vector2f p2 = worldToMapPixel({static_cast<float>(x + 1), static_cast<float>(y + 1), 0});

                    sf::RectangleShape zone({p2.x - p1.x, p2.y - p1.y});
                    zone.setPosition(p1.x, p1.y);
                    zone.setFillColor(sf::Color(100, 30, 160, static_cast<sf::Uint8>(std::min(er * 2.0f, 160.0f))));
                    window.draw(zone);
                }
            }
        }
    }

    void MapScreen::renderMarkers(sf::RenderWindow &window)
    {
        for (const auto &marker : m_Markers)
        {
            sf::Vector2f pos = worldToMapPixel(marker.worldPos);

            if (pos.x < m_MapX || pos.x > m_MapX + m_MapScreenSize ||
                pos.y < m_MapY || pos.y > m_MapY + m_MapScreenSize)
                continue;

            sf::Color col = marker.color;
            if (marker.pulse)
            {
                float alpha = (std::sin(m_PulseTimer * 3.0f) + 1.0f) * 0.5f;
                col.a = static_cast<sf::Uint8>(100 + alpha * 155);
            }

            float size = marker.size * std::sqrt(m_Zoom);

            switch (marker.shape)
            {
            case MapMarker::Shape::Circle:
            {
                sf::CircleShape shape(size);
                shape.setOrigin(size, size);
                shape.setPosition(pos);
                shape.setFillColor(col);
                window.draw(shape);
                break;
            }
            case MapMarker::Shape::Diamond:
            {
                sf::CircleShape shape(size, 4);
                shape.setOrigin(size, size);
                shape.setPosition(pos);
                shape.setFillColor(col);
                window.draw(shape);
                break;
            }
            case MapMarker::Shape::Square:
            {
                sf::RectangleShape shape({size * 2, size * 2});
                shape.setOrigin(size, size);
                shape.setPosition(pos);
                shape.setFillColor(col);
                window.draw(shape);
                break;
            }
            case MapMarker::Shape::Triangle:
            {
                sf::CircleShape shape(size, 3);
                shape.setOrigin(size, size);
                shape.setPosition(pos);
                shape.setFillColor(col);
                window.draw(shape);
                break;
            }
            }

            if (!marker.label.empty() && m_FontLoaded)
            {
                drawText(window, marker.label, pos.x + size + 3, pos.y - 6, 10, col);
            }
        }
    }

    void MapScreen::renderLegend(sf::RenderWindow &window, float W, float H)
    {
        float lx = W - 200;
        float ly = H - 180;

        drawText(window, "LEGEND:", lx, ly, 12, sf::Color(50, 180, 50));
        ly += 18;

        struct LegendEntry
        {
            std::string label;
            sf::Color color;
        };
        std::vector<LegendEntry> entries = {
            {"You", sf::Color(50, 255, 50)},
            {"BT-7274", sf::Color(230, 115, 25)},
            {"Relay Tower", sf::Color::White},
            {"Enemies", sf::Color(255, 50, 50)},
            {"Loot", sf::Color(180, 140, 60)},
            {"Ether Erosion", sf::Color(100, 30, 160)},
        };

        for (const auto &e : entries)
        {
            sf::RectangleShape dot({8, 8});
            dot.setPosition(lx, ly + 2);
            dot.setFillColor(e.color);
            window.draw(dot);
            drawText(window, e.label, lx + 14, ly, 10, sf::Color(160, 160, 160));
            ly += 16;
        }
    }

    void MapScreen::drawText(sf::RenderWindow &window, const std::string &str,
                             float x, float y, int size, sf::Color color)
    {
        if (!m_FontLoaded)
            return;
        sf::Text text;
        text.setFont(m_Font);
        text.setString(str);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setPosition(x, y);
        window.draw(text);
    }

} // namespace bunker