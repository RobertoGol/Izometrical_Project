#include "ai/TitanAI.hpp"
#include "engine/Log.hpp"
#include "gameplay/BulletSystem.hpp"
#include "gameplay/DamageSystem.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace bunker
{

    TitanAI::TitanAI()
    {
        m_CurrentCamProfile = {350.0f, 120.0f, 1.02f};
    }

    bool TitanAI::tryMount(GameState& gs)
    {
        if (gs.playerMode == UnitMode::Titan)
            return false;

        float dx = gs.playerPos.x - gs.titan.position.x;
        float dy = gs.playerPos.y - gs.titan.position.y;
        float distSq = dx * dx + dy * dy;

        if (distSq > 4.0f)
        {
            bunker::logInfo() << "[BT-7274] Слишком далеко для посадки." << std::endl;
            return false;
        }

        gs.playerMode = UnitMode::Titan;
        gs.titan.isPiloted = true;
        gs.playerPos = gs.titan.position;

        bunker::logInfo() << "[BT-7274] Протокол 3: Защитить Пилота. Связь установлена." << std::endl;
        return true;
    }

    void TitanAI::dismount(GameState& gs)
    {
        if (gs.playerMode != UnitMode::Titan)
            return;

        gs.playerMode = UnitMode::Scout;
        gs.titan.isPiloted = false;

        gs.playerPos.x = gs.titan.position.x + 1.0f;
        gs.playerPos.y = gs.titan.position.y;

        if (Collisions::checkWorldCollision(gs, gs.playerPos.x, gs.playerPos.y, Config::PLAYER_RADIUS))
        {
            gs.playerPos.x = gs.titan.position.x - 1.0f;
            gs.playerPos.y = gs.titan.position.y;
        }
        if (Collisions::checkWorldCollision(gs, gs.playerPos.x, gs.playerPos.y, Config::PLAYER_RADIUS))
        {
            gs.playerPos.x = gs.titan.position.x;
            gs.playerPos.y = gs.titan.position.y + 1.0f;
        }
        if (Collisions::checkWorldCollision(gs, gs.playerPos.x, gs.playerPos.y, Config::PLAYER_RADIUS))
        {
            gs.playerPos.x = gs.titan.position.x;
            gs.playerPos.y = gs.titan.position.y - 1.0f;
        }

        m_VortexActive = false;
        m_CaughtBulletsCount = 0;

        bunker::logInfo() << "[BT-7274] Высадка. Переход в автономный режим." << std::endl;
    }

    bool TitanAI::tryMountGunner(GameState& gs)
    {
        (void)gs;
        if (m_IsGunnerMounted)
            return false;
        m_IsGunnerMounted = true;
        bunker::logInfo()
            << "[CO-OP] Второй Пилот залез на броню Танка БТ-7274 и занял турель! Кооп-разделение ролей активировано."
            << std::endl;
        return true;
    }

    void TitanAI::dismountGunner(GameState& gs)
    {
        (void)gs;
        if (!m_IsGunnerMounted)
            return;
        m_IsGunnerMounted = false;
        bunker::logInfo()
            << "[CO-OP] Второй Пилот спрыгнул с Танка. Водитель забирает управление пулемётами (Доступен ДУПЛЕКС)."
            << std::endl;
    }

    void TitanAI::update(GameState& gs, const InputSnapshot& input, float dt)
    {
        updateBoiler(dt);

        if (!m_CoreOverdriveActive)
        {
            m_CoreChargePercent = std::min(100.0f, m_CoreChargePercent + 4.5f * dt);
        }
        else
        {
            m_CoreChargePercent = std::max(0.0f, m_CoreChargePercent - 18.0f * dt);
            if (m_CoreChargePercent <= 0.0f)
            {
                m_CoreOverdriveActive = false;
                m_BoilerSteamPressure = 350.0f;
                m_CoolantTemperature = 180.0f;
            }
        }

        if (gs.titan.fireCooldown > 0.0f)
            gs.titan.fireCooldown -= dt;

        if (m_IsReMapping)
        {
            m_ReMapTimer -= dt;
            if (m_ReMapTimer <= 0.0f)
            {
                m_IsReMapping = false;
                bunker::logInfo() << "[BT-7274] Калибр сменён. Орудие готово." << std::endl;
            }
            return;
        }

        if (gs.playerMode == UnitMode::Titan)
        {
            if (input.selectClass[0])
                triggerReMap(gs, AncientLoadout::XO16_SolidKinetic);
            if (input.selectClass[1])
                triggerReMap(gs, AncientLoadout::Scorch_ThermiteMortar);
            if (input.selectClass[2])
                triggerReMap(gs, AncientLoadout::Ion_SplitLaser_Vacuum);

            if (input.switchWeapon)
            {
                cycleCockpitFireMode();
            }

            bool holdingQ = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q);
            updateVortexShield(gs, holdingQ, dt);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
            {
                if (validateCoreOverdriveTrigger(gs))
                {
                    executeCoreOverdrive();
                }
            }
        }

        updatePilotStress(gs, dt);

        if (!gs.titan.isPiloted)
        {
            updateAutonomousMode(gs, dt);
        }
        else
        {
            updatePilotedMode(gs, input, dt);
        }
    }


} // namespace bunker
