#pragma once

#include <SFML/Graphics.hpp>
#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "core/Constants.hpp"
#include <string>
#include <vector>

namespace bunker
{

    struct MapMarker
    {
        Vector3D worldPos;
        std::string label;
        sf::Color color;
        enum class Shape
        {
            Circle,
            Diamond,
            Square,
            Triangle
        } shape = Shape::Circle;
        float size = 5.0f;
        bool pulse = false;
    };

    class MapScreen
    {
    private:
        bool m_IsOpen = false;
        float m_Zoom = 1.0f;
        float m_MinZoom = 0.5f;
        float m_MaxZoom = 3.0f;
        sf::Vector2f m_PanOffset = {0.0f, 0.0f};
        float m_PulseTimer = 0.0f;

        float m_MapScreenSize = 0.0f;
        float m_MapX = 0.0f;
        float m_MapY = 0.0f;
        float m_TileSize = 0.0f;

        std::vector<MapMarker> m_Markers;
        sf::Font m_Font;
        bool m_FontLoaded = false;

    public:
        MapScreen() = default;

        void loadFont(const sf::Font &font)
        {
            m_Font = font;
            m_FontLoaded = true;
        }

        bool isOpen() const { return m_IsOpen; }
        void toggle();
        void close() { m_IsOpen = false; }
        void handleInput(const sf::Event &event, float dt);
        void updatePan(float dt);
        void render(sf::RenderWindow &window, const GameState &gs);

    private:
        void collectMarkers(const GameState &gs);
        sf::Vector2f worldToMapPixel(const Vector3D &wPos) const;
        void renderGrid(sf::RenderWindow &window, const GameState &gs);
        void renderErosion(sf::RenderWindow &window, const GameState &gs);
        void renderMarkers(sf::RenderWindow &window);
        void renderLegend(sf::RenderWindow &window, float W, float H);
        void drawText(sf::RenderWindow &window, const std::string &str,
                      float x, float y, int size, sf::Color color);
    };

} // namespace bunker
