#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include <cmath>
#include <algorithm>
#include <vector>

namespace bunker
{

    enum class StimulusType
    {
        Footstep,      // Тактические шаги Пилота или скольжение подкатом
        Gunshot,       // Выстрел из пушки / миномёта Скорча
        Explosion,     // Взрыв ракеты или аварийный сброс пара 250°C
        GlassBreak,    // Звон разлетающихся стеклянных панелей
        FlashlightBeam // Ослепляющий луч фонарика Пип-Боя
    };

    struct SensoryStimulus
    {
        Vector3D origin;
        StimulusType type = StimulusType::Footstep;
        float intensity = 12.0f; // Максимальная дистанция слуха в тайлах
        float maxAge = 2.0f;     // Время затухания акустического следа
        float currentAge = 0.0f;
    };

    struct PeripheralCone
    {
        Vector3D forwardDir = {1.0f, 0.0f, 0.0f};
        float fovCosHalfAngle = 0.5f; // cos(60 deg) -> Зрительный конус 120°

        bool isInsideFOV(const Vector3D &eyePos, const Vector3D &targetPos) const
        {
            float dx = targetPos.x - eyePos.x;
            float dy = targetPos.y - eyePos.y;
            float lenSq = dx * dx + dy * dy;
            if (lenSq <= 0.01f)
                return true;
            float invLen = 1.0f / std::sqrt(lenSq);
            float dot = (dx * invLen * forwardDir.x) + (dy * invLen * forwardDir.y);
            return dot >= fovCosHalfAngle;
        }
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // Полноценный Сенсорно-Акустический Движок Восприятия Убежища 17 (Omniverse v.126)
    // ═══════════════════════════════════════════════════════════════════════════════

    class PerceptionSystem
    {
    public:
        PerceptionSystem() = default;

        // ── Проверка прямой линии видимости (LOS) между точками ──
        static bool hasLineOfSight(const WorldGridState &grid, const Vector3D &from, const Vector3D &to);

        // ── Расчёт эффективного радиуса слуха/зрения с учётом тумана ──
        static float calculateEffectiveDetectRadius(float baseRadius, float visibilityModifier);

        // ── Продвинутые сенсорные и акустические службы РобКо ──
        static void emitSoundStimulus(const Vector3D &pos, StimulusType type, float intensity);
        static void updateStimuli(float dt);
        static float calculateAcousticMuffling(const WorldGridState &grid, const Vector3D &soundPos, const Vector3D &listenerPos);
        static bool evaluateSensoryAwareness(const GameState &gs, const Enemy &observer, PeripheralCone cone, float &outAwarenessDelta);
        static const std::vector<SensoryStimulus> &activeStimuli();
    };

} // namespace bunker
