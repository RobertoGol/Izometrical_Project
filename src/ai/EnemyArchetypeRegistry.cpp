#include "ai/EnemyArchetypeRegistry.hpp"
#include "core/Constants.hpp"

namespace bunker
{

    std::vector<HostileProfile> EnemyArchetypeRegistry::s_Profiles;

    void EnemyArchetypeRegistry::initializeRegistry()
    {
        s_Profiles.clear();

        // 1. Биомасса Роя (Vermin Rush / Органический рой RoY)
        HostileProfile vermin;
        vermin.kind = HostileKind::VerminRush;
        vermin.archetype = "Vermin Rush (RoY Swarm)";
        vermin.speed = Config::ENEMY_WALK_SPEED * 1.35f;
        vermin.detectRadius = 5.5f;
        vermin.loseRadius = 8.0f;
        vermin.aggroThreshold = 60.0f;
        vermin.awarenessGain = 65.0f;
        vermin.attackRange = 0.95f;
        vermin.attackCooldown = 0.80f;
        vermin.damage = 6.0f;
        vermin.swarmRush = true;
        s_Profiles.push_back(vermin);

        // 2. Бешеные Гули (Ghoul Rush)
        HostileProfile ghoul;
        ghoul.kind = HostileKind::GhoulRush;
        ghoul.archetype = "Feral Ghoul Rush";
        ghoul.speed = Config::ENEMY_WALK_SPEED * 1.15f;
        ghoul.detectRadius = 6.5f;
        ghoul.loseRadius = 10.0f;
        ghoul.aggroThreshold = 80.0f;
        ghoul.awarenessGain = 50.0f;
        ghoul.attackRange = 1.20f;
        ghoul.attackCooldown = 1.10f;
        ghoul.damage = 14.0f;
        s_Profiles.push_back(ghoul);

        // 3. Тактические мутанты (Human Tactical / SuperMutants)
        HostileProfile tactical;
        tactical.kind = HostileKind::HumanTactical;
        tactical.archetype = "Tactical SuperMutant Vanguard";
        tactical.speed = Config::ENEMY_WALK_SPEED * 0.90f;
        tactical.detectRadius = 8.5f;
        tactical.loseRadius = 12.0f;
        tactical.aggroThreshold = 100.0f;
        tactical.awarenessGain = 40.0f;
        tactical.attackRange = 6.50f;
        tactical.attackCooldown = 1.80f;
        tactical.damage = 18.0f;
        tactical.usesCover = true;
        s_Profiles.push_back(tactical);

        // 4. Тяжёлые роботы-боссы (Robot Control / Вражеский Аналог Титана)
        HostileProfile robot;
        robot.kind = HostileKind::RobotControl;
        robot.archetype = "Hostile Titan Mech Combatant";
        robot.speed = Config::ENEMY_WALK_SPEED * 0.70f;
        robot.detectRadius = 9.0f;
        robot.loseRadius = 14.0f;
        robot.aggroThreshold = 100.0f;
        robot.awarenessGain = 35.0f;
        robot.attackRange = 7.50f;
        robot.attackCooldown = 2.20f;
        robot.damage = 32.0f;
        robot.partCrippling = true;
        s_Profiles.push_back(robot);
    }

    const HostileProfile &EnemyArchetypeRegistry::getProfile(HostileKind kind)
    {
        if (s_Profiles.empty())
        {
            initializeRegistry();
        }
        for (const auto &p : s_Profiles)
        {
            if (p.kind == kind)
                return p;
        }
        return s_Profiles[0];
    }

} // namespace bunker