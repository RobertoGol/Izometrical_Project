#include "persistence/StoryFlagRegistry.hpp"
#include <iostream>

namespace bunker
{

    StoryFlagRegistry::StoryFlagRegistry()
    {
        resetAll();
    }

    void StoryFlagRegistry::setFlag(StoryQuestFlag flag, bool value)
    {
        m_Flags[flag] = value;
        if (value)
        {
            std::cout << "[STORY REGISTRY] Сюжетный флаг активирован: квестовый этап пройден!" << std::endl;
        }
    }

    bool StoryFlagRegistry::getFlag(StoryQuestFlag flag) const
    {
        auto it = m_Flags.find(flag);
        if (it != m_Flags.end())
        {
            return it->second;
        }
        return false;
    }

    void StoryFlagRegistry::resetAll()
    {
        m_Flags.clear();
        m_Flags[StoryQuestFlag::FoundPipPad] = false;
        m_Flags[StoryQuestFlag::EnteredCryoLocker] = false;
        m_Flags[StoryQuestFlag::UnlockedArchiveSync] = false;
        m_Flags[StoryQuestFlag::GarageRepairUnlocked] = false;
        m_Flags[StoryQuestFlag::RegionalTowerRestored] = false;
        m_Flags[StoryQuestFlag::DefeatedHeavyMechBoss] = false;
    }

} // namespace bunker
