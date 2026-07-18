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

} // namespace bunker
