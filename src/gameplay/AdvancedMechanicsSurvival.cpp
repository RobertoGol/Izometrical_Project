#include "gameplay/AdvancedMechanics.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 2) RADIO / TAPE SYSTEM
    // ═══════════════════════════════════════════════════════════════════════════════

    RadioTapeSystem::RadioTapeSystem()
    {
        m_Tapes.push_back({"CRYO_00", "Cryo Locker 00", "Если ты слышишь это — Убежище уже проснулось не по протоколу.",
                           false, false});
        m_Tapes.push_back({"GARAGE_BT", "Garage: BT-7274",
                           "Котёл танка холодный. Нужен ручной запуск и синхронизация пилота.", false, false});
        m_Tapes.push_back({"SURFACE_17", "Surface weather",
                           "На поверхности эфирный туман режет дальность сенсоров почти вдвое.", false, false});
        m_Tapes.push_back({"LAB_60S", "RobCo Lab 60s Archive (v.97.A.V)",
                           "Диагностика главного фрейма Убежища 17. Синхронизация аудиобанка Vorbis завершена.", false,
                           false});
        m_Tapes.push_back({"MECH_WAR", "ROY/RAY Swarm War Log",
                           "ROY и RAY — 100% механические самореплицирующиеся дроны. Органика и гнезда исключены.",
                           false, false});
        m_Tapes.push_back({"PORT_16", "Space Port Sector 16",
                           "Шлюз бастиона 16 запечатан. Давление гидравлики БТ-7274 стабильно: 350 Бар.", false,
                           false});
        m_Tapes.push_back({"BATTLE_215", "Orbital Battle Log 215",
                           "Температура хладагента танка 180°C. При пороге в 250°C сработает аварийный сброс пара.",
                           false, false});

        m_Radio.push_back({4.0f, "V17", "...приём... Башня молчит. Найдите Pip-Pad и синхронизируйтесь.", false});
        m_Radio.push_back(
            {18.0f, "BT", "Пилот, корпус повреждён. Ангарные ремкомплекты восстановят подсистемы.", false});
        m_Radio.push_back({45.0f, "LAN", "Локальная сеть Lanline доступна: чат, заказы, доставки, отряд.", false});
    }

    void RadioTapeSystem::update(float dt)
    {
        m_Time += dt;
        m_LastSubtitle.clear();
        for (auto& msg : m_Radio)
        {
            if (!msg.fired && m_Time >= msg.atTime)
            {
                msg.fired = true;
                m_LastSubtitle = "[" + msg.channel + "] " + msg.text;
                m_Log.push_back(m_LastSubtitle);
                if (m_Log.size() > 12)
                {
                    m_Log.pop_front();
                }
                break;
            }
        }
    }

    void RadioTapeSystem::discoverTape(const std::string& id)
    {
        for (auto& tape : m_Tapes)
        {
            if (tape.id == id)
            {
                tape.found = true;
                m_LastSubtitle = "[TAPE FOUND] " + tape.title;
                m_Log.push_back(m_LastSubtitle);
                return;
            }
        }
    }

    std::string RadioTapeSystem::playNextUnplayed()
    {
        for (auto& tape : m_Tapes)
        {
            if (tape.found && !tape.played)
            {
                tape.played = true;
                m_LastSubtitle = "[TAPE] " + tape.title + ": " + tape.transcript;
                m_Log.push_back(m_LastSubtitle);
                return m_LastSubtitle;
            }
        }
        return "[TAPE] Нет новых записей.";
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 3) RATIONS / HEAL / RELOAD
    // ═══════════════════════════════════════════════════════════════════════════════

    void SurvivalSystem::update(GameState& gs, PlayerInventory& inv, float dt)
    {
        (void)inv;
        updateStress(gs, dt);
        updateBuffs(gs, dt);
        updateReload(gs, dt);
        updateSecondWindAndSoulLine(gs);
    }

    bool SurvivalSystem::useStim(GameState& gs, PlayerInventory& inv)
    {
        if (!inv.hasItem(ITEM_STIM) || gs.playerHealth >= gs.playerMaxHealth)
        {
            return false;
        }
        inv.removeItem(ITEM_STIM, 1);
        gs.playerHealth = advClamp(gs.playerHealth + 45.0f, 0.0f, gs.playerMaxHealth);
        m_Stress = advClamp(m_Stress - 10.0f, 0.0f, 100.0f);
        return true;
    }

    bool SurvivalSystem::eatRation(GameState& gs, PlayerInventory& inv, RationKind kind)
    {
        unsigned int id = ITEM_RATION_PROTEIN;
        float amount = 1.0f;
        float duration = 45.0f;
        switch (kind)
        {
        case RationKind::Protein:
            id = ITEM_RATION_PROTEIN;
            amount = 12.0f;
            duration = 60.0f;
            break;
        case RationKind::Stamina:
            id = ITEM_RATION_STAMINA;
            amount = 0.35f;
            duration = 50.0f;
            break;
        case RationKind::Focus:
            id = ITEM_RATION_FOCUS;
            amount = 0.20f;
            duration = 35.0f;
            break;
        case RationKind::AntiErosion:
            id = ITEM_RATION_ANTI_EROSION;
            amount = 25.0f;
            duration = 40.0f;
            break;
        default:
            break;
        }
        if (!inv.hasItem(id))
        {
            return false;
        }
        inv.removeItem(id, 1);
        m_Buffs.push_back({kind, duration, amount});
        if (kind == RationKind::Protein)
        {
            gs.playerMaxHealth += amount;
            gs.playerHealth = advClamp(gs.playerHealth + amount, 0.0f, gs.playerMaxHealth);
        }
        return true;
    }

    void SurvivalSystem::startReload(PlayerInventory& inv)
    {
        if (m_Weapon.isReloading || m_Weapon.magazine >= m_Weapon.magazineMax)
        {
            return;
        }
        if (m_Weapon.reserveAmmo <= 0 && !inv.hasItem(ITEM_AMMO_556))
        {
            return;
        }
        if (m_Weapon.reserveAmmo <= 0 && inv.removeItem(ITEM_AMMO_556, 1))
        {
            m_Weapon.reserveAmmo += 30;
        }
        m_Weapon.isReloading = true;
        m_Weapon.reloadTimer = reloadDuration();
    }

    bool SurvivalSystem::consumeRound()
    {
        if (m_Weapon.isReloading || m_Weapon.magazine <= 0)
        {
            return false;
        }
        --m_Weapon.magazine;
        return true;
    }

    float SurvivalSystem::speedMultiplier() const
    {
        float result = 1.0f;
        for (const auto& b : m_Buffs)
        {
            if (b.kind == RationKind::Stamina)
            {
                result += b.amount;
            }
        }
        if (m_Stress > 70.0f)
        {
            result -= 0.15f;
        }
        return advClamp(result, 0.55f, 1.75f);
    }

    float SurvivalSystem::aimPenalty() const
    {
        float penalty = (m_Stress > 35.0f) ? (m_Stress - 35.0f) / 650.0f : 0.0f;
        for (const auto& b : m_Buffs)
        {
            if (b.kind == RationKind::Focus)
            {
                penalty -= b.amount;
            }
        }
        return advClamp(penalty, 0.0f, 0.35f);
    }

    float SurvivalSystem::reloadDuration() const
    {
        float d = 1.45f;
        for (const auto& b : m_Buffs)
        {
            if (b.kind == RationKind::Focus)
            {
                d *= 0.82f;
            }
        }
        return d;
    }

    void SurvivalSystem::updateStress(GameState& gs, float dt)
    {
        bool nearEnemy = false;
        for (const auto& e : gs.enemies)
        {
            if (e.isAlive && advDistSq(e.position, gs.playerPos) < 5.5f * 5.5f)
            {
                nearEnemy = true;
                break;
            }
        }
        if (nearEnemy || gs.playerHealth < gs.playerMaxHealth * 0.35f)
        {
            addStress((nearEnemy ? 4.0f : 2.0f) * dt);
        }
        else
        {
            calm(2.5f * dt);
        }
    }

    void SurvivalSystem::updateBuffs(GameState& gs, float dt)
    {
        for (auto& b : m_Buffs)
        {
            b.timeLeft -= dt;
            if (b.kind == RationKind::AntiErosion)
            {
                gs.playerErosionLevel = advClamp(gs.playerErosionLevel - b.amount * 0.03f * dt, 0.0f, 100.0f);
            }
        }
        m_Buffs.erase(
            std::remove_if(m_Buffs.begin(), m_Buffs.end(), [](const ActiveBuff& b) { return b.timeLeft <= 0.0f; }),
            m_Buffs.end());
    }

    void SurvivalSystem::updateReload(GameState& gs, float dt)
    {
        gs.fireCooldown = std::max(0.0f, gs.fireCooldown - dt);
        if (!m_Weapon.isReloading)
        {
            return;
        }
        m_Weapon.reloadTimer -= dt;
        if (m_Weapon.reloadTimer <= 0.0f)
        {
            const int need = m_Weapon.magazineMax - m_Weapon.magazine;
            const int take = std::min(need, m_Weapon.reserveAmmo);
            m_Weapon.magazine += take;
            m_Weapon.reserveAmmo -= take;
            m_Weapon.isReloading = false;
        }
    }

    void SurvivalSystem::updateSecondWindAndSoulLine(GameState& gs)
    {
        if (gs.playerHealth <= 0.0f && !m_SoulLineUsed)
        {
            m_SoulLineUsed = true;
            gs.playerHealth = 1.0f;
            m_Stress = 100.0f;
            bunker::logInfo() << "[SURVIVAL] !! SOUL LINE !! Срыв смерти! Нить души удержала Пилота в Убежище 17!"
                              << std::endl;
            return;
        }
        if (gs.playerHealth > 0.0f && gs.playerHealth < gs.playerMaxHealth * 0.16f && !m_SecondWindUsed &&
            m_Stress >= 55.0f)
        {
            m_SecondWindUsed = true;
            gs.playerHealth = std::min(gs.playerMaxHealth, gs.playerHealth + 28.0f);
            m_Stress = advClamp(m_Stress - 25.0f, 0.0f, 100.0f);
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 4) TANK UTILITIES
    // ═══════════════════════════════════════════════════════════════════════════════

    void TankUtilitySystem::update(GameState& gs, float dt)
    {
        for (auto& s : m_Sparks)
        {
            s.pos += s.vel * dt;
            s.ttl -= dt;
        }
        m_Sparks.erase(
            std::remove_if(m_Sparks.begin(), m_Sparks.end(), [](const MuzzleSpark& s) { return s.ttl <= 0.0f; }),
            m_Sparks.end());

        m_Runtime.utilityCooldown = std::max(0.0f, m_Runtime.utilityCooldown - dt);
        m_Runtime.cannonThermalLoad = advClamp(m_Runtime.cannonThermalLoad - 18.0f * dt, 0.0f, 100.0f);
        if (m_Runtime.cannonThermalLoad < 62.0f)
        {
            m_Runtime.overheated = false;
        }

        if (gs.titan.isPiloted)
        {
            gs.playerPos = gs.titan.position;
            gs.playerMode = UnitMode::Titan;
        }

        if (m_Runtime.towing)
        {
            const Vector3D toTank = gs.titan.position - m_Runtime.towAnchor;
            m_Runtime.towAnchor += toTank * advClamp(dt * 1.8f, 0.0f, 1.0f);
            if (toTank.lengthSq() < 0.35f)
            {
                m_Runtime.towing = false;
            }
        }
    }

    void TankUtilitySystem::nextUtility()
    {
        switch (m_Runtime.utility)
        {
        case TankUtilityMode::BucketRig:
            m_Runtime.utility = TankUtilityMode::RamShield;
            break;
        case TankUtilityMode::RamShield:
            m_Runtime.utility = TankUtilityMode::TowCoupler;
            break;
        default:
            m_Runtime.utility = TankUtilityMode::BucketRig;
            break;
        }
    }

    void TankUtilitySystem::swapSeat()
    {
        m_Runtime.seat = (m_Runtime.seat == TankSeat::Driver) ? TankSeat::Gunner : TankSeat::Driver;
    }

    bool TankUtilitySystem::useUtility(GameState& gs)
    {
        if (m_Runtime.utilityCooldown > 0.0f)
        {
            return false;
        }
        switch (m_Runtime.utility)
        {
        case TankUtilityMode::BucketRig:
            carveFront(gs, 2, 35);
            m_Runtime.utilityCooldown = 1.0f;
            return true;
        case TankUtilityMode::RamShield:
            gs.titan.systems.tracksCondition = advClamp(gs.titan.systems.tracksCondition + 12.0f, 0.0f, 100.0f);
            gs.titan.health = advClamp(gs.titan.health + 10.0f, 0.0f, gs.titan.maxHealth);
            for (auto& e : gs.enemies)
            {
                if (e.isAlive && advDistSq(e.position, gs.titan.position) < 2.0f * 2.0f)
                {
                    if (DamageSystem::applyEnemyDamage(gs, e, 60.0f, DamageType::Kinetic))
                    {
                        gs.score += 20;
                    }
                }
            }
            m_Runtime.utilityCooldown = 3.0f;
            return true;
        case TankUtilityMode::TowCoupler:
            m_Runtime.towing = true;
            m_Runtime.towAnchor = gs.mouseWorldPos;
            m_Runtime.utilityCooldown = 2.0f;
            return true;
        default:
            break;
        }
        return false;
    }

    bool TankUtilitySystem::registerCannonShot(GameState& gs, float heat)
    {
        if (m_Runtime.overheated)
        {
            return false;
        }
        m_Runtime.cannonThermalLoad = advClamp(m_Runtime.cannonThermalLoad + heat, 0.0f, 100.0f);
        if (m_Runtime.cannonThermalLoad >= 100.0f)
        {
            m_Runtime.overheated = true;
            gs.titan.systems.turretStatus = advClamp(gs.titan.systems.turretStatus - 4.0f, 0.0f, 100.0f);
        }

        // Спавн 4 лёгких процедурных искр дульного пламени (Оптимизировано O(1)):
        for (int i = 0; i < 4; ++i)
        {
            float ang = static_cast<float>(i) * 1.57f;
            Vector3D vel{std::cos(ang) * 6.5f, std::sin(ang) * 6.5f, 2.0f};
            m_Sparks.push_back({gs.titan.position, vel, 0.12f});
        }
        return true;
    }

    bool TankUtilitySystem::repairInHangar(GameState& gs, PlayerInventory& inv)
    {
        if (!isInsideHangar(gs.titan.position))
        {
            return false;
        }
        if (!inv.removeItem(ITEM_REPAIR_KIT, 1))
        {
            return false;
        }
        gs.titan.health = advClamp(gs.titan.health + 70.0f, 0.0f, gs.titan.maxHealth);
        gs.titan.systems.coreEnergy = advClamp(gs.titan.systems.coreEnergy + 25.0f, 0.0f, 100.0f);
        gs.titan.systems.sensorLink = advClamp(gs.titan.systems.sensorLink + 35.0f, 0.0f, 100.0f);
        gs.titan.systems.tracksCondition = advClamp(gs.titan.systems.tracksCondition + 35.0f, 0.0f, 100.0f);
        gs.titan.systems.turretStatus = advClamp(gs.titan.systems.turretStatus + 35.0f, 0.0f, 100.0f);
        return true;
    }

    void TankUtilitySystem::carveFront(GameState& gs, int radius, int damage)
    {
        const int cx = static_cast<int>(std::round(gs.titan.position.x));
        const int cy = static_cast<int>(std::round(gs.titan.position.y));
        for (int x = cx - radius; x <= cx + radius; ++x)
        {
            for (int y = cy - radius; y <= cy + radius; ++y)
            {
                if (x < 0 || y < 0 || x >= Config::MAP_WIDTH || y >= Config::MAP_HEIGHT)
                {
                    continue;
                }
                if (gs.sectorMap[x][y] == 1)
                {
                    gs.wallDurability[x][y] -= damage;
                    if (gs.wallDurability[x][y] <= 0)
                    {
                        gs.sectorMap[x][y] = 0;
                    }
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════════

} // namespace bunker
