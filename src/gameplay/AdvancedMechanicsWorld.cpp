#include "gameplay/AdvancedMechanics.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace bunker
{

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
        default:
            break;
        }
        m_Breakables.push_back(b);
        return b.id;
    }

    void ReactiveWorldSystem::seedDefault()
    {
        if (!m_Breakables.empty())
        {
            return;
        }
        add(BreakableKind::Crate, {7.0f, 5.0f, 0.0f});
        add(BreakableKind::Glass, {9.0f, 8.0f, 0.0f});
        add(BreakableKind::Barrel, {12.0f, 9.0f, 0.0f});
        add(BreakableKind::Console, {4.0f, 13.0f, 0.0f});
        add(BreakableKind::Vegetation, {15.0f, 15.0f, 0.0f});
    }

    void ReactiveWorldSystem::update(GameState& gs, float dt)
    {
        for (auto& wave : m_Waves)
        {
            wave.ttl -= dt;
            wave.radius += (wave.maxRadius / 0.45f) * dt;
            applyWave(gs, wave, dt);
        }
        m_Waves.erase(std::remove_if(m_Waves.begin(), m_Waves.end(), [](const ShockWave& w) { return w.ttl <= 0.0f; }),
                      m_Waves.end());

        for (auto& b : m_Breakables)
        {
            if (b.broken)
            {
                continue;
            }
            b.position += b.velocity * dt;
            b.velocity = b.velocity * std::pow(0.04f, dt);
            if (b.kind == BreakableKind::Barrel && b.health <= 0.0f)
            {
                explodeBarrel(gs, b);
            }
            else if (b.health <= 0.0f)
            {
                b.broken = true;
            }
        }
    }

    void ReactiveWorldSystem::damageAt(GameState& gs, Vector3D pos, float radius, float damage, float impulse)
    {
        for (auto& b : m_Breakables)
        {
            if (b.broken)
            {
                continue;
            }
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

    void ReactiveWorldSystem::explodeBarrel(GameState& gs, BreakableObject& b)
    {
        b.broken = true;
        damageAt(gs, b.position, 2.8f, 75.0f, 5.5f);
        for (auto& e : gs.enemies)
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

    void ReactiveWorldSystem::applyWave(GameState& gs, const ShockWave& wave, float dt)
    {
        const float inner = std::max(0.0f, wave.radius - 0.35f);
        const float outer = wave.radius;
        const float dp = advDist2D(gs.playerPos, wave.origin);
        if (dp >= inner && dp <= outer)
        {
            gs.playerPos += advNormalize2D(gs.playerPos - wave.origin) * (wave.force * dt);
        }
        for (auto& e : gs.enemies)
        {
            if (!e.isAlive)
            {
                continue;
            }
            const float de = advDist2D(e.position, wave.origin);
            if (de >= inner && de <= outer)
            {
                e.position += advNormalize2D(e.position - wave.origin) * (wave.force * 0.8f * dt);
            }
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

    void StoryRouteSystem::update(GameState& gs, RadioTapeSystem* radio)
    {
        m_LastEvent.clear();
        for (auto& t : m_Triggers)
        {
            if (t.fired)
            {
                continue;
            }
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
        for (const auto& t : m_Triggers)
        {
            m_Route.push_back({t.objective, t.fired});
        }
    }

    void StoryRouteSystem::mark(const std::string& text)
    {
        for (auto& r : m_Route)
        {
            if (r.text == text)
            {
                r.completed = true;
            }
        }
    }

    void StoryRouteSystem::fire(GameState& gs, ZoneTrigger& t, RadioTapeSystem* radio)
    {
        t.fired = true;
        mark(t.objective);
        gs.score += 75;
        switch (t.id)
        {
        case ZoneEventId::CryoLocker:
            m_LastEvent = "Криокамера открыта. Найдите Pip-Pad.";
            if (radio)
            {
                radio->discoverTape("CRYO_00");
            }
            break;
        case ZoneEventId::Archive:
            gs.story.archiveRecovered = true;
            m_LastEvent = "Архив восстановлен. Получен допуск к гаражу.";
            if (radio)
            {
                radio->discoverTape("LAB_60S");
                radio->discoverTape("MECH_WAR");
            }
            break;
        case ZoneEventId::Garage:
            gs.story.tankLinked = true;
            m_LastEvent = "BT-7274 найден. Доступна синхронизация пилота.";
            if (radio)
            {
                radio->discoverTape("GARAGE_BT");
            }
            break;
        case ZoneEventId::FirstCombat:
            m_LastEvent = "Первый контакт. Враги теперь активнее реагируют на шум.";
            break;
        case ZoneEventId::Exterior:
            gs.story.exitedBunker = true;
            m_LastEvent = "Поверхность. Погодные угрозы активны.";
            if (radio)
            {
                radio->discoverTape("SURFACE_17");
            }
            break;
        case ZoneEventId::ReturnToBase:
            m_LastEvent = "Возвращение в убежище. Можно чинить танк в ангаре.";
            break;
        case ZoneEventId::SurfaceClearance:
            m_LastEvent = "Допуск поверхности подтверждён.";
            if (radio)
            {
                radio->discoverTape("PORT_16");
                radio->discoverTape("BATTLE_215");
            }
            break;
        default:
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

    void SkillSystem::grantXp(GameState& gs, int xp)
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

    bool SkillSystem::upgrade(GameState& gs, SkillId id)
    {
        if (gs.characterProg.unusedPoints <= 0)
        {
            return false;
        }
        int& r = m_Skills.rank[id];
        if (r >= 5)
        {
            return false;
        }
        ++r;
        --gs.characterProg.unusedPoints;
        applyPassive(gs, id, r);
        return true;
    }

    void SkillSystem::applyPassive(GameState& gs, SkillId id, int r)
    {
        switch (id)
        {
        case SkillId::ArchiveSync:
            gs.regionalGrid.localRelayAvailable = true;
            if (r >= 3)
            {
                gs.regionalGrid.towerSyncRecovered = true;
            }
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
        auto& table = m_Tables[tier];
        if (table.empty())
        {
            return {1, ItemType::Resource, 1, 0.1f, "SCRAP"};
        }
        float total = 0.0f;
        for (const auto& e : table)
        {
            total += e.weight;
        }
        std::uniform_real_distribution<float> pick(0.0f, total);
        float r = pick(m_Rng);
        for (const auto& e : table)
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

    void LootGenerator::fillContainer(LootContainer& c, LootTier tier, int rolls)
    {
        c.containsItems.clear();
        for (int i = 0; i < rolls; ++i)
        {
            c.containsItems.push_back(roll(tier));
        }

        // Гарантированный специальный предмет: Аналог ядра Титана в легендарных хранилищах
        if (c.type == LootContainerType::DevVault)
        {
            c.containsItems.push_back({1001, ItemType::Quest, 1, 15.0f, "ТАНКОВОЕ ЯДРО БТ (ANALOG TITAN CORE)"});
        }
    }

    void LootGenerator::normalizeWorldLoot(GameState& gs)
    {
        int i = 0;
        for (auto& c : gs.lootContainers)
        {
            if (!c.containsItems.empty())
            {
                continue;
            }
            LootTier tier = LootTier::Common;
            if (c.type == LootContainerType::IronSafe)
            {
                tier = LootTier::Rare;
            }
            if (c.type == LootContainerType::DevVault)
            {
                tier = LootTier::Legendary;
            }
            fillContainer(c, tier, 1 + (i++ % 3));
        }
    }

    void LootGenerator::add(LootTier tier, unsigned int id, ItemType type, std::string name, int minQ, int maxQ,
                            float w, float unitWeight)
    {
        m_Tables[tier].push_back({{id, type, 1, unitWeight, std::move(name)}, minQ, maxQ, w});
    }

    void LootGenerator::addDefaults()
    {
        add(LootTier::Common, 201, ItemType::Resource, "SCRAP METAL", 2, 8, 8.0f, 0.08f);
        add(LootTier::Common, SurvivalSystem::ITEM_AMMO_556, ItemType::Ammo, "5.56 AMMO BOX", 1, 3, 6.0f, 0.15f);
        add(LootTier::Common, SurvivalSystem::ITEM_RATION_PROTEIN, ItemType::Medicine, "PROTEIN RATION", 1, 2, 3.0f,
            0.25f);
        add(LootTier::Uncommon, SurvivalSystem::ITEM_STIM, ItemType::Medicine, "STIM INJECTOR", 1, 2, 5.0f, 0.10f);
        add(LootTier::Uncommon, SurvivalSystem::ITEM_RATION_STAMINA, ItemType::Medicine, "STAMINA RATION", 1, 2, 4.0f,
            0.25f);
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
        switch (m_Preview.activeType)
        {
        case CampObjectType::ConcreteWall:
            m_Preview.activeType = CampObjectType::DefenseTurret;
            break;
        case CampObjectType::DefenseTurret:
            m_Preview.activeType = CampObjectType::SupplyCrate;
            break;
        default:
            m_Preview.activeType = CampObjectType::ConcreteWall;
            break;
        }
    }

    void CampSystem::updatePreview(const GameState& gs, const Vector3D& mouseWorld)
    {
        m_Preview.tileX = static_cast<int>(std::floor(mouseWorld.x));
        m_Preview.tileY = static_cast<int>(std::floor(mouseWorld.y));
        m_Preview.isPlacementValid = canPlace(gs, m_Preview.tileX, m_Preview.tileY);
    }

    bool CampSystem::place(GameState& gs, PlayerInventory& inv, float costMult)
    {
        if (!m_Enabled || !m_Preview.isPlacementValid)
        {
            return false;
        }
        const int cost = std::max(1, static_cast<int>(baseCost(m_Preview.activeType) * costMult));
        if (!inv.removeItem(ITEM_BUILD_MATERIAL, cost))
        {
            return false;
        }

        CampObject obj;
        obj.id = ++m_NextId;
        obj.type = m_Preview.activeType;
        obj.tileX = m_Preview.tileX;
        obj.tileY = m_Preview.tileY;
        obj.health = (obj.type == CampObjectType::ConcreteWall)
                         ? 180.0f
                         : (obj.type == CampObjectType::DefenseTurret ? 90.0f : 60.0f);
        m_Objects.push_back(obj);

        switch (obj.type)
        {
        case CampObjectType::ConcreteWall:
            gs.sectorMap[obj.tileX][obj.tileY] = 1;
            gs.wallDurability[obj.tileX][obj.tileY] = 180;
            break;
        case CampObjectType::SupplyCrate:
        {
            LootContainer c;
            c.position = {obj.tileX + 0.5f, obj.tileY + 0.5f, 0.0f};
            c.type = LootContainerType::WoodenCrate;
            gs.lootContainers.push_back(c);
            break;
        }
        default:
            break;
        }
        return true;
    }

    void CampSystem::updateTurrets(GameState& gs, float dt)
    {
        (void)dt;
        for (const auto& obj : m_Objects)
        {
            if (obj.type != CampObjectType::DefenseTurret)
            {
                continue;
            }
            Vector3D pos{obj.tileX + 0.5f, obj.tileY + 0.5f, 0.0f};
            Enemy* best = nullptr;
            float bestD = 5.5f * 5.5f;
            for (auto& e : gs.enemies)
            {
                if (!e.isAlive)
                {
                    continue;
                }
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
        default:
            break;
        }
        return 5;
    }

    bool CampSystem::canPlace(const GameState& gs, int x, int y) const
    {
        if (x < 0 || y < 0 || x >= Config::MAP_WIDTH || y >= Config::MAP_HEIGHT)
        {
            return false;
        }
        if (gs.sectorMap[x][y] == 1)
        {
            return false;
        }
        if (advDistSq({x + 0.5f, y + 0.5f, 0.0f}, gs.playerPos) > 5.0f * 5.0f)
        {
            return false;
        }
        for (const auto& obj : m_Objects)
        {
            if (obj.tileX == x && obj.tileY == y)
            {
                return false;
            }
        }
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

    const PrefabDef* PrefabLibrary::get(const std::string& name) const
    {
        for (const auto& p : m_Prefabs)
        {
            if (p.name == name)
            {
                return &p;
            }
        }
        return m_Prefabs.empty() ? nullptr : &m_Prefabs.front();
    }

    void ToolGunSystem::cycleMode()
    {
        switch (m_Mode)
        {
        case ToolGunMode::SpawnPrefab:
            m_Mode = ToolGunMode::Delete;
            break;
        case ToolGunMode::Delete:
            m_Mode = ToolGunMode::PaintErosion;
            break;
        case ToolGunMode::PaintErosion:
            m_Mode = ToolGunMode::Validate;
            break;
        case ToolGunMode::Validate:
            m_Mode = ToolGunMode::Export;
            break;
        default:
            m_Mode = ToolGunMode::SpawnPrefab;
            break;
        }
    }

    bool ToolGunSystem::apply(GameState& gs, Vector3D where, const PrefabLibrary& lib)
    {
        const int x = static_cast<int>(std::floor(where.x));
        const int y = static_cast<int>(std::floor(where.y));
        if (x < 0 || y < 0 || x >= Config::MAP_WIDTH || y >= Config::MAP_HEIGHT)
        {
            return false;
        }
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
            bunker::logInfo() << "[TOOLGUN] " << m_LastValidation << std::endl;
            return true;
        case ToolGunMode::Export:
        {
            m_LastExport = exportMap(gs);
            std::ofstream out("saves/exported_map_level.cfg");
            if (out.is_open())
            {
                out << m_LastExport;
                out.close();
                bunker::logInfo() << "[TOOLGUN EXPORT] Карта уровня экспортирована в saves/exported_map_level.cfg!"
                                  << std::endl;
            }
            return true;
        }
        default:
            break;
        }
        return false;
    }

    bool ToolGunSystem::undo(GameState& gs)
    {
        if (m_Undo.empty())
        {
            return false;
        }
        auto action = std::move(m_Undo.back());
        m_Undo.pop_back();
        action.undo(gs);
        m_Redo.push_back(std::move(action));
        return true;
    }

    bool ToolGunSystem::redo(GameState& gs)
    {
        if (m_Redo.empty())
        {
            return false;
        }
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
        {
            m_Undo.erase(m_Undo.begin());
        }
    }

    bool ToolGunSystem::spawnPrefab(GameState& gs, int x, int y, const PrefabLibrary& lib)
    {
        const PrefabDef* prefab = lib.get("small_bunker_room");
        if (!prefab)
        {
            return false;
        }
        auto oldMap = gs.sectorMap;
        auto oldDur = gs.wallDurability;
        for (int row = 0; row < static_cast<int>(prefab->rows.size()); ++row)
        {
            for (int col = 0; col < static_cast<int>(prefab->rows[row].size()); ++col)
            {
                const int tx = x + col;
                const int ty = y + row;
                if (tx < 0 || ty < 0 || tx >= Config::MAP_WIDTH || ty >= Config::MAP_HEIGHT)
                {
                    continue;
                }
                const char c = prefab->rows[row][col];
                switch (c)
                {
                case '#':
                    gs.sectorMap[tx][ty] = 1;
                    gs.wallDurability[tx][ty] = 110;
                    break;
                case '.':
                    gs.sectorMap[tx][ty] = 0;
                    gs.wallDurability[tx][ty] = 0;
                    break;
                case 'c':
                {
                    LootContainer lc;
                    lc.position = {tx + 0.5f, ty + 0.5f, 0.0f};
                    lc.type = LootContainerType::WoodenCrate;
                    gs.lootContainers.push_back(lc);
                    break;
                }
                default:
                    break;
                }
            }
        }
        auto newMap = gs.sectorMap;
        auto newDur = gs.wallDurability;
        pushAction({"spawn prefab",
                    [oldMap, oldDur](GameState& s)
                    {
                        s.sectorMap = oldMap;
                        s.wallDurability = oldDur;
                    },
                    [newMap, newDur](GameState& s)
                    {
                        s.sectorMap = newMap;
                        s.wallDurability = newDur;
                    }});
        return true;
    }

    bool ToolGunSystem::deleteTile(GameState& gs, int x, int y)
    {
        const int oldTile = gs.sectorMap[x][y];
        const int oldDur = gs.wallDurability[x][y];
        gs.sectorMap[x][y] = 0;
        gs.wallDurability[x][y] = 0;
        pushAction({"delete tile",
                    [x, y, oldTile, oldDur](GameState& s)
                    {
                        s.sectorMap[x][y] = oldTile;
                        s.wallDurability[x][y] = oldDur;
                    },
                    [x, y](GameState& s)
                    {
                        s.sectorMap[x][y] = 0;
                        s.wallDurability[x][y] = 0;
                    }});
        return true;
    }

    bool ToolGunSystem::paintErosion(GameState& gs, int x, int y)
    {
        const float old = gs.etherErosionMap[x][y];
        gs.etherErosionMap[x][y] = advClamp(old + 10.0f, 0.0f, 100.0f);
        const float now = gs.etherErosionMap[x][y];
        pushAction({"paint erosion", [x, y, old](GameState& s) { s.etherErosionMap[x][y] = old; },
                    [x, y, now](GameState& s) { s.etherErosionMap[x][y] = now; }});
        return true;
    }

    std::string ToolGunSystem::validate(const GameState& gs) const
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
                    {
                        ++brokenWalls;
                    }
                }
            }
        }
        for (const auto& e : gs.enemies)
        {
            const int ex = static_cast<int>(e.position.x);
            const int ey = static_cast<int>(e.position.y);
            if (ex < 0 || ey < 0 || ex >= Config::MAP_WIDTH || ey >= Config::MAP_HEIGHT || gs.sectorMap[ex][ey] == 1)
            {
                ++invalidEnemies;
            }
        }

        bool playerBlocked = false;
        int px = static_cast<int>(gs.playerPos.x);
        int py = static_cast<int>(gs.playerPos.y);
        if (px >= 0 && px < Config::MAP_WIDTH && py >= 0 && py < Config::MAP_HEIGHT)
        {
            playerBlocked = (gs.sectorMap[px][py] == 1);
        }

        std::ostringstream out;
        out << "VALIDATION: walls=" << walls << " brokenDurability=" << brokenWalls
            << " invalidEnemies=" << invalidEnemies
            << (playerBlocked ? " [CRITICAL: PLAYER STUCK IN WALL!]" : " [GRID OK]");
        return out.str();
    }

    std::string ToolGunSystem::exportMap(const GameState& gs) const
    {
        std::ostringstream out;
        out << "# Bunker Protocol ISO map export\n";
        for (int y = 0; y < Config::MAP_HEIGHT; ++y)
        {
            for (int x = 0; x < Config::MAP_WIDTH; ++x)
            {
                out << (gs.sectorMap[x][y] == 1 ? '#' : '.');
            }
            out << '\n';
        }
        return out.str();
    }

    // ═══════════════════════════════════════════════════════════════════════════════

} // namespace bunker
