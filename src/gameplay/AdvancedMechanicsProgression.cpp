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


} // namespace bunker
