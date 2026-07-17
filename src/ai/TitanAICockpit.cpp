#include "ai/TitanAI.hpp"
#include "engine/Log.hpp"
#include "gameplay/BulletSystem.hpp"
#include "gameplay/DamageSystem.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace bunker
{

    void TitanAI::cycleCockpitFireMode()
    {
        if (m_IsGunnerMounted)
            return; // В кооп режиме роли фиксированы

        switch (m_FireMode)
        {
        case CockpitFireMode::CannonOnly:
            m_FireMode = CockpitFireMode::TeammateMGOnly;
            bunker::logInfo() << "[COCKPIT] Режим огня: Турельные пулемёты (Место тиммейта)" << std::endl;
            break;
        case CockpitFireMode::TeammateMGOnly:
            m_FireMode = CockpitFireMode::DuplexTwinSalvo;
            bunker::logInfo() << "[COCKPIT] Режим огня: ДУПЛЕКС (Синхронный залп Пушка + Пулемёты!)" << std::endl;
            break;
        case CockpitFireMode::DuplexTwinSalvo:
            m_FireMode = CockpitFireMode::CannonOnly;
            bunker::logInfo() << "[COCKPIT] Режим огня: Основная пушка" << std::endl;
            break;
        default:
            break;
        }
    }

    std::string TitanAI::getFireModeBanner() const
    {
        if (m_IsGunnerMounted)
            return "CO-OP SEAT SPLIT";
        switch (m_FireMode)
        {
        case CockpitFireMode::CannonOnly:
            return "CANNON ONLY";
        case CockpitFireMode::TeammateMGOnly:
            return "TURRET MG ONLY";
        case CockpitFireMode::DuplexTwinSalvo:
            return "DUPLEX [TWIN SALVO]";
        default:
            break;
        }
        return "UNKNOWN";
    }

    void TitanAI::fireFromCockpit(GameState& gs)
    {
        if (gs.titan.fireCooldown > 0.0f)
            return;
        if (m_VortexActive || m_IsReMapping)
            return;

        float mdx = gs.mouseWorldPos.x - gs.titan.position.x;
        float mdy = gs.mouseWorldPos.y - gs.titan.position.y;
        float mLenSq = mdx * mdx + mdy * mdy;
        if (mLenSq < 0.01f)
            return;

        float mLen = std::sqrt(mLenSq);
        Vector3D targetDir = {mdx / mLen, mdy / mLen, 0.0f};

        if (m_PilotStress > 30.0f)
        {
            float tremor = (m_PilotStress / 100.0f) * 0.15f;
            float spread = (static_cast<float>(rand() % 100) / 100.0f - 0.5f) * tremor;
            targetDir.x += spread;
            targetDir.y += spread;
        }

        if (gs.titan.systems.turretStatus < 50.0f)
        {
            float broken = (static_cast<float>(rand() % 100) / 100.0f - 0.5f) * 0.25f;
            targetDir.x += broken;
            targetDir.y += broken;
        }

        // ── Логика стрельбы по местам в кабине (Пакет 2 Target Mechanics) ──
        bool fireCannon = (m_FireMode == CockpitFireMode::CannonOnly || m_FireMode == CockpitFireMode::DuplexTwinSalvo);
        bool fireHMG =
            (m_FireMode == CockpitFireMode::TeammateMGOnly || m_FireMode == CockpitFireMode::DuplexTwinSalvo);

        if (m_IsGunnerMounted)
        {
            fireCannon = true;
            fireHMG = false;
        }

        if (fireCannon)
        {
            Bullet b;
            b.start = gs.titan.position;
            b.current = gs.titan.position;
            b.direction = targetDir;

            switch (m_ActiveLoadout)
            {
            case AncientLoadout::XO16_SolidKinetic:
                b.type = BulletType::Standard;
                b.speed = 36.0f;
                gs.bullets.push_back(b);
                gs.titan.fireCooldown = m_CoreOverdriveActive ? 0.03f : 0.08f;
                break;

            case AncientLoadout::Scorch_ThermiteMortar:
                b.type = BulletType::Standard;
                b.speed = 18.0f;
                gs.bullets.push_back(b);
                gs.titan.fireCooldown = 0.65f;
                break;

            case AncientLoadout::Ion_SplitLaser_Vacuum:
                b.type = BulletType::Standard;
                b.speed = 45.0f;
                gs.bullets.push_back(b);
                gs.titan.fireCooldown = 0.14f;
                break;
            default:
                break;
            }
        }

        if (fireHMG)
        {
            // Залп из спаренного турельного пулемёта тиммейта
            Bullet hmg;
            hmg.start = gs.titan.position;
            hmg.current = gs.titan.position;
            hmg.type = BulletType::Pellet;
            hmg.speed = 40.0f;
            Vector3D sideVec = {-targetDir.y, targetDir.x, 0.0f};
            hmg.start.x += sideVec.x * 0.15f;
            hmg.start.y += sideVec.y * 0.15f;
            hmg.current = hmg.start;
            hmg.direction = targetDir;
            gs.bullets.push_back(hmg);

            if (m_FireMode == CockpitFireMode::TeammateMGOnly)
            {
                gs.titan.fireCooldown = 0.05f;
            }
        }
    }

    void TitanAI::autoFire(GameState& gs)
    {
        if (gs.titan.isPiloted)
            return;
        if (gs.titan.fireCooldown > 0.0f)
            return;
        if (m_VortexActive || m_IsReMapping)
            return;
        if (gs.enemies.empty())
            return;

        for (auto& e : gs.enemies)
        {
            if (!e.isAlive)
                continue;

            float edx = e.position.x - gs.titan.position.x;
            float edy = e.position.y - gs.titan.position.y;
            float eDistSq = edx * edx + edy * edy;

            if (eDistSq <= 400.0f)
            {
                float eDist = std::sqrt(eDistSq);
                Vector3D targetDir = {edx / eDist, edy / eDist, 0.0f};

                Bullet b;
                b.start = gs.titan.position;
                b.current = gs.titan.position;
                b.direction = targetDir;
                b.type = BulletType::Standard;
                b.speed = 36.0f;
                gs.bullets.push_back(b);

                gs.titan.fireCooldown = 0.09f;
                break;
            }
        }
    }

    PilotInputControls TitanAI::filterAndStabilizeInputs(const PilotInputControls& rawInput,
                                                         const TerrainFrictionData& terrain)
    {

        PilotInputControls stabilized = rawInput;

        if (rawInput.handTremorHz > 5.0f)
        {
            stabilized.steeringWheelAngle = rawInput.steeringWheelAngle * 0.85f;
        }

        if (terrain.surfaceSlickness > 0.6f)
        {
            float balanceOffset = std::sin(rawInput.throttleLever) * terrain.surfaceSlickness * 15.0f;
            (void)balanceOffset;
        }

        return stabilized;
    }


} // namespace bunker
