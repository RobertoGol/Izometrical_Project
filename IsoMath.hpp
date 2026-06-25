#ifndef ISOMATH_HPP
#define ISOMATH_HPP

#include <SFML/Graphics.hpp>
#include "Constants.hpp"

class IsoMath {
public:
    // Мир -> Экран
    static sf::Vector2f worldToScreen(float x, float y) {
        float sx = (x - y) * (Config::TILE_W / 2.0f);
        float sy = (x + y) * (Config::TILE_H / 2.0f);
        return { sx + Config::SCREEN_WIDTH / 2.0f, sy + Config::SCREEN_HEIGHT / 4.0f };
    }

    // Экран -> Мир (Для прицеливания)
    static sf::Vector2f screenToWorld(float sx, float sy) {
        float nx = sx - Config::SCREEN_WIDTH / 2.0f;
        float ny = sy - Config::SCREEN_HEIGHT / 4.0f;
        float x = (nx / (Config::TILE_W / 2.0f) + ny / (Config::TILE_H / 2.0f)) / 2.0f;
        float y = (ny / (Config::TILE_H / 2.0f) - nx / (Config::TILE_W / 2.0f)) / 2.0f;
        return { x, y };
    }
};

#endif