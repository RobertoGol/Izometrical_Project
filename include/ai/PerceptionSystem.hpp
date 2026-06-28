#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include <cmath>
#include <algorithm>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════
    // Выделенная сенсорная подсистема восприятия ИИ (Фаза 3 / Волна 3)
    // Реализует проверку линии видимости (Line of Sight) по тайлам Убежища 17
    // ═══════════════════════════════════════════════════════════════════════

    class PerceptionSystem
    {
    public:
        PerceptionSystem() = default;

        // ── Проверка прямой линии видимости (LOS) между точками ──
        static bool hasLineOfSight(const WorldGridState &grid, const Vector3D &from, const Vector3D &to);

        // ── Расчёт эффективного радиуса слуха/зрения с учётом тумана ──
        static float calculateEffectiveDetectRadius(float baseRadius, float visibilityModifier);
    };

} // namespace bunker