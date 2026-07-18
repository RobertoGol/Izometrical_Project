#pragma once

#include "gameplay/advanced/AdvancedMechanicsSurvivalSystems.hpp"
#include "gameplay/LootManager.hpp"
namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 6) STORY ROUTE / ZONE EVENTS
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class ZoneEventId
    {
        CryoLocker,
        Archive,
        Garage,
        Exterior,
        FirstCombat,
        ReturnToBase,
        SurfaceClearance
    };

    struct ZoneTrigger
    {
        ZoneEventId id = ZoneEventId::CryoLocker;
        Vector3D center;
        float radius = 1.0f;
        bool fired = false;
        std::string objective;
    };

    class StoryRouteSystem
    {
      public:
        StoryRouteSystem();
        void update(GameState& gs, RadioTapeSystem* radio = nullptr);

        const std::vector<StoryRouteEntry>& route() const
        {
            return m_Route;
        }
        const std::string& lastEvent() const
        {
            return m_LastEvent;
        }

      private:
        std::vector<ZoneTrigger> m_Triggers;
        std::vector<StoryRouteEntry> m_Route;
        std::string m_LastEvent;

        void rebuildRoute();
        void mark(const std::string& text);
        void fire(GameState& gs, ZoneTrigger& t, RadioTapeSystem* radio);
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 7) SKILLS / XP
    // ═══════════════════════════════════════════════════════════════════════════════

    enum class SkillId
    {
        ArchiveSync,
        FootKill,
        TankAction,
        StressSurvival,
        Mechanic,
        Builder
    };

    struct SkillRuntime
    {
        std::map<SkillId, int> rank;
        int pendingKillsFoot = 0;
        int pendingKillsTank = 0;
    };

    class SkillSystem
    {
      public:
        SkillSystem();
        void grantXp(GameState& gs, int xp);
        bool upgrade(GameState& gs, SkillId id);

        float footDamageMultiplier() const
        {
            return 1.0f + 0.08f * rank(SkillId::FootKill);
        }
        float tankDamageMultiplier() const
        {
            return 1.0f + 0.10f * rank(SkillId::TankAction);
        }
        float repairMultiplier() const
        {
            return 1.0f + 0.12f * rank(SkillId::Mechanic);
        }
        float buildCostMultiplier() const
        {
            return 1.0f - 0.07f * rank(SkillId::Builder);
        }
        int rank(SkillId id) const
        {
            auto it = m_Skills.rank.find(id);
            return it == m_Skills.rank.end() ? 0 : it->second;
        }
        const SkillRuntime& runtime() const
        {
            return m_Skills;
        }

      private:
        SkillRuntime m_Skills;

        static int xpForNext(int level)
        {
            return 250 + level * 125;
        }
        void applyPassive(GameState& gs, SkillId id, int r);
    };

    // ═══════════════════════════════════════════════════════════════════════════════
    // 8) LOOT GENERATOR BY TIERS
    // ═══════════════════════════════════════════════════════════════════════════════

} // namespace bunker
