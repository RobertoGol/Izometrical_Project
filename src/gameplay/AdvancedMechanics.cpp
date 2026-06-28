#include "gameplay/AdvancedMechanics.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 2) RADIO / TAPE SYSTEM
    // ═══════════════════════════════════════════════════════════════════════════════

    RadioTapeSystem::RadioTapeSystem()
    {
        m_Tapes.push_back({"CRYO_00", "Cryo Locker 00", "Если ты слышишь это — Убежище уже проснулось не по протоколу.", false, false});
        m_Tapes.push_back({"GARAGE_BT", "Garage: BT-7274", "Котёл танка холодный. Нужен ручной запуск и синхронизация пилота.", false, false});
        m_Tapes.push_back({"SURFACE_17", "Surface weather", "На поверхности эфирный туман режет дальность сенсоров почти вдвое.", false, false});

        m_Radio.push_back({4.0f, "V17", "...приём... Башня молчит. Найдите Pip-Pad и синхронизируйтесь.", false});
        m_Radio.push_back({18.0f, "BT", "Пилот, корпус повреждён. Ангарные ремкомплекты восстановят подсистемы.", false});
        m_Radio.push_back({45.0f, "LAN", "Локальная сеть Lanline доступна: чат, заказы, доставки, отряд.", false});
    }

    void RadioTapeSystem::update(float dt)
    {
        m_Time += dt;
        m_LastSubtitle.clear();
        for (auto &msg : m_Radio)
        {
            if (!msg.fired && m_Time >= msg.atTime)
            {
                msg.fired = true;
                m_LastSubtitle = "[" + msg.channel + "] " + msg.text;
                m_Log.push_back(m_LastSubtitle);
                if (m_Log.size() > 12)
                    m_Log.pop_front();
                break;
            }
        }
    }

    void RadioTapeSystem::discoverTape(const std::string &id)
    {
        for (auto &tape : m_Tapes)
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
        for (auto &tape : m_Tapes)
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

    void SurvivalSystem::update(GameState &gs, PlayerInventory &inv, float dt)
    {
        (void)inv;
        updateStress(gs, dt);
        updateBuffs(gs, dt);
        updateReload(gs, dt);
        updateSecondWindAndSoulLine(gs);
    }

    bool SurvivalSystem::useStim(GameState &gs, PlayerInventory &inv)
    {
        if (!inv.hasItem(ITEM_STIM) || gs.playerHealth >= gs.playerMaxHealth)
            return false;
        inv.removeItem(ITEM_STIM, 1);
        gs.playerHealth = advClamp(gs.playerHealth + 45.0f, 0.0f, gs.playerMaxHealth);
        m_Stress = advClamp(m_Stress - 10.0f, 0.0f, 100.0f);
        return true;
    }

    bool SurvivalSystem::eatRation(GameState &gs, PlayerInventory &inv, RationKind kind)
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
        }
        if (!inv.hasItem(id))
            return false;
        inv.removeItem(id, 1);
        m_Buffs.push_back({kind, duration, amount});
        if (kind == RationKind::Protein)
        {
            gs.playerMaxHealth += amount;
            gs.playerHealth = advClamp(gs.playerHealth + amount, 0.0f, gs.playerMaxHealth);
        }
        return true;
    }

    void SurvivalSystem::startReload(PlayerInventory &inv)
    {
        if (m_Weapon.isReloading || m_Weapon.magazine >= m_Weapon.magazineMax)
            return;
        if (m_Weapon.reserveAmmo <= 0 && !inv.hasItem(ITEM_AMMO_556))
            return;
        if (m_Weapon.reserveAmmo <= 0 && inv.removeItem(ITEM_AMMO_556, 1))
            m_Weapon.reserveAmmo += 30;
        m_Weapon.isReloading = true;
        m_Weapon.reloadTimer = reloadDuration();
    }

    bool SurvivalSystem::consumeRound()
    {
        if (m_Weapon.isReloading || m_Weapon.magazine <= 0)
            return false;
        --m_Weapon.magazine;
        return true;
    }

    float SurvivalSystem::speedMultiplier() const
    {
        float result = 1.0f;
        for (const auto &b : m_Buffs)
            if (b.kind == RationKind::Stamina)
                result += b.amount;
        if (m_Stress > 70.0f)
            result -= 0.15f;
        return advClamp(result, 0.55f, 1.75f);
    }

    float SurvivalSystem::aimPenalty() const
    {
        float penalty = (m_Stress > 35.0f) ? (m_Stress - 35.0f) / 650.0f : 0.0f;
        for (const auto &b : m_Buffs)
            if (b.kind == RationKind::Focus)
                penalty -= b.amount;
        return advClamp(penalty, 0.0f, 0.35f);
    }

    float SurvivalSystem::reloadDuration() const
    {
        float d = 1.45f;
        for (const auto &b : m_Buffs)
            if (b.kind == RationKind::Focus)
                d *= 0.82f;
        return d;
    }

    void SurvivalSystem::updateStress(GameState &gs, float dt)
    {
        bool nearEnemy = false;
        for (const auto &e : gs.enemies)
        {
            if (e.isAlive && advDistSq(e.position, gs.playerPos) < 5.5f * 5.5f)
            {
                nearEnemy = true;
                break;
            }
        }
        if (nearEnemy || gs.playerHealth < gs.playerMaxHealth * 0.35f)
            addStress((nearEnemy ? 4.0f : 2.0f) * dt);
        else
            calm(2.5f * dt);
    }

    void SurvivalSystem::updateBuffs(GameState &gs, float dt)
    {
        for (auto &b : m_Buffs)
        {
            b.timeLeft -= dt;
            if (b.kind == RationKind::AntiErosion)
            {
                gs.playerErosionLevel = advClamp(gs.playerErosionLevel - b.amount * 0.03f * dt, 0.0f, 100.0f);
            }
        }
        m_Buffs.erase(std::remove_if(m_Buffs.begin(), m_Buffs.end(), [](const ActiveBuff &b)
                                     { return b.timeLeft <= 0.0f; }),
                      m_Buffs.end());
    }

    void SurvivalSystem::updateReload(GameState &gs, float dt)
    {
        gs.fireCooldown = std::max(0.0f, gs.fireCooldown - dt);
        if (!m_Weapon.isReloading)
            return;
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

    void SurvivalSystem::updateSecondWindAndSoulLine(GameState &gs)
    {
        if (gs.playerHealth <= 0.0f && !m_SoulLineUsed)
        {
            m_SoulLineUsed = true;
            gs.playerHealth = 1.0f;
            m_Stress = 100.0f;
            std::cout << "[SURVIVAL] !! SOUL LINE !! Срыв смерти! Нить души удержала Пилота в Убежище 17!" << std::endl;
            return;
        }
        if (gs.playerHealth > 0.0f && gs.playerHealth < gs.playerMaxHealth * 0.16f && !m_SecondWindUsed && m_Stress >= 55.0f)
        {
            m_SecondWindUsed = true;
            gs.playerHealth = std::min(gs.playerMaxHealth, gs.playerHealth + 28.0f);
            m_Stress = advClamp(m_Stress - 25.0f, 0.0f, 100.0f);
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 4) TANK UTILITIES
    // ═══════════════════════════════════════════════════════════════════════════════

    void TankUtilitySystem::update(GameState &gs, float dt)
    {
        for (auto &s : m_Sparks)
        {
            s.pos += s.vel * dt;
            s.ttl -= dt;
        }
        m_Sparks.erase(std::remove_if(m_Sparks.begin(), m_Sparks.end(), [](const MuzzleSpark &s)
                                      { return s.ttl <= 0.0f; }),
                       m_Sparks.end());

        m_Runtime.utilityCooldown = std::max(0.0f, m_Runtime.utilityCooldown - dt);
        m_Runtime.cannonThermalLoad = advClamp(m_Runtime.cannonThermalLoad - 18.0f * dt, 0.0f, 100.0f);
        if (m_Runtime.cannonThermalLoad < 62.0f)
            m_Runtime.overheated = false;

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
                m_Runtime.towing = false;
        }
    }

    void TankUtilitySystem::nextUtility()
    {
        if (m_Runtime.utility == TankUtilityMode::BucketRig)
            m_Runtime.utility = TankUtilityMode::RamShield;
        else if (m_Runtime.utility == TankUtilityMode::RamShield)
            m_Runtime.utility = TankUtilityMode::TowCoupler;
        else
            m_Runtime.utility = TankUtilityMode::BucketRig;
    }

    void TankUtilitySystem::swapSeat()
    {
        m_Runtime.seat = (m_Runtime.seat == TankSeat::Driver) ? TankSeat::Gunner : TankSeat::Driver;
    }

    bool TankUtilitySystem::useUtility(GameState &gs)
    {
        if (m_Runtime.utilityCooldown > 0.0f)
            return false;
        switch (m_Runtime.utility)
        {
        case TankUtilityMode::BucketRig:
            carveFront(gs, 2, 35);
            m_Runtime.utilityCooldown = 1.0f;
            return true;
        case TankUtilityMode::RamShield:
            gs.titan.systems.tracksCondition = advClamp(gs.titan.systems.tracksCondition + 12.0f, 0.0f, 100.0f);
            gs.titan.health = advClamp(gs.titan.health + 10.0f, 0.0f, gs.titan.maxHealth);
            for (auto &e : gs.enemies)
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
        }
        return false;
    }

    bool TankUtilitySystem::registerCannonShot(GameState &gs, float heat)
    {
        if (m_Runtime.overheated)
            return false;
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

    bool TankUtilitySystem::repairInHangar(GameState &gs, PlayerInventory &inv)
    {
        if (!isInsideHangar(gs.titan.position))
            return false;
        if (!inv.removeItem(ITEM_REPAIR_KIT, 1))
            return false;
        gs.titan.health = advClamp(gs.titan.health + 70.0f, 0.0f, gs.titan.maxHealth);
        gs.titan.systems.coreEnergy = advClamp(gs.titan.systems.coreEnergy + 25.0f, 0.0f, 100.0f);
        gs.titan.systems.sensorLink = advClamp(gs.titan.systems.sensorLink + 35.0f, 0.0f, 100.0f);
        gs.titan.systems.tracksCondition = advClamp(gs.titan.systems.tracksCondition + 35.0f, 0.0f, 100.0f);
        gs.titan.systems.turretStatus = advClamp(gs.titan.systems.turretStatus + 35.0f, 0.0f, 100.0f);
        return true;
    }

    void TankUtilitySystem::carveFront(GameState &gs, int radius, int damage)
    {
        const int cx = static_cast<int>(std::round(gs.titan.position.x));
        const int cy = static_cast<int>(std::round(gs.titan.position.y));
        for (int x = cx - radius; x <= cx + radius; ++x)
        {
            for (int y = cy - radius; y <= cy + radius; ++y)
            {
                if (x < 0 || y < 0 || x >= Config::MAP_WIDTH || y >= Config::MAP_HEIGHT)
                    continue;
                if (gs.sectorMap[x][y] == 1)
                {
                    gs.wallDurability[x][y] -= damage;
                    if (gs.wallDurability[x][y] <= 0)
                        gs.sectorMap[x][y] = 0;
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 5) REACTIVE WORLD SYSTEM
    // ═══════════════════════════════════════════════════════════════════════════════

    int ReactiveWorldSystem::add(BreakableKind kind, Vector3D pos)
    {
        BreakableObject b;
        b.id = ++m_NextId;
        b.kind = kind;
        b.position = pos;
        switch (kind)
        {
        case BreakableKind::Glass:
            b.health = 8.0f;
            b.radius = 0.30f;
            break;
        case BreakableKind::Vegetation:
            b.health = 12.0f;
            b.radius = 0.45f;
            break;
        case BreakableKind::Crate:
            b.health = 35.0f;
            b.radius = 0.42f;
            break;
        case BreakableKind::Barrel:
            b.health = 25.0f;
            b.radius = 0.40f;
            break;
        case BreakableKind::Console:
            b.health = 55.0f;
            b.radius = 0.55f;
            break;
        }
        m_Breakables.push_back(b);
        return b.id;
    }

    void ReactiveWorldSystem::seedDefault()
    {
        if (!m_Breakables.empty())
            return;
        add(BreakableKind::Crate, {7.0f, 5.0f, 0.0f});
        add(BreakableKind::Glass, {9.0f, 8.0f, 0.0f});
        add(BreakableKind::Barrel, {12.0f, 9.0f, 0.0f});
        add(BreakableKind::Console, {4.0f, 13.0f, 0.0f});
        add(BreakableKind::Vegetation, {15.0f, 15.0f, 0.0f});
    }

    void ReactiveWorldSystem::update(GameState &gs, float dt)
    {
        for (auto &wave : m_Waves)
        {
            wave.ttl -= dt;
            wave.radius += (wave.maxRadius / 0.45f) * dt;
            applyWave(gs, wave, dt);
        }
        m_Waves.erase(std::remove_if(m_Waves.begin(), m_Waves.end(), [](const ShockWave &w)
                                     { return w.ttl <= 0.0f; }),
                      m_Waves.end());

        for (auto &b : m_Breakables)
        {
            if (b.broken)
                continue;
            b.position += b.velocity * dt;
            b.velocity = b.velocity * std::pow(0.04f, dt);
            if (b.kind == BreakableKind::Barrel && b.health <= 0.0f)
                explodeBarrel(gs, b);
            else if (b.health <= 0.0f)
                b.broken = true;
        }
    }

    void ReactiveWorldSystem::damageAt(GameState &gs, Vector3D pos, float radius, float damage, float impulse)
    {
        for (auto &b : m_Breakables)
        {
            if (b.broken)
                continue;
            const float d = advDist2D(pos, b.position);
            if (d <= radius + b.radius)
            {
                const float falloff = 1.0f - advClamp(d / std::max(radius, 0.01f), 0.0f, 1.0f);
                b.health -= damage * std::max(0.25f, falloff);
                Vector3D dir = advNormalize2D(b.position - pos);
                b.velocity += dir * (impulse * falloff);
            }
        }
        m_Waves.push_back({pos, 0.1f, radius * 1.7f, impulse, 0.45f});
        (void)gs;
    }

    void ReactiveWorldSystem::explodeBarrel(GameState &gs, BreakableObject &b)
    {
        b.broken = true;
        damageAt(gs, b.position, 2.8f, 75.0f, 5.5f);
        for (auto &e : gs.enemies)
        {
            if (e.isAlive && advDistSq(e.position, b.position) < 2.8f * 2.8f)
            {
                if (DamageSystem::applyEnemyDamage(gs, e, 90.0f, DamageType::Explosive))
                {
                    gs.score += 50;
                }
            }
        }
    }

    void ReactiveWorldSystem::applyWave(GameState &gs, const ShockWave &wave, float dt)
    {
        const float inner = std::max(0.0f, wave.radius - 0.35f);
        const float outer = wave.radius;
        const float dp = advDist2D(gs.playerPos, wave.origin);
        if (dp >= inner && dp <= outer)
        {
            gs.playerPos += advNormalize2D(gs.playerPos - wave.origin) * (wave.force * dt);
        }
        for (auto &e : gs.enemies)
        {
            if (!e.isAlive)
                continue;
            const float de = advDist2D(e.position, wave.origin);
            if (de >= inner && de <= outer)
                e.position += advNormalize2D(e.position - wave.origin) * (wave.force * 0.8f * dt);
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 6) STORY ROUTE
    // ═══════════════════════════════════════════════════════════════════════════════

    StoryRouteSystem::StoryRouteSystem()
    {
        m_Triggers.push_back({ZoneEventId::CryoLocker, {5.0f, 5.0f, 0.0f}, 1.5f, false, "Проснуться в криокамере"});
        m_Triggers.push_back({ZoneEventId::Archive, {4.0f, 13.0f, 0.0f}, 1.8f, false, "Синхронизировать архив"});
        m_Triggers.push_back({ZoneEventId::Garage, {3.0f, 3.0f, 0.0f}, 2.5f, false, "Найти BT-7274 в гараже"});
        m_Triggers.push_back({ZoneEventId::FirstCombat, {11.0f, 8.0f, 0.0f}, 4.0f, false, "Пережить первый контакт"});
        m_Triggers.push_back({ZoneEventId::Exterior, {18.0f, 18.0f, 0.0f}, 2.2f, false, "Выйти на поверхность"});
        m_Triggers.push_back({ZoneEventId::ReturnToBase, {5.0f, 5.0f, 0.0f}, 1.8f, false, "Вернуться в убежище"});
        rebuildRoute();
    }

    void StoryRouteSystem::update(GameState &gs, RadioTapeSystem *radio)
    {
        m_LastEvent.clear();
        for (auto &t : m_Triggers)
        {
            if (t.fired)
                continue;
            if (advDistSq(gs.playerPos, t.center) <= t.radius * t.radius)
            {
                fire(gs, t, radio);
                break;
            }
        }
    }

    void StoryRouteSystem::rebuildRoute()
    {
        m_Route.clear();
        for (const auto &t : m_Triggers)
            m_Route.push_back({t.objective, t.fired});
    }

    void StoryRouteSystem::mark(const std::string &text)
    {
        for (auto &r : m_Route)
            if (r.text == text)
                r.completed = true;
    }

    void StoryRouteSystem::fire(GameState &gs, ZoneTrigger &t, RadioTapeSystem *radio)
    {
        t.fired = true;
        mark(t.objective);
        gs.score += 75;
        switch (t.id)
        {
        case ZoneEventId::CryoLocker:
            m_LastEvent = "Криокамера открыта. Найдите Pip-Pad.";
            if (radio)
                radio->discoverTape("CRYO_00");
            break;
        case ZoneEventId::Archive:
            gs.story.archiveRecovered = true;
            m_LastEvent = "Архив восстановлен. Получен допуск к гаражу.";
            break;
        case ZoneEventId::Garage:
            gs.story.tankLinked = true;
            m_LastEvent = "BT-7274 найден. Доступна синхронизация пилота.";
            if (radio)
                radio->discoverTape("GARAGE_BT");
            break;
        case ZoneEventId::FirstCombat:
            m_LastEvent = "Первый контакт. Враги теперь активнее реагируют на шум.";
            break;
        case ZoneEventId::Exterior:
            gs.story.exitedBunker = true;
            m_LastEvent = "Поверхность. Погодные угрозы активны.";
            if (radio)
                radio->discoverTape("SURFACE_17");
            break;
        case ZoneEventId::ReturnToBase:
            m_LastEvent = "Возвращение в убежище. Можно чинить танк в ангаре.";
            break;
        case ZoneEventId::SurfaceClearance:
            m_LastEvent = "Допуск поверхности подтверждён.";
            break;
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 7) SKILL SYSTEM
    // ═══════════════════════════════════════════════════════════════════════════════

    SkillSystem::SkillSystem()
    {
        m_Skills.rank[SkillId::ArchiveSync] = 0;
        m_Skills.rank[SkillId::FootKill] = 0;
        m_Skills.rank[SkillId::TankAction] = 0;
        m_Skills.rank[SkillId::StressSurvival] = 0;
        m_Skills.rank[SkillId::Mechanic] = 0;
        m_Skills.rank[SkillId::Builder] = 0;
    }

    void SkillSystem::grantXp(GameState &gs, int xp)
    {
        gs.characterProg.experience += xp;
        while (gs.characterProg.experience >= xpForNext(gs.characterProg.level))
        {
            gs.characterProg.experience -= xpForNext(gs.characterProg.level);
            gs.characterProg.level += 1;
            gs.characterProg.unusedPoints += 1;
            gs.playerMaxHealth += 5.0f;
            gs.playerHealth = gs.playerMaxHealth;
        }
    }

    bool SkillSystem::upgrade(GameState &gs, SkillId id)
    {
        if (gs.characterProg.unusedPoints <= 0)
            return false;
        int &r = m_Skills.rank[id];
        if (r >= 5)
            return false;
        ++r;
        --gs.characterProg.unusedPoints;
        applyPassive(gs, id, r);
        return true;
    }

    void SkillSystem::applyPassive(GameState &gs, SkillId id, int r)
    {
        switch (id)
        {
        case SkillId::ArchiveSync:
            gs.regionalGrid.localRelayAvailable = true;
            if (r >= 3)
                gs.regionalGrid.towerSyncRecovered = true;
            break;
        case SkillId::StressSurvival:
            gs.playerMaxHealth += 3.0f;
            gs.playerHealth += 3.0f;
            break;
        case SkillId::Mechanic:
            gs.titan.systems.coreEnergy = advClamp(gs.titan.systems.coreEnergy + 4.0f, 0.0f, 100.0f);
            break;
        default:
            break;
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 8) LOOT GENERATOR
    // ═══════════════════════════════════════════════════════════════════════════════

    LootGenerator::LootGenerator() : m_Rng(0x17A55E7u)
    {
        addDefaults();
    }

    InventoryItem LootGenerator::roll(LootTier tier)
    {
        auto &table = m_Tables[tier];
        if (table.empty())
            return {1, ItemType::Resource, 1, 0.1f, "SCRAP"};
        float total = 0.0f;
        for (const auto &e : table)
            total += e.weight;
        std::uniform_real_distribution<float> pick(0.0f, total);
        float r = pick(m_Rng);
        for (const auto &e : table)
        {
            r -= e.weight;
            if (r <= 0.0f)
            {
                InventoryItem item = e.item;
                std::uniform_int_distribution<int> q(e.minQty, e.maxQty);
                item.quantity = q(m_Rng);
                return item;
            }
        }
        return table.back().item;
    }

    void LootGenerator::fillContainer(LootContainer &c, LootTier tier, int rolls)
    {
        c.containsItems.clear();
        for (int i = 0; i < rolls; ++i)
            c.containsItems.push_back(roll(tier));
    }

    void LootGenerator::normalizeWorldLoot(GameState &gs)
    {
        int i = 0;
        for (auto &c : gs.lootContainers)
        {
            if (!c.containsItems.empty())
                continue;
            LootTier tier = LootTier::Common;
            if (c.type == LootContainerType::IronSafe)
                tier = LootTier::Rare;
            if (c.type == LootContainerType::DevVault)
                tier = LootTier::Legendary;
            fillContainer(c, tier, 1 + (i++ % 3));
        }
    }

    void LootGenerator::add(LootTier tier, unsigned int id, ItemType type, std::string name, int minQ, int maxQ, float w, float unitWeight)
    {
        m_Tables[tier].push_back({{id, type, 1, unitWeight, std::move(name)}, minQ, maxQ, w});
    }

    void LootGenerator::addDefaults()
    {
        add(LootTier::Common, 201, ItemType::Resource, "SCRAP METAL", 2, 8, 8.0f, 0.08f);
        add(LootTier::Common, SurvivalSystem::ITEM_AMMO_556, ItemType::Ammo, "5.56 AMMO BOX", 1, 3, 6.0f, 0.15f);
        add(LootTier::Common, SurvivalSystem::ITEM_RATION_PROTEIN, ItemType::Medicine, "PROTEIN RATION", 1, 2, 3.0f, 0.25f);
        add(LootTier::Uncommon, SurvivalSystem::ITEM_STIM, ItemType::Medicine, "STIM INJECTOR", 1, 2, 5.0f, 0.10f);
        add(LootTier::Uncommon, SurvivalSystem::ITEM_RATION_STAMINA, ItemType::Medicine, "STAMINA RATION", 1, 2, 4.0f, 0.25f);
        add(LootTier::Rare, TankUtilitySystem::ITEM_REPAIR_KIT, ItemType::Resource, "BT REPAIR KIT", 1, 2, 4.0f, 0.60f);
        add(LootTier::Rare, SurvivalSystem::ITEM_RATION_FOCUS, ItemType::Medicine, "FOCUS RATION", 1, 1, 3.0f, 0.20f);
        add(LootTier::Epic, 777, ItemType::Weapon, "GMOD TOOLGUN", 1, 1, 1.0f, 0.0f);
        add(LootTier::Legendary, 999, ItemType::Quest, "DEV DEBUG BACKPACK", 1, 1, 1.0f, 0.0f);
        add(LootTier::Legendary, 888, ItemType::Weapon, "DEBUGGUN CHAIN LIGHTNING", 1, 1, 1.0f, 0.0f);

        // Внедрение внутриигровых предметов из Fallout 76 UI Art Collection (Jake Raymor):
        add(LootTier::Uncommon, 520, ItemType::Medicine, "SUGAR FREE NUKA SHINE", 1, 2, 4.0f, 0.20f);
        add(LootTier::Rare, 521, ItemType::Medicine, "WEIGHT BE-GONE POTION", 1, 1, 3.0f, 0.10f);
        add(LootTier::Epic, 710, ItemType::Weapon, "SHEEPSQUATCH QUILL CLUB", 1, 1, 2.0f, 4.5f);
        add(LootTier::Legendary, 810, ItemType::Quest, "SPACE EXPLORER BACKPACK", 1, 1, 1.0f, 1.0f);
        add(LootTier::Rare, 811, ItemType::Resource, "PIONEER SCOUT BADGE", 1, 1, 4.0f, 0.01f);
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 9) CAMP SYSTEM
    // ═══════════════════════════════════════════════════════════════════════════════

    void CampSystem::cycleType()
    {
        if (m_Preview.activeType == CampObjectType::ConcreteWall)
            m_Preview.activeType = CampObjectType::DefenseTurret;
        else if (m_Preview.activeType == CampObjectType::DefenseTurret)
            m_Preview.activeType = CampObjectType::SupplyCrate;
        else
            m_Preview.activeType = CampObjectType::ConcreteWall;
    }

    void CampSystem::updatePreview(const GameState &gs, const Vector3D &mouseWorld)
    {
        m_Preview.tileX = static_cast<int>(std::floor(mouseWorld.x));
        m_Preview.tileY = static_cast<int>(std::floor(mouseWorld.y));
        m_Preview.isPlacementValid = canPlace(gs, m_Preview.tileX, m_Preview.tileY);
    }

    bool CampSystem::place(GameState &gs, PlayerInventory &inv, float costMult)
    {
        if (!m_Enabled || !m_Preview.isPlacementValid)
            return false;
        const int cost = std::max(1, static_cast<int>(baseCost(m_Preview.activeType) * costMult));
        if (!inv.removeItem(ITEM_BUILD_MATERIAL, cost))
            return false;

        CampObject obj;
        obj.id = ++m_NextId;
        obj.type = m_Preview.activeType;
        obj.tileX = m_Preview.tileX;
        obj.tileY = m_Preview.tileY;
        obj.health = (obj.type == CampObjectType::ConcreteWall) ? 180.0f : (obj.type == CampObjectType::DefenseTurret ? 90.0f : 60.0f);
        m_Objects.push_back(obj);

        if (obj.type == CampObjectType::ConcreteWall)
        {
            gs.sectorMap[obj.tileX][obj.tileY] = 1;
            gs.wallDurability[obj.tileX][obj.tileY] = 180;
        }
        else if (obj.type == CampObjectType::SupplyCrate)
        {
            LootContainer c;
            c.position = {obj.tileX + 0.5f, obj.tileY + 0.5f, 0.0f};
            c.type = LootContainerType::WoodenCrate;
            gs.lootContainers.push_back(c);
        }
        return true;
    }

    void CampSystem::updateTurrets(GameState &gs, float dt)
    {
        (void)dt;
        for (const auto &obj : m_Objects)
        {
            if (obj.type != CampObjectType::DefenseTurret)
                continue;
            Vector3D pos{obj.tileX + 0.5f, obj.tileY + 0.5f, 0.0f};
            Enemy *best = nullptr;
            float bestD = 5.5f * 5.5f;
            for (auto &e : gs.enemies)
            {
                if (!e.isAlive)
                    continue;
                const float d = advDistSq(pos, e.position);
                if (d < bestD)
                {
                    bestD = d;
                    best = &e;
                }
            }
            if (best)
            {
                DamageSystem::applyEnemyDamage(gs, *best, 12.0f * gs.deltaTime, DamageType::Kinetic);
            }
        }
    }

    int CampSystem::baseCost(CampObjectType type)
    {
        switch (type)
        {
        case CampObjectType::ConcreteWall:
            return 4;
        case CampObjectType::DefenseTurret:
            return 12;
        case CampObjectType::SupplyCrate:
            return 6;
        }
        return 5;
    }

    bool CampSystem::canPlace(const GameState &gs, int x, int y) const
    {
        if (x < 0 || y < 0 || x >= Config::MAP_WIDTH || y >= Config::MAP_HEIGHT)
            return false;
        if (gs.sectorMap[x][y] == 1)
            return false;
        if (advDistSq({x + 0.5f, y + 0.5f, 0.0f}, gs.playerPos) > 5.0f * 5.0f)
            return false;
        for (const auto &obj : m_Objects)
            if (obj.tileX == x && obj.tileY == y)
                return false;
        return true;
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 11) PREFAB LIBRARY & TOOLGUN
    // ═══════════════════════════════════════════════════════════════════════════════

    PrefabLibrary::PrefabLibrary()
    {
        m_Prefabs.push_back({"small_bunker_room", {"#####", "#...#", "#.c.#", "#...#", "#####"}});
        m_Prefabs.push_back({"garage_checkpoint", {"#######", "#.....#", "#..c..#", "#.....#", "###.###"}});
        m_Prefabs.push_back({"defense_corner", {"###", "#t.", "#.."}});

        // Внутриигровые рекламные постеры и вывески из Fallout 76 UI Art Collection:
        m_Prefabs.push_back({"big_freds_bbq_shack", {"######", "#....#", "#.c..#", "######"}});
        m_Prefabs.push_back({"nuka_speedway_hotrod", {"####", "#..#", "####"}});
        m_Prefabs.push_back({"big_als_tattoo_parlor", {"#####", "#...#", "#####"}});
    }

    const PrefabDef *PrefabLibrary::get(const std::string &name) const
    {
        for (const auto &p : m_Prefabs)
            if (p.name == name)
                return &p;
        return m_Prefabs.empty() ? nullptr : &m_Prefabs.front();
    }

    void ToolGunSystem::cycleMode()
    {
        if (m_Mode == ToolGunMode::SpawnPrefab)
            m_Mode = ToolGunMode::Delete;
        else if (m_Mode == ToolGunMode::Delete)
            m_Mode = ToolGunMode::PaintErosion;
        else if (m_Mode == ToolGunMode::PaintErosion)
            m_Mode = ToolGunMode::Validate;
        else if (m_Mode == ToolGunMode::Validate)
            m_Mode = ToolGunMode::Export;
        else
            m_Mode = ToolGunMode::SpawnPrefab;
    }

    bool ToolGunSystem::apply(GameState &gs, Vector3D where, const PrefabLibrary &lib)
    {
        const int x = static_cast<int>(std::floor(where.x));
        const int y = static_cast<int>(std::floor(where.y));
        if (x < 0 || y < 0 || x >= Config::MAP_WIDTH || y >= Config::MAP_HEIGHT)
            return false;
        switch (m_Mode)
        {
        case ToolGunMode::SpawnPrefab:
            return spawnPrefab(gs, x, y, lib);
        case ToolGunMode::Delete:
            return deleteTile(gs, x, y);
        case ToolGunMode::PaintErosion:
            return paintErosion(gs, x, y);
        case ToolGunMode::Validate:
            m_LastValidation = validate(gs);
            std::cout << "[TOOLGUN] " << m_LastValidation << std::endl;
            return true;
        case ToolGunMode::Export:
        {
            m_LastExport = exportMap(gs);
            std::ofstream out("saves/exported_map_level.cfg");
            if (out.is_open())
            {
                out << m_LastExport;
                out.close();
                std::cout << "[TOOLGUN EXPORT] Карта уровня экспортирована в saves/exported_map_level.cfg!" << std::endl;
            }
            return true;
        }
        }
        return false;
    }

    bool ToolGunSystem::undo(GameState &gs)
    {
        if (m_Undo.empty())
            return false;
        auto action = std::move(m_Undo.back());
        m_Undo.pop_back();
        action.undo(gs);
        m_Redo.push_back(std::move(action));
        return true;
    }

    bool ToolGunSystem::redo(GameState &gs)
    {
        if (m_Redo.empty())
            return false;
        auto action = std::move(m_Redo.back());
        m_Redo.pop_back();
        action.redo(gs);
        m_Undo.push_back(std::move(action));
        return true;
    }

    void ToolGunSystem::pushAction(ToolGunAction a)
    {
        m_Undo.push_back(std::move(a));
        m_Redo.clear();
        if (m_Undo.size() > 64)
            m_Undo.erase(m_Undo.begin());
    }

    bool ToolGunSystem::spawnPrefab(GameState &gs, int x, int y, const PrefabLibrary &lib)
    {
        const PrefabDef *prefab = lib.get("small_bunker_room");
        if (!prefab)
            return false;
        auto oldMap = gs.sectorMap;
        auto oldDur = gs.wallDurability;
        for (int row = 0; row < static_cast<int>(prefab->rows.size()); ++row)
        {
            for (int col = 0; col < static_cast<int>(prefab->rows[row].size()); ++col)
            {
                const int tx = x + col;
                const int ty = y + row;
                if (tx < 0 || ty < 0 || tx >= Config::MAP_WIDTH || ty >= Config::MAP_HEIGHT)
                    continue;
                const char c = prefab->rows[row][col];
                if (c == '#')
                {
                    gs.sectorMap[tx][ty] = 1;
                    gs.wallDurability[tx][ty] = 110;
                }
                else if (c == '.')
                {
                    gs.sectorMap[tx][ty] = 0;
                    gs.wallDurability[tx][ty] = 0;
                }
                else if (c == 'c')
                {
                    LootContainer lc;
                    lc.position = {tx + 0.5f, ty + 0.5f, 0.0f};
                    lc.type = LootContainerType::WoodenCrate;
                    gs.lootContainers.push_back(lc);
                }
            }
        }
        auto newMap = gs.sectorMap;
        auto newDur = gs.wallDurability;
        pushAction({"spawn prefab", [oldMap, oldDur](GameState &s)
                    { s.sectorMap = oldMap; s.wallDurability = oldDur; },
                    [newMap, newDur](GameState &s)
                    { s.sectorMap = newMap; s.wallDurability = newDur; }});
        return true;
    }

    bool ToolGunSystem::deleteTile(GameState &gs, int x, int y)
    {
        const int oldTile = gs.sectorMap[x][y];
        const int oldDur = gs.wallDurability[x][y];
        gs.sectorMap[x][y] = 0;
        gs.wallDurability[x][y] = 0;
        pushAction({"delete tile", [x, y, oldTile, oldDur](GameState &s)
                    { s.sectorMap[x][y] = oldTile; s.wallDurability[x][y] = oldDur; },
                    [x, y](GameState &s)
                    { s.sectorMap[x][y] = 0; s.wallDurability[x][y] = 0; }});
        return true;
    }

    bool ToolGunSystem::paintErosion(GameState &gs, int x, int y)
    {
        const float old = gs.etherErosionMap[x][y];
        gs.etherErosionMap[x][y] = advClamp(old + 10.0f, 0.0f, 100.0f);
        const float now = gs.etherErosionMap[x][y];
        pushAction({"paint erosion", [x, y, old](GameState &s)
                    { s.etherErosionMap[x][y] = old; },
                    [x, y, now](GameState &s)
                    { s.etherErosionMap[x][y] = now; }});
        return true;
    }

    std::string ToolGunSystem::validate(const GameState &gs) const
    {
        int walls = 0;
        int brokenWalls = 0;
        int invalidEnemies = 0;
        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                if (gs.sectorMap[x][y] == 1)
                {
                    ++walls;
                    if (gs.wallDurability[x][y] <= 0)
                        ++brokenWalls;
                }
            }
        }
        for (const auto &e : gs.enemies)
        {
            const int ex = static_cast<int>(e.position.x);
            const int ey = static_cast<int>(e.position.y);
            if (ex < 0 || ey < 0 || ex >= Config::MAP_WIDTH || ey >= Config::MAP_HEIGHT || gs.sectorMap[ex][ey] == 1)
                ++invalidEnemies;
        }

        bool playerBlocked = false;
        int px = static_cast<int>(gs.playerPos.x);
        int py = static_cast<int>(gs.playerPos.y);
        if (px >= 0 && px < Config::MAP_WIDTH && py >= 0 && py < Config::MAP_HEIGHT)
            playerBlocked = (gs.sectorMap[px][py] == 1);

        std::ostringstream out;
        out << "VALIDATION: walls=" << walls << " brokenDurability=" << brokenWalls
            << " invalidEnemies=" << invalidEnemies << (playerBlocked ? " [CRITICAL: PLAYER STUCK IN WALL!]" : " [GRID OK]");
        return out.str();
    }

    std::string ToolGunSystem::exportMap(const GameState &gs) const
    {
        std::ostringstream out;
        out << "# Bunker Protocol ISO map export\n";
        for (int y = 0; y < Config::MAP_HEIGHT; ++y)
        {
            for (int x = 0; x < Config::MAP_WIDTH; ++x)
                out << (gs.sectorMap[x][y] == 1 ? '#' : '.');
            out << '\n';
        }
        return out.str();
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 12) OBJ MODEL LOADER
    // ═══════════════════════════════════════════════════════════════════════════════

    ObjModel ObjModelLoader::load(const std::string &path)
    {
        ObjModel model;
        std::ifstream in(path);
        if (!in)
            return model;
        std::string line;
        while (std::getline(in, line))
        {
            std::istringstream ss(line);
            std::string tag;
            ss >> tag;
            if (tag == "v")
            {
                ObjVertex v;
                ss >> v.x >> v.y >> v.z;
                model.vertices.push_back(v);
            }
            else if (tag == "f")
            {
                std::array<int, 3> idx{{0, 0, 0}};
                for (int i = 0; i < 3; ++i)
                {
                    std::string token;
                    ss >> token;
                    idx[i] = parseFaceIndex(token) - 1;
                }
                if (idx[0] >= 0 && idx[1] >= 0 && idx[2] >= 0)
                    model.faces.push_back({idx});
            }
        }
        return model;
    }

    int ObjModelLoader::parseFaceIndex(const std::string &token)
    {
        std::string n;
        for (char c : token)
        {
            if (c == '/')
                break;
            n.push_back(c);
        }
        if (n.empty())
            return 0;
        return std::max(0, std::atoi(n.c_str()));
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 13) LANLINE SERVICES
    // ═══════════════════════════════════════════════════════════════════════════════

    int LanlineServices::createLocalLobby(const std::string &playerName)
    {
        m_Peers.clear();
        m_Deliveries.clear();
        m_Chat.clear();
        m_Connected = true;
        m_LobbyId = ++m_NextLobbyId;
        addPeer(playerName.empty() ? "Solo_Pilot" : playerName);
        addPeer("Gunner_BT7274");
        addPeer("Scout_LogHorizon");
        addPeer("Vault17_Quartermaster");
        systemMessage("LANLINE net #1001 connected. 4 squad combatants online.");
        return m_LobbyId;
    }

    int LanlineServices::addPeer(const std::string &name)
    {
        LanlinePeer p;
        p.id = ++m_NextPeerId;
        p.name = name;
        m_Peers.push_back(p);
        return p.id;
    }

    void LanlineServices::sendChat(int fromPeer, const std::string &text)
    {
        if (!m_Connected)
            return;
        m_Chat.push_back({fromPeer, text, 10.0f});
        if (m_Chat.size() > 20)
            m_Chat.erase(m_Chat.begin());
    }

    void LanlineServices::setVoice(int peerId, bool active)
    {
        for (auto &p : m_Peers)
            if (p.id == peerId)
                p.voiceActive = active;
    }

    int LanlineServices::requestDelivery(const std::string &payload, Vector3D dropPos)
    {
        LanlineDelivery d;
        d.id = ++m_NextDeliveryId;
        d.payload = payload;
        d.dropPos = dropPos;
        d.eta = 8.0f + static_cast<float>((m_NextDeliveryId * 7) % 6);
        m_Deliveries.push_back(d);
        return d.id;
    }

    void LanlineServices::update(GameState &gs, PlayerInventory &inv, float dt)
    {
        for (auto &c : m_Chat)
            c.ttl -= dt;
        m_Chat.erase(std::remove_if(m_Chat.begin(), m_Chat.end(), [](const LanlineChatMessage &c)
                                    { return c.ttl <= 0.0f; }),
                     m_Chat.end());

        for (auto &d : m_Deliveries)
        {
            if (d.delivered)
                continue;
            d.eta -= dt;
            if (d.eta <= 0.0f)
            {
                d.delivered = true;
                if (d.payload == "ammo")
                    inv.addItem(SurvivalSystem::ITEM_AMMO_556, ItemType::Ammo, 2, 0.15f, "5.56 AMMO BOX");
                else if (d.payload == "med")
                    inv.addItem(SurvivalSystem::ITEM_STIM, ItemType::Medicine, 1, 0.10f, "STIM INJECTOR");
                else
                    inv.addItem(201, ItemType::Resource, 10, 0.08f, "SCRAP METAL");
                gs.score += 25;
                systemMessage("Delivery arrived: " + d.payload);
            }
        }

        static float s_ChatTimer = 15.0f;
        s_ChatTimer -= dt;
        if (s_ChatTimer <= 0.0f && m_Peers.size() > 1)
        {
            s_ChatTimer = 30.0f;
            sendChat(m_Peers[1].id, "Сенсоры бастиона в норме. Готов прикрыть огнём.");
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 14) PROFILE / SESSION MIGRATION
    // ═══════════════════════════════════════════════════════════════════════════════

    void ProfileSessionSystem::startSession(PlayerProfile &p)
    {
        migrate(p);
        p.sessionsPlayed += 1;
        m_SessionTime = 0.0f;
        m_Started = true;
    }

    void ProfileSessionSystem::update(PlayerProfile &p, float dt)
    {
        if (!m_Started)
            return;
        m_SessionTime += dt;
        p.totalPlayTime += dt;
    }

    void ProfileSessionSystem::migrate(PlayerProfile &p)
    {
        if (p.version < 15)
        {
            p.flags["legacy_import"] = 1;
            p.version = 15;
        }
        if (p.version < 16)
        {
            p.flags["advanced_mechanics_enabled"] = 1;
            p.version = 16;
        }
    }

    std::string ProfileSessionSystem::summary(const PlayerProfile &p) const
    {
        std::ostringstream ss;
        ss << "PROFILE v" << p.version << " name=" << p.playerName
           << " kills=" << p.totalKills << " deaths=" << p.totalDeaths
           << " sessions=" << p.sessionsPlayed << " playtime=" << std::fixed << std::setprecision(1) << p.totalPlayTime;
        return ss.str();
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 15) FACADE
    // ═══════════════════════════════════════════════════════════════════════════════

    void AdvancedMechanics::initialize(GameState &gs, PlayerInventory &inv)
    {
        (void)inv;
        reactive.seedDefault();
        loot.normalizeWorldLoot(gs);
        profile.startSession(playerProfile);
        lanline.createLocalLobby(playerProfile.playerName);

        ObjModel testModel = ObjModelLoader::load("assets/models/bastion.obj");
        if (!testModel.empty())
        {
            std::cout << "[OBJ LOADER] Модель bastion.obj успешно загружена: "
                      << testModel.vertices.size() << " вершин, "
                      << testModel.faces.size() << " полигонов." << std::endl;
        }
    }

    void AdvancedMechanics::update(GameState &gs, PlayerInventory &inv, const InputSnapshot &input, float dt)
    {
        profile.update(playerProfile, dt);
        weather.update(gs, dt);
        radio.update(dt);
        story.update(gs, &radio);
        survival.update(gs, inv, dt);
        tankUtility.update(gs, dt);
        reactive.update(gs, dt);
        camp.updatePreview(gs, gs.mouseWorldPos);
        camp.updateTurrets(gs, dt);
        lanline.update(gs, inv, dt);

        if (input.toggleCamp)
            camp.toggle();
        if (input.switchWeapon && gs.playerMode == UnitMode::Titan)
            tankUtility.nextUtility();
        if (input.dismountVehicle && gs.playerMode == UnitMode::Titan)
            tankUtility.swapSeat();
        if (input.interact)
        {
            tankUtility.repairInHangar(gs, inv);
        }
    }

    void AdvancedMechanics::onExplosion(GameState &gs, Vector3D pos, float radius, float damage)
    {
        reactive.damageAt(gs, pos, radius, damage, radius * 1.8f);
    }

} // namespace bunker
