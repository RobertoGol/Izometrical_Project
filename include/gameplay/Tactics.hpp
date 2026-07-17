#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "entities/Collisions.hpp"
#include "core/Constants.hpp"
#include <cmath>
#include <algorithm>
#include <string>

namespace bunker
{

    class TacticsManager
    {
    private:
        PilotClass m_ActivePilotClass = PilotClass::Grapple;
        TitanClass m_ActiveTitanClass = TitanClass::Tone;
        bool m_InsideVehicle = false;

    public:
        StatModifiers currentStats;

        float tacticalCooldown = 0.0f;
        float tacticalActiveTimer = 0.0f;
        bool isTacticalActive = false;

        GrapplePhysics grapple;
        DeployableShield aWallShield;

        Vector3D pulseBladePos;
        float pulseBladeDuration = 0.0f;
        float pulseBladeRadius = 0.0f;
        bool isPulseBladeActive = false;
        bool isPhaseDimensionActive = false;

        TacticsManager() = default;

        void updateCooldowns(GameState &gs, float dt);
        void activateTactical(GameState &gs, const Vector3D &mouseWorld);
        void processGrapplePhysics(GameState &gs, float dt);
        void changePilotClass(PilotClass newClass);
        void changeTitanFirmware(TitanClass newClass);
        void enterVehicle();
        void exitVehicle();

        PilotClass getActivePilotClass() const { return m_ActivePilotClass; }
        TitanClass getActiveTitanClass() const { return m_ActiveTitanClass; }
        bool isInVehicle() const { return m_InsideVehicle; }
        std::string getActiveClassName() const;

    private:
        void updateActiveStats();
    };

} // namespace bunker