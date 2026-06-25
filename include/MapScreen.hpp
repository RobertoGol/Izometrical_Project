#pragma once

#include <SFML/Graphics.hpp>
#include "Types.hpp"
#include "GameState.hpp"
#include "Constants.hpp"
#include <string>
#include <cmath>
#include <algorithm>
#include <vector>

namespace bunker {

// ═══════════════════════════════════════════════════════
// Полноэкранная карта в стиле Fallout 76
// Открывается по M, закрывается по M / Escape
// Можно скроллить (WASD / мышь), зумить (колёсико)
// Маркеры: игрок, танк, враги, вышка, Pip-Pad, 
//          лут-контейнеры, эрозия, транспорт
// ═══════════════════════════════════════════════════════

struct MapMarker {
    Vector3D worldPos;
    std::string label;
    sf::Color color;
    enum class Shape { Circle, Diamond, Square, Triangle } shape = Shape::Circle;
    float size = 5.0f;
    bool  pulse = false;  // Мерцающий маркер
};

class MapScreen {
private:
    bool  m_IsOpen       = false;
    float m_Zoom         = 1.0f;
    float m_MinZoom      = 0.5f;
    float m_MaxZoom      = 3.0f;
    sf::Vector2f m_PanOffset = {0.0f, 0.0f};  // Смещение скролла
    float m_PulseTimer   = 0.0f;

    // Размеры карты на экране
    float m_MapScreenSize = 0.0f;
    float m_MapX          = 0.0f;
    float m_MapY          = 0.0f;
    float m_TileSize      = 0.0f;

    // Собранные маркеры текущего кадра
    std::vector<MapMarker> m_Markers;

    sf::Font m_Font;
    bool     m_FontLoaded = false;

public:
    MapScreen() = default;

    void loadFont(const sf::Font& font) {
        m_Font = font;
        m_FontLoaded = true;
    }

    bool isOpen() const { return m_IsOpen; }

    // ═══════════════════════════════════════════════
    // Переключение карты (M)
    // ═══════════════════════════════════════════════
    void toggle() {
        m_IsOpen = !m_IsOpen;
        if (m_IsOpen) {
            m_PanOffset = {0.0f, 0.0f};
            m_Zoom = 1.0f;
        }
    }

    void close() { m_IsOpen = false; }

    // ═══════════════════════════════════════════════
    // Обработка ввода на карте
    // ═══════════════════════════════════════════════
    void handleInput(const sf::Event& event, float dt) {
        if (!m_IsOpen) return;

        // Зум колёсиком
        if (event.type == sf::Event::MouseWheelScrolled) {
            m_Zoom += event.mouseWheelScroll.delta * 0.15f;
            m_Zoom = std::clamp(m_Zoom, m_MinZoom, m_MaxZoom);
        }

        // Закрытие по Escape
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape ||
                event.key.code == sf::Keyboard::M) {
                close();
            }
        }
    }

    // Скролл по зажатым WASD (вызывать каждый кадр)
    void updatePan(float dt) {
        if (!m_IsOpen) return;

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

    // ═══════════════════════════════════════════════
    // Рендер полноэкранной карты
    // ═══════════════════════════════════════════════
    void render(sf::RenderWindow& window, const GameState& gs) {
        if (!m_IsOpen) return;

        m_PulseTimer += 0.05f;
        if (m_PulseTimer > 6.28f) m_PulseTimer = 0.0f;

        float W = static_cast<float>(Config::SCREEN_WIDTH);
        float H = static_cast<float>(Config::SCREEN_HEIGHT);

        // ── Полупрозрачный фон ──
        sf::RectangleShape overlay({W, H});
        overlay.setFillColor(sf::Color(8, 12, 8, 220));
        window.draw(overlay);

        // ── Область карты (по центру экрана с отступами) ──
        float margin = 60.0f;
        m_MapScreenSize = std::min(W, H) - margin * 2.0f;
        m_MapX = (W - m_MapScreenSize) / 2.0f;
        m_MapY = (H - m_MapScreenSize) / 2.0f + 15.0f;
        m_TileSize = (m_MapScreenSize * m_Zoom) / static_cast<float>(Config::MAP_WIDTH);

        // Рамка карты
        sf::RectangleShape border({m_MapScreenSize, m_MapScreenSize});
        border.setPosition(m_MapX, m_MapY);
        border.setFillColor(sf::Color(15, 20, 15, 200));
        border.setOutlineThickness(2.0f);
        border.setOutlineColor(sf::Color(50, 180, 50, 150));
        window.draw(border);

        // ── Заголовок ──
        drawText(window, "WORLD MAP", W / 2 - 60, m_MapY - 35, 18, sf::Color(50, 220, 50));
        drawText(window, "[M] Close  [Scroll] Zoom  [WASD] Pan",
                 W / 2 - 140, H - 40, 11, sf::Color(100, 100, 100));

        // ── Clipping (рисуем только внутри рамки) ──
        // Используем scissor через View
        sf::View mapView;
        mapView.reset(sf::FloatRect(0, 0, W, H));
        window.setView(mapView);

        // ── Рендер тайлов ──
        renderTiles(window, gs);

        // ── Собираем и рендерим маркеры ──
        collectMarkers(gs);
        renderMarkers(window, gs);

        // ── Легенда ──
        renderLegend(window, W, H);
    }

private:
    // Мировые координаты → пиксели на экране карты
    sf::Vector2f worldToMap(float wx, float wy) const {
        float px = m_MapX + (wx * m_TileSize) + m_PanOffset.x;
        float py = m_MapY + (wy * m_TileSize) + m_PanOffset.y;
        return {px, py};
    }

    bool isInMapBounds(float px, float py) const {
        return px >= m_MapX && px <= m_MapX + m_MapScreenSize &&
               py >= m_MapY && py <= m_MapY + m_MapScreenSize;
    }

    // ── Тайлы карты ──
    void renderTiles(sf::RenderWindow& window, const GameState& gs) {
        for (int x = 0; x < Config::MAP_WIDTH; ++x) {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y) {
                sf::Vector2f pos = worldToMap(static_cast<float>(x), static_cast<float>(y));

                if (!isInMapBounds(pos.x, pos.y)) continue;

                sf::RectangleShape tile({m_TileSize - 1.0f, m_TileSize - 1.0f});
                tile.setPosition(pos);

                if (gs.sectorMap[x][y] == 1) {
                    // Стены
                    tile.setFillColor(sf::Color(70, 75, 85, 200));
                } else if (gs.etherErosionMap[x][y] > 5.0f) {
                    // Эрозия
                    int alpha = static_cast<int>(std::min(gs.etherErosionMap[x][y] * 2.0f, 180.0f));
                    tile.setFillColor(sf::Color(100, 30, 160, alpha));
                } else {
                    // Пол
                    tile.setFillColor(sf::Color(25, 30, 25, 150));
                }

                tile.setOutlineThickness(0.5f);
                tile.setOutlineColor(sf::Color(40, 50, 40, 80));
                window.draw(tile);
            }
        }
    }

    // ── Сбор маркеров ──
    void collectMarkers(const GameState& gs) {
        m_Markers.clear();

        // Вышка связи
        m_Markers.push_back({
            gs.towerPosition, "RELAY TOWER",
            sf::Color::White, MapMarker::Shape::Diamond, 6.0f, true
        });

        // Танк БТ-7274
        m_Markers.push_back({
            gs.titan.position, "BT-7274",
            sf::Color(230, 115, 25), MapMarker::Shape::Triangle, 6.0f, false
        });

        // Pip-Pad (если не подобран)
        if (!gs.bunkerProgression.hasFoundPipPad) {
            m_Markers.push_back({
                gs.bunkerProgression.pipPadSpawnPos, "PIP-PAD",
                sf::Color(255, 255, 50), MapMarker::Shape::Diamond, 5.0f, true
            });
        }

        // Лут-контейнеры (не вскрытые)
        for (const auto& c : gs.lootContainers) {
            if (!c.isOpened) {
                sf::Color lootColor = (c.type == LootContainerType::IronSafe)
                    ? sf::Color(200, 200, 255) : sf::Color(180, 140, 60);
                m_Markers.push_back({
                    c.position, "",
                    lootColor, MapMarker::Shape::Square, 3.5f, false
                });
            }
        }

        // Враги (красные точки, без подписи)
        for (const auto& e : gs.enemies) {
            if (!e.isAlive) continue;
            m_Markers.push_back({
                e.position, "",
                sf::Color(255, 50, 50, 180), MapMarker::Shape::Circle, 2.5f, false
            });
        }

        // Игрок (всегда последний — рисуется поверх всех)
        m_Markers.push_back({
            gs.playerPos, "YOU",
            sf::Color(50, 255, 50), MapMarker::Shape::Circle, 7.0f, true
        });
    }

    // ── Рендер маркеров ──
    void renderMarkers(sf::RenderWindow& window, const GameState& gs) {
        for (const auto& marker : m_Markers) {
            sf::Vector2f pos = worldToMap(marker.worldPos.x, marker.worldPos.y);
            if (!isInMapBounds(pos.x, pos.y)) continue;

            float size = marker.size;
            if (marker.pulse) {
                size += std::sin(m_PulseTimer * 3.0f) * 1.5f;
            }

            sf::Color col = marker.color;

            switch (marker.shape) {
            case MapMarker::Shape::Circle: {
                sf::CircleShape shape(size);
                shape.setOrigin(size, size);
                shape.setPosition(pos);
                shape.setFillColor(col);
                window.draw(shape);
                break;
            }
            case MapMarker::Shape::Diamond: {
                sf::CircleShape shape(size, 4);
                shape.setOrigin(size, size);
                shape.setPosition(pos);
                shape.setFillColor(col);
                shape.setRotation(45.0f);
                window.draw(shape);
                break;
            }
            case MapMarker::Shape::Square: {
                sf::RectangleShape shape({size * 2, size * 2});
                shape.setOrigin(size, size);
                shape.setPosition(pos);
                shape.setFillColor(col);
                window.draw(shape);
                break;
            }
            case MapMarker::Shape::Triangle: {
                sf::CircleShape shape(size, 3);
                shape.setOrigin(size, size);
                shape.setPosition(pos);
                shape.setFillColor(col);
                window.draw(shape);
                break;
            }
            }

            // Подпись
            if (!marker.label.empty() && m_FontLoaded) {
                drawText(window, marker.label, pos.x + size + 3, pos.y - 6, 10, col);
            }
        }
    }

    // ── Легенда (правый нижний угол) ──
    void renderLegend(sf::RenderWindow& window, float W, float H) {
        float lx = W - 200;
        float ly = H - 180;

        drawText(window, "LEGEND:", lx, ly, 12, sf::Color(50, 180, 50));
        ly += 18;

        struct LegendEntry { std::string label; sf::Color color; };
        std::vector<LegendEntry> entries = {
            {"You",          sf::Color(50, 255, 50)},
            {"BT-7274",      sf::Color(230, 115, 25)},
            {"Relay Tower",  sf::Color::White},
            {"Enemies",      sf::Color(255, 50, 50)},
            {"Loot",         sf::Color(180, 140, 60)},
            {"Ether Erosion", sf::Color(100, 30, 160)},
        };

        for (const auto& e : entries) {
            sf::RectangleShape dot({8, 8});
            dot.setPosition(lx, ly + 2);
            dot.setFillColor(e.color);
            window.draw(dot);
            drawText(window, e.label, lx + 14, ly, 10, sf::Color(160, 160, 160));
            ly += 16;
        }
    }

    void drawText(sf::RenderWindow& window, const std::string& str,
                  float x, float y, int size, sf::Color color) {
        if (!m_FontLoaded) return;
        sf::Text text;
        text.setFont(m_Font);
        text.setString(str);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setPosition(x, y);
        window.draw(text);
    }
};

}  // namespace bunker
