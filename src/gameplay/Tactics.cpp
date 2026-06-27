#include "gameplay/Tactics.hpp"

namespace bunker {

void TacticsManager::updateCooldowns(GameState& gs, float dt) {
    static bool firstFrame = true;
    if (firstFrame) {
        updateActiveStats();
        firstFrame = false;
    }

    if (tacticalCooldown > 0.0f)
        tacticalCooldown -= dt;

    if (isTacticalActive) {
        tacticalActiveTimer -= dt;
        if (tacticalActiveTimer <= 0.0f) {
            isTacticalActive = false;
            aWallShield.isDeployed = false;
            isPulseBladeActive = false;
            isPhaseDimensionActive = false;
            updateActiveStats();
        }
    }

    if (isTacticalActive && m_ActivePilotClass == PilotClass::Stim && !m_InsideVehicle) {
        gs.playerHealth = std::min(gs.playerMaxHealth,
                                   gs.playerHealth + Config::STIM_HEAL_RATE * dt);
    }
}

void TacticsManager::activateTactical(GameState& gs, const Vector3D& mouseWorld) {
    if (!gs.bunkerProgression.hasFoundPipPad || m_InsideVehicle ||
        tacticalCooldown > 0.0f || isTacticalActive)
        return;

    isTacticalActive = true;

    switch (m_ActivePilotClass) {
    case PilotClass::Grapple: {
        grapple.isAttached = true;
        grapple.hookPoint = mouseWorld;

        float dx = grapple.hookPoint.x - gs.playerPos.x;
        float dy = grapple.hookPoint.y - gs.playerPos.y;
        float distSq = dx * dx + dy * dy;

        if (distSq > Config::GRAPPLE_MAX_RANGE_SQ) {
            grapple.isAttached = false;
            isTacticalActive = false;
            return;
        }
        grapple.length = std::sqrt(distSq);
        tacticalActiveTimer = Config::GRAPPLE_DURATION;
        tacticalCooldown    = Config::GRAPPLE_COOLDOWN;
        break;
    }
    case PilotClass::Stim:
        tacticalActiveTimer = Config::STIM_DURATION;
        tacticalCooldown    = Config::STIM_COOLDOWN;
        break;

    case PilotClass::PhaseShift:
        isPhaseDimensionActive = true;
        tacticalActiveTimer = Config::PHASE_DURATION;
        tacticalCooldown    = Config::PHASE_COOLDOWN;
        break;

    case PilotClass::Cloak:
        tacticalActiveTimer = Config::CLOAK_DURATION;
        tacticalCooldown    = Config::CLOAK_COOLDOWN;
        break;

    case PilotClass::AWall:
        aWallShield.isDeployed = true;
        aWallShield.position = gs.playerPos;
        aWallShield.health = Config::AWALL_HP;
        tacticalActiveTimer = Config::AWALL_DURATION;
        tacticalCooldown    = Config::AWALL_COOLDOWN;
        break;

    case PilotClass::PulseBlade:
        isPulseBladeActive = true;
        pulseBladePos = mouseWorld;
        pulseBladeDuration = 4.0f;
        pulseBladeRadius   = 6.0f;
        tacticalActiveTimer = 4.0f;
        tacticalCooldown    = 12.0f;
        break;

    default:
        isTacticalActive = false;
        break;
    }
    updateActiveStats();
}

void TacticsManager::processGrapplePhysics(GameState& gs, float dt) {
    if (!grapple.isAttached || m_ActivePilotClass != PilotClass::Grapple || m_InsideVehicle)
        return;

    float tdx = grapple.hookPoint.x - gs.playerPos.x;
    float tdy = grapple.hookPoint.y - gs.playerPos.y;
    float currentDist = std::sqrt(tdx * tdx + tdy * tdy);

    if (currentDist > 0.4f && tacticalActiveTimer > 0.05f) {
        grapple.velocity.x = (tdx / currentDist) * Config::GRAPPLE_PULL_FORCE;
        grapple.velocity.y = (tdy / currentDist) * Config::GRAPPLE_PULL_FORCE;

        float progress = 1.0f - (currentDist / grapple.length);
        gs.playerPos.z = std::sin(progress * 3.14159265f) * 1.8f;

        float nextX = gs.playerPos.x + grapple.velocity.x * dt;
        float nextY = gs.playerPos.y + grapple.velocity.y * dt;

        if (!Collisions::checkWorldCollision(gs, nextX, gs.playerPos.y, Config::PLAYER_RADIUS))
            gs.playerPos.x = nextX;
        if (!Collisions::checkWorldCollision(gs, gs.playerPos.x, nextY, Config::PLAYER_RADIUS))
            gs.playerPos.y = nextY;
    } else {
        grapple.isAttached = false;
        isTacticalActive = false;
        gs.playerPos.z = 0.0f;
    }
}

void TacticsManager::changePilotClass(PilotClass newClass) {
    if (m_InsideVehicle) return;
    m_ActivePilotClass = newClass;
    isTacticalActive = false;
    tacticalActiveTimer = 0.0f;
    grapple.isAttached = false;
    aWallShield.isDeployed = false;
    isPulseBladeActive = false;
    isPhaseDimensionActive = false;
    updateActiveStats();
}

void TacticsManager::changeTitanFirmware(TitanClass newClass) {
    m_ActiveTitanClass = newClass;
    if (m_InsideVehicle) updateActiveStats();
}

void TacticsManager::enterVehicle() {
    m_InsideVehicle = true;
    isTacticalActive = false;
    grapple.isAttached = false;
    aWallShield.isDeployed = false;
    isPhaseDimensionActive = false;
    updateActiveStats();
}

void TacticsManager::exitVehicle() {
    m_InsideVehicle = false;
    updateActiveStats();
}

std::string TacticsManager::getActiveClassName() const {
    switch (m_ActivePilotClass) {
        case PilotClass::Grapple:    return "GRAPPLE";
        case PilotClass::Cloak:      return "CLOAK";
        case PilotClass::Stim:       return "STIM";
        case PilotClass::PhaseShift: return "PHASE SHIFT";
        case PilotClass::HoloPilot:  return "HOLO PILOT";
        case PilotClass::AWall:      return "A-WALL";
        case PilotClass::PulseBlade: return "PULSE BLADE";
        default: return "UNKNOWN";
    }
}

void TacticsManager::updateActiveStats() {
    if (!m_InsideVehicle) {
        currentStats.isVehicleMode = false;
        currentStats.damageMultiplier = 1.0f;
        currentStats.erosionResistance = 0.0f;

        switch (m_ActivePilotClass) {
        case PilotClass::Grapple:
            currentStats.maxHealth = 100.0f;
            currentStats.moveSpeed = 5.8f;
            currentStats.weaponLabel = "PILOT: GRAPPLE GEAR";
            break;
        case PilotClass::Stim:
            currentStats.maxHealth = 100.0f;
            currentStats.moveSpeed = isTacticalActive ? 6.8f : 5.5f;
            currentStats.weaponLabel = "STIM CARBINE";
            break;
        case PilotClass::PhaseShift:
            currentStats.maxHealth = 100.0f;
            currentStats.moveSpeed = 5.5f;
            currentStats.weaponLabel = "PHASE CARBINE";
            break;
        case PilotClass::Cloak:
            currentStats.maxHealth = 100.0f;
            currentStats.moveSpeed = 5.2f;
            currentStats.weaponLabel = "CLOAK CARBINE";
            break;
        case PilotClass::AWall:
            currentStats.maxHealth = 120.0f;
            currentStats.moveSpeed = 4.8f;
            currentStats.weaponLabel = "A-WALL LMG";
            currentStats.damageMultiplier = isTacticalActive ? 1.35f : 1.0f;
            break;
        case PilotClass::PulseBlade:
            currentStats.maxHealth = 100.0f;
            currentStats.moveSpeed = 5.5f;
            currentStats.weaponLabel = "PULSE CARBINE";
            break;
        case PilotClass::HoloPilot:
            currentStats.maxHealth = 100.0f;
            currentStats.moveSpeed = 5.5f;
            currentStats.weaponLabel = "HOLO CARBINE";
            break;
        }
    } else {
        currentStats.isVehicleMode = true;
        currentStats.moveSpeed = Config::TITAN_SPEED;
        currentStats.maxHealth = Config::TITAN_HP;
        currentStats.weaponLabel = "TITAN WEAPONS";
    }
}

}  // namespace bunker
