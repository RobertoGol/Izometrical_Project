#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "entities/Collisions.hpp"
#include "core/Constants.hpp"
#include "engine/InputManager.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

namespace bunker
{

    enum class AncientLoadout
    {
        XO16_SolidKinetic,
        Scorch_ThermiteMortar,
        Ion_SplitLaser_Vacuum
    };

    // Режимы стрельбы по позициям кабины БТ-7274 (Пакет 2 Target Architecture)
    enum class CockpitFireMode
    {
        CannonOnly,     // Основное орудие (Место 1 — Пилот)
        TeammateMGOnly, // Турельные пулемёты (Место 2 — Тиммейт или соло-игрок)
        DuplexTwinSalvo // ДУПЛЕКС — одновременный залп из пушки и пулемётов!
    };

    struct BallisticCamProfile
    {
        float hydraulicPressureBar = 350.0f;
        float recoilDecayMs = 120.0f;
        float mechanicalLeadFactor = 1.02f;
    };

    struct PilotInputControls
    {
        float throttleLever = 0.0f;
        float steeringWheelAngle = 0.0f;
        bool triggerPressed = false;
        float handTremorHz = 0.0f;
    };

    struct TerrainFrictionData
    {
        float surfaceSlickness = 0.0f;
        float structuralIntegrity = 1.0f;
    };

    class TitanAI
    {
    private:
        const float m_TitanAcceleration = 4.0f;
        const float m_TitanDeceleration = 6.0f;
        const float m_ExclusionRadiusSq = 36.0f;

        Vector3D m_Velocity = {0.0f, 0.0f, 0.0f};

        AncientLoadout m_ActiveLoadout = AncientLoadout::XO16_SolidKinetic;
        BallisticCamProfile m_CurrentCamProfile;

        bool m_VortexActive = false;
        float m_VortexEnergy = 100.0f;
        int m_CaughtBulletsCount = 0;

        float m_CoreChargePercent = 0.0f;
        float m_BoilerSteamPressure = 350.0f;
        float m_CoolantTemperature = 180.0f;
        bool m_CoreOverdriveActive = false;

        float m_ReMapTimer = 0.0f;
        bool m_IsReMapping = false;
        float m_PilotStress = 0.0f;

        // Пакет 2: Места в кабине и режим Дуплекс
        CockpitFireMode m_FireMode = CockpitFireMode::DuplexTwinSalvo;
        bool m_IsGunnerMounted = false;

    public:
        TitanAI();

        bool tryMount(GameState &gs);
        void dismount(GameState &gs);
        void update(GameState &gs, const InputSnapshot &input, float dt);
        void fireFromCockpit(GameState &gs);
        void autoFire(GameState &gs);
        PilotInputControls filterAndStabilizeInputs(const PilotInputControls &rawInput, const TerrainFrictionData &terrain);

        void cycleCockpitFireMode();
        bool tryMountGunner(GameState &gs);
        void dismountGunner(GameState &gs);
        bool isGunnerMounted() const { return m_IsGunnerMounted; }
        CockpitFireMode getCockpitFireMode() const { return m_FireMode; }
        std::string getFireModeBanner() const;

        bool isVortexActive() const { return m_VortexActive; }
        float getVortexEnergy() const { return m_VortexEnergy; }
        bool isCoreActive() const { return m_CoreOverdriveActive; }
        float getCoreCharge() const { return m_CoreChargePercent; }
        AncientLoadout getLoadout() const { return m_ActiveLoadout; }
        bool isReMapping() const { return m_IsReMapping; }
        float getBoilerPressure() const { return m_BoilerSteamPressure; }
        float getCoolantTemp() const { return m_CoolantTemperature; }
        float getPilotStress() const { return m_PilotStress; }
        std::string getLoadoutName() const;

    private:
        void updateBoiler(float dt);
        void updatePilotStress(const GameState &gs, float dt);
        void triggerReMap(GameState &gs, AncientLoadout newWeapon);
        void updateVortexShield(GameState &gs, bool isHoldingQ, float dt);
        bool validateCoreOverdriveTrigger(const GameState &gs);
        void executeCoreOverdrive();
        void updateAutonomousMode(GameState &gs, float dt);
        void updatePilotedMode(GameState &gs, const InputSnapshot &input, float dt);
        Vector3D calculateCombatAnchor(const GameState &gs);
    };

} // namespace bunker