#pragma once

#include <unordered_map>
#include <string>

namespace bunker
{

    enum class StoryQuestFlag
    {
        FoundPipPad,
        EnteredCryoLocker,
        UnlockedArchiveSync,
        GarageRepairUnlocked,
        RegionalTowerRestored,
        DefeatedHeavyMechBoss
    };

    // ═══════════════════════════════════════════════════════════════════════
    // Реестр квестовых флагов Убежища 17 (Волна 6)
    // Управляет разблокировкой механик ремонта и прогрессией зон
    // ═══════════════════════════════════════════════════════════════════════

    class StoryFlagRegistry
    {
    private:
        std::unordered_map<StoryQuestFlag, bool> m_Flags;

    public:
        StoryFlagRegistry();

        void setFlag(StoryQuestFlag flag, bool value = true);
        bool getFlag(StoryQuestFlag flag) const;
        void resetAll();

        const std::unordered_map<StoryQuestFlag, bool> &allFlags() const { return m_Flags; }
    };

} // namespace bunker