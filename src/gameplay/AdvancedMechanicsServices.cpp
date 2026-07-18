#include "gameplay/AdvancedMechanics.hpp"
#include <iomanip>
#include <sstream>

namespace bunker
{

    // PROFILE / SESSION MIGRATION

    void ProfileSessionSystem::startSession(PlayerProfile& p)
    {
        migrate(p);
        p.sessionsPlayed += 1;
        m_SessionTime = 0.0f;
        m_Started = true;
    }

    void ProfileSessionSystem::update(PlayerProfile& p, float dt)
    {
        if (!m_Started)
        {
            return;
        }
        m_SessionTime += dt;
        p.totalPlayTime += dt;
    }

    void ProfileSessionSystem::migrate(PlayerProfile& p)
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

    std::string ProfileSessionSystem::summary(const PlayerProfile& p) const
    {
        std::ostringstream ss;
        ss << "PROFILE v" << p.version << " name=" << p.playerName << " kills=" << p.totalKills
           << " deaths=" << p.totalDeaths << " sessions=" << p.sessionsPlayed << " playtime=" << std::fixed
           << std::setprecision(1) << p.totalPlayTime;
        return ss.str();
    }

    // ═══════════════════════════════════════════════════════════════════════════════

} // namespace bunker
