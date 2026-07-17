#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "gameplay/DamageSystem.hpp"

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════
    // Термодинамика котла и обвесов БТ-7274 (Волна 5)
    // При перегреве > 250°C включает аварийный сброс пара с AOE-уроном и локом пушки
    // ═══════════════════════════════════════════════════════════════════════

    class ThermalLoadSystem
    {
    private:
        float m_CoolantTempC = 180.0f;
        float m_WeaponLockTimer = 0.0f;

    public:
        ThermalLoadSystem() = default;

        void updateThermodynamics(GameState &gs, float dt);
        void addHeatPulse(float heatAmount);

        bool isCannonLocked() const { return m_WeaponLockTimer > 0.0f; }
        float currentTemp() const { return m_CoolantTempC; }
    };

} // namespace bunker