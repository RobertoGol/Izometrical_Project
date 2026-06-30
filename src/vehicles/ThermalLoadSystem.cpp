#include "vehicles/ThermalLoadSystem.hpp"
#include <algorithm>
#include <iostream>

namespace bunker
{

    void ThermalLoadSystem::updateThermodynamics(GameState &gs, float dt)
    {
        if (m_WeaponLockTimer > 0.0f)
            m_WeaponLockTimer -= dt;

        // Охлаждение радиатора БТ-7274 в покое
        m_CoolantTempC = std::max(180.0f, m_CoolantTempC - 12.0f * dt);

        // Порог аварийного сброса пара (согласно утвержденному решению Консилиума)
        if (m_CoolantTempC >= 250.0f && m_WeaponLockTimer <= 0.0f)
        {
            m_CoolantTempC = 190.0f;  // Сброс пара охлаждает котел
            m_WeaponLockTimer = 2.5f; // Блокировка основного орудия на 2.5 секунды
            DamageSystem::applyRadiusDamage(gs, gs.titan.position, 3.5f, 45.0f, DamageType::Thermal);
            std::cout << "[THERMAL] !! АВАРИЙНЫЙ СБРОС ПАРА !! Перегрев > 250°C! Орудие заблокировано на 2.5с!" << std::endl;
        }
    }

    void ThermalLoadSystem::addHeatPulse(float heatAmount)
    {
        m_CoolantTempC += heatAmount;
    }

} // namespace bunker