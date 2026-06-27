#include "ai/TitanAI.hpp"
#include "gameplay/BulletSystem.hpp"
#include "gameplay/DamageSystem.hpp"
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <iostream>

namespace bunker
{

    TitanAI::TitanAI()
    {
        m_CurrentCamProfile = {350.0f, 120.0f, 1.02f};
    }

    bool TitanAI::tryMount(GameState &gs)
    {
        if (gs.playerMode == UnitMode::Titan)
            return false;

        float dx = gs.playerPos.x - gs.titan.position.x;
        float dy = gs.playerPos.y - gs.titan.position.y;
        float distSq = dx * dx + dy * dy;

        if (distSq > 4.0f)
        {
            std::cout << "[BT-7274] Слишком далеко для посадки." << std::endl;
            return false;
        }

        gs.playerMode = UnitMode::Titan;
        gs.titan.isPiloted = true;
        gs.playerPos = gs.titan.position;

        std::cout << "[BT-7274] Протокол 3: Защитить Пилота. Связь установлена." << std::endl;
        return true;
    }

    void TitanAI::dismount(GameState &gs)
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

        std::cout << "[BT-7274] Высадка. Переход в автономный режим." << std::endl;
    }

    bool TitanAI::tryMountGunner(GameState &gs)
    {
        if (m_IsGunnerMounted)
            return false;
        m_IsGunnerMounted = true;
        std::cout << "[CO-OP] Второй Пилот залез на броню Танка БТ-7274 и занял турель! Кооп-разделение ролей активировано." << std::endl;
        return true;
    }

    void TitanAI::dismountGunner(GameState &gs)
    {
        if (!m_IsGunnerMounted)
            return;
        m_IsGunnerMounted = false;
        std::cout << "[CO-OP] Второй Пилот спрыгнул с Танка. Водитель забирает управление пулемётами (Доступен ДУПЛЕКС)." << std::endl;
    }

    void TitanAI::update(GameState &gs, const InputSnapshot &input, float dt)
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
                std::cout << "[BT-7274] Калибр сменён. Орудие готово." << std::endl;
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

            bool holdingQ = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
            updateVortexShield(gs, holdingQ, dt);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
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

    void TitanAI::cycleCockpitFireMode()
    {
        if (m_IsGunnerMounted)
            return; // В кооп режиме роли фиксированы

        switch (m_FireMode)
        {
        case CockpitFireMode::CannonOnly:
            m_FireMode = CockpitFireMode::TeammateMGOnly;
            std::cout << "[COCKPIT] Режим огня: Турельные пулемёты (Место тиммейта)" << std::endl;
            break;
        case CockpitFireMode::TeammateMGOnly:
            m_FireMode = CockpitFireMode::DuplexTwinSalvo;
            std::cout << "[COCKPIT] Режим огня: ДУПЛЕКС (Синхронный залп Пушка + Пулемёты!)" << std::endl;
            break;
        case CockpitFireMode::DuplexTwinSalvo:
            m_FireMode = CockpitFireMode::CannonOnly;
            std::cout << "[COCKPIT] Режим огня: Основная пушка" << std::endl;
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
        }
        return "UNKNOWN";
    }

    void TitanAI::fireFromCockpit(GameState &gs)
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
        bool fireHMG = (m_FireMode == CockpitFireMode::TeammateMGOnly || m_FireMode == CockpitFireMode::DuplexTwinSalvo);

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

    void TitanAI::autoFire(GameState &gs)
    {
        if (gs.titan.isPiloted)
            return;
        if (gs.titan.fireCooldown > 0.0f)
            return;
        if (m_VortexActive || m_IsReMapping)
            return;
        if (gs.enemies.empty())
            return;

        for (auto &e : gs.enemies)
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

    PilotInputControls TitanAI::filterAndStabilizeInputs(
        const PilotInputControls &rawInput,
        const TerrainFrictionData &terrain)
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

    void TitanAI::updatePilotStress(const GameState &gs, float dt)
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

    void TitanAI::triggerReMap(GameState &gs, AncientLoadout newWeapon)
    {
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
        }

        std::cout << "[BT-7274] Смена калибра → " << getLoadoutName() << std::endl;
    }

    void TitanAI::updateVortexShield(GameState &gs, bool isHoldingQ, float dt)
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
            for (auto &b : gs.bullets)
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

                    std::cout << "[BT-7274] Вортекс выброс: " << m_CaughtBulletsCount << " снарядов!" << std::endl;
                }
                m_CaughtBulletsCount = 0;
            }

            m_VortexActive = false;
            m_VortexEnergy = std::min(100.0f, m_VortexEnergy + 15.0f * dt);
        }
    }

    bool TitanAI::validateCoreOverdriveTrigger(const GameState &gs)
    {
        if (m_CoreChargePercent < 100.0f)
            return false;
        if (m_CoreOverdriveActive)
            return false;

        int nearbyEnemies = 0;
        for (const auto &e : gs.enemies)
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
        std::cout << "[BT-7274] !! CORE OVERDRIVE АКТИВИРОВАН !! Давление: 1200 Бар!" << std::endl;
    }

    void TitanAI::updateAutonomousMode(GameState &gs, float dt)
    {
        float tdx = gs.playerPos.x - gs.titan.position.x;
        float tdy = gs.playerPos.y - gs.titan.position.y;
        float distSq = tdx * tdx + tdy * tdy;

        if (distSq < m_ExclusionRadiusSq)
        {
            float dist = std::sqrt(distSq);
            if (dist > 0.001f)
            {
                Vector3D push = {tdx / dist, tdy / dist, 0.0f};
                float nextX = gs.titan.position.x - push.x * 12.0f * dt;
                float nextY = gs.titan.position.y - push.y * 12.0f * dt;

                if (!Collisions::checkWorldCollision(gs, nextX, gs.titan.position.y, Config::TITAN_RADIUS))
                    gs.titan.position.x = nextX;
                if (!Collisions::checkWorldCollision(gs, gs.titan.position.x, nextY, Config::TITAN_RADIUS))
                    gs.titan.position.y = nextY;
            }
        }
        else
        {
            Vector3D anchor = calculateCombatAnchor(gs);
            float adx = anchor.x - gs.titan.position.x;
            float ady = anchor.y - gs.titan.position.y;
            float aLenSq = adx * adx + ady * ady;

            if (aLenSq > 1.0f)
            {
                float aLen = std::sqrt(aLenSq);
                float nextX = gs.titan.position.x + (adx / aLen) * gs.titan.speed * dt;
                float nextY = gs.titan.position.y + (ady / aLen) * gs.titan.speed * dt;

                if (!Collisions::checkWorldCollision(gs, nextX, gs.titan.position.y, Config::TITAN_RADIUS))
                    gs.titan.position.x = nextX;
                if (!Collisions::checkWorldCollision(gs, gs.titan.position.x, nextY, Config::TITAN_RADIUS))
                    gs.titan.position.y = nextY;
            }
        }
    }

    void TitanAI::updatePilotedMode(GameState &gs, const InputSnapshot &input, float dt)
    {
        float camDx = gs.mouseWorldPos.x - gs.titan.position.x;
        float camDy = gs.mouseWorldPos.y - gs.titan.position.y;
        float camLen = std::sqrt(camDx * camDx + camDy * camDy);

        Vector3D camForward = {0.707f, -0.707f, 0.0f};
        Vector3D camRight = {0.707f, 0.707f, 0.0f};

        if (camLen > 0.01f)
        {
            camForward.x = camDx / camLen;
            camForward.y = camDy / camLen;
            camRight.x = camForward.y;
            camRight.y = -camForward.x;
        }

        Vector3D moveInput = {0.0f, 0.0f, 0.0f};
        moveInput.x = camForward.x * input.moveForward + camRight.x * input.moveStrafe;
        moveInput.y = camForward.y * input.moveForward + camRight.y * input.moveStrafe;

        float inputLenSq = moveInput.x * moveInput.x + moveInput.y * moveInput.y;
        Vector3D targetVel = {0.0f, 0.0f, 0.0f};

        if (inputLenSq > 0.001f)
        {
            float len = std::sqrt(inputLenSq);
            float activeSpeed = m_VortexActive ? (gs.titan.speed * 0.4f) : gs.titan.speed;
            if (gs.titan.systems.tracksCondition < 40.0f)
                activeSpeed *= 0.3f;

            targetVel.x = (moveInput.x / len) * activeSpeed;
            targetVel.y = (moveInput.y / len) * activeSpeed;
        }

        float lerpAcc = (inputLenSq > 0.001f) ? m_TitanAcceleration : m_TitanDeceleration;
        m_Velocity.x += (targetVel.x - m_Velocity.x) * lerpAcc * dt;
        m_Velocity.y += (targetVel.y - m_Velocity.y) * lerpAcc * dt;

        float nextX = gs.titan.position.x + m_Velocity.x * dt;
        float nextY = gs.titan.position.y + m_Velocity.y * dt;

        if (!Collisions::checkWorldCollision(gs, nextX, gs.titan.position.y, Config::TITAN_RADIUS))
            gs.titan.position.x = nextX;
        if (!Collisions::checkWorldCollision(gs, gs.titan.position.x, nextY, Config::TITAN_RADIUS))
            gs.titan.position.y = nextY;

        gs.playerPos = gs.titan.position;
    }

    Vector3D TitanAI::calculateCombatAnchor(const GameState &gs)
    {
        if (gs.enemies.empty())
            return gs.playerPos;

        Vector3D center = {0.0f, 0.0f, 0.0f};
        int count = 0;

        for (const auto &e : gs.enemies)
        {
            if (!e.isAlive)
                continue;
            center.x += e.position.x;
            center.y += e.position.y;
            count++;
        }

        if (count == 0)
            return gs.playerPos;

        center.x /= count;
        center.y /= count;

        return {
            gs.playerPos.x + (center.x - gs.playerPos.x) * 0.40f,
            gs.playerPos.y + (center.y - gs.playerPos.y) * 0.40f,
            0.0f};
    }

} // namespace bunker
