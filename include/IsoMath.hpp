#pragma once

#include <SFML/Graphics.hpp>
#include "Constants.hpp"
#include "Types.hpp"

namespace bunker {

class IsoMath {
public:
    // ── Мир → Экран ──
    // Преобразует мировые координаты (x, y) в пиксельные экранные
    static sf::Vector2f worldToScreen(float x, float y) {
        float sx = (x - y) * (Config::TILE_W / 2.0f);
        float sy = (x + y) * (Config::TILE_H / 2.0f);
        return {sx, sy};
    }

    // Перегрузка для Vector3D (игнорирует z для 2D-проекции)
    static sf::Vector2f worldToScreen(const Vector3D& pos) {
        return worldToScreen(pos.x, pos.y);
    }

    // Перегрузка для Vector2D
    static sf::Vector2f worldToScreen(const Vector2D& pos) {
        return worldToScreen(pos.x, pos.y);
    }

    // ── Экран → Мир ──
    // Обратная проекция: из пиксельных экранных координат в мировые
    static sf::Vector2f screenToWorld(float sx, float sy) {
        float x = (sx / (Config::TILE_W / 2.0f) + sy / (Config::TILE_H / 2.0f)) / 2.0f;
        float y = (sy / (Config::TILE_H / 2.0f) - sx / (Config::TILE_W / 2.0f)) / 2.0f;
        return {x, y};
    }

    // Экран → Мир с возвратом Vector3D
    static Vector3D screenToWorld3D(float sx, float sy) {
        sf::Vector2f w = screenToWorld(sx, sy);
        return {w.x, w.y, 0.0f};
    }

    // ── Расстояние в мировых координатах ──
    static float worldDistance(const Vector3D& a, const Vector3D& b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    static float worldDistanceSq(const Vector3D& a, const Vector3D& b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return dx * dx + dy * dy;
    }

    // ── Угол между двумя точками в градусах (0–360) ──
    static float angleDeg(const Vector3D& from, const Vector3D& to) {
        float dx = to.x - from.x;
        float dy = to.y - from.y;
        float angle = std::atan2(dy, dx) * 180.0f / 3.14159265f;
        if (angle < 0.0f) angle += 360.0f;
        return angle;
    }

    // ── Lerp (линейная интерполяция) ──
    static float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    static Vector3D lerp(const Vector3D& a, const Vector3D& b, float t) {
        return {
            lerp(a.x, b.x, t),
            lerp(a.y, b.y, t),
            lerp(a.z, b.z, t)
        };
    }

    // ── Clamp ──
    static float clamp(float val, float lo, float hi) {
        if (val < lo) return lo;
        if (val > hi) return hi;
        return val;
    }

    static int clamp(int val, int lo, int hi) {
        if (val < lo) return lo;
        if (val > hi) return hi;
        return val;
    }
};

}  // namespace bunker
