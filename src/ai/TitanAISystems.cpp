#include "ai/TitanAI.hpp"
#include "engine/Log.hpp"
#include "gameplay/BulletSystem.hpp"
#include "gameplay/DamageSystem.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace bunker
{

    std::string TitanAI::getLoadoutName() const
    {
        switch (m_ActiveLoadout)
        {
        case AncientLoadout::XO16_SolidKinetic:
            return "[1] XO-16 AUTOCANNON";
        case AncientLoadout::Scorch_ThermiteMortar:
            return "[2] SCORCH MORTAR";
        case AncientLoadout::Ion_SplitLaser_Vacuum:
            return "[3] ION LASER";
        default:
            break;
        }
        return "UNKNOWN";
    }

    void TitanAI::updateBoiler(float dt)
    {
        if (m_CoreOverdriveActive)
        {
            m_BoilerSteamPressure = std::min(1200.0f, m_BoilerSteamPressure + 50.0f * dt);
            m_CoolantTemperature = std::min(980.0f, m_CoolantTemperature + 30.0f * dt);
        }
        else
        {
            m_BoilerSteamPressure += (350.0f - m_BoilerSteamPressure) * 0.5f * dt;
            m_CoolantTemperature += (180.0f - m_CoolantTemperature) * 0.3f * dt;
        }
    }

    void TitanAI::updatePilotStress(const GameState& gs, float dt)
    {
        float hpPercent = gs.titan.health / gs.titan.maxHealth;
        float targetStress = (1.0f - hpPercent) * 60.0f;

        if (gs.titan.systems.turretStatus < 50.0f)
            targetStress += 15.0f;
        if (gs.titan.systems.tracksCondition < 40.0f)
            targetStress += 20.0f;
        if (gs.titan.systems.sensorLink < 30.0f)
            targetStress += 10.0f;

        m_PilotStress += (targetStress - m_PilotStress) * 2.0f * dt;
        m_PilotStress = std::clamp(m_PilotStress, 0.0f, 100.0f);
    }

    void TitanAI::triggerReMap(GameState& gs, AncientLoadout newWeapon)
    {
        (void)gs;
        if (m_VortexActive || m_IsReMapping)
            return;
        if (m_ActiveLoadout == newWeapon)
            return;

        m_ActiveLoadout = newWeapon;
        m_IsReMapping = true;
        m_ReMapTimer = 0.4f;

        switch (newWeapon)
        {
        case AncientLoadout::XO16_SolidKinetic:
            m_CurrentCamProfile = {350.0f, 120.0f, 1.02f};
            break;
        case AncientLoadout::Scorch_ThermiteMortar:
            m_CurrentCamProfile = {600.0f, 450.0f, 1.85f};
            break;
        case AncientLoadout::Ion_SplitLaser_Vacuum:
            m_CurrentCamProfile = {100.0f, 10.0f, 1.0f};
            break;
        default:
            break;
        }

        bunker::logInfo() << "[BT-7274] Смена калибра → " << getLoadoutName() << std::endl;
    }

    void TitanAI::updateVortexShield(GameState& gs, bool isHoldingQ, float dt)
    {
        if (m_ActiveLoadout == AncientLoadout::Scorch_ThermiteMortar)
        {
            m_VortexActive = false;
            return;
        }

        if (isHoldingQ && m_VortexEnergy > 5.0f)
        {
            m_VortexActive = true;
            m_VortexEnergy = std::max(0.0f, m_VortexEnergy - 30.0f * dt);

            float shieldRadiusSq = 4.0f;
            for (auto& b : gs.bullets)
            {
                if (!b.isAlive)
                    continue;
                float bdx = b.current.x - gs.titan.position.x;
                float bdy = b.current.y - gs.titan.position.y;
                if ((bdx * bdx + bdy * bdy) <= shieldRadiusSq)
                {
                    b.isAlive = false;
                    m_CaughtBulletsCount++;
                }
            }
        }
        else
        {
            if (m_VortexActive && m_CaughtBulletsCount > 0)
            {
                float mdx = gs.mouseWorldPos.x - gs.titan.position.x;
                float mdy = gs.mouseWorldPos.y - gs.titan.position.y;
                float mLenSq = mdx * mdx + mdy * mdy;

                if (mLenSq > 0.01f)
                {
                    float mLen = std::sqrt(mLenSq);
                    Vector3D returnDir = {mdx / mLen, mdy / mLen, 0.0f};

                    for (int i = 0; i < m_CaughtBulletsCount; ++i)
                    {
                        Bullet rb;
                        rb.start = gs.titan.position;
                        rb.current = gs.titan.position;
                        rb.type = BulletType::Standard;
                        rb.speed = 28.0f;
                        float spread = (static_cast<float>(rand() % 100) / 100.0f - 0.5f) * 0.25f;
                        rb.direction = {returnDir.x + spread, returnDir.y + spread, 0.0f};
                        gs.bullets.push_back(rb);
                    }

                    bunker::logInfo() << "[BT-7274] Вортекс выброс: " << m_CaughtBulletsCount << " снарядов!"
                                      << std::endl;
                }
                m_CaughtBulletsCount = 0;
            }

            m_VortexActive = false;
            m_VortexEnergy = std::min(100.0f, m_VortexEnergy + 15.0f * dt);
        }
    }

    bool TitanAI::validateCoreOverdriveTrigger(const GameState& gs)
    {
        if (m_CoreChargePercent < 100.0f)
            return false;
        if (m_CoreOverdriveActive)
            return false;

        int nearbyEnemies = 0;
        for (const auto& e : gs.enemies)
        {
            if (!e.isAlive)
                continue;
            float dx = e.position.x - gs.titan.position.x;
            float dy = e.position.y - gs.titan.position.y;
            if ((dx * dx + dy * dy) <= 225.0f)
                nearbyEnemies++;
        }

        return nearbyEnemies >= 3 || gs.titan.health < gs.titan.maxHealth * 0.3f;
    }

    void TitanAI::executeCoreOverdrive()
    {
        if (m_CoreChargePercent < 100.0f)
            return;
        m_CoreOverdriveActive = true;
        m_BoilerSteamPressure = 1200.0f;
        m_CoolantTemperature = 980.0f;
        bunker::logInfo() << "[BT-7274] !! CORE OVERDRIVE АКТИВИРОВАН !! Давление: 1200 Бар!" << std::endl;
    }


} // namespace bunker
