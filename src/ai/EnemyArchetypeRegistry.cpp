#include <SFML/Graphics.hpp>
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
        vermin.detectRadius = 5.5f;
        vermin.loseRadius = 8.0f;
        vermin.attackRadius = 0.55f;
        vermin.preferredRange = 0.45f;
        vermin.speed = 3.15f;
        vermin.damage = 4.0f;
        vermin.attackCooldown = 0.55f;
        vermin.awarenessGain = 75.0f;
        vermin.awarenessDecay = 26.0f;
        vermin.bodyColor = sf::Color(210, 45, 45);
        vermin.shapePoints = 3;
        vermin.textureAssetPath = "assets/enemies/vermin_swarm.png";
        s_Profiles.push_back(vermin);

        // 2. Бешеные Гули (Ghoul Rush)
        HostileProfile ghoul;
        ghoul.kind = HostileKind::GhoulRush;
        ghoul.detectRadius = 6.5f;
        ghoul.loseRadius = 10.0f;
        ghoul.attackRadius = 0.75f;
        ghoul.preferredRange = 0.60f;
        ghoul.speed = 2.35f;
        ghoul.damage = 10.0f;
        ghoul.attackCooldown = 1.05f;
        ghoul.awarenessGain = 58.0f;
        ghoul.awarenessDecay = 16.0f;
        ghoul.bodyColor = sf::Color(150, 230, 80);
        ghoul.shapePoints = 8;
        ghoul.textureAssetPath = "assets/enemies/feral_ghoul.png";
        s_Profiles.push_back(ghoul);

        // 3. Тактические мутанты (Human Tactical / SuperMutants)
        HostileProfile tactical;
        tactical.kind = HostileKind::HumanTactical;
        tactical.detectRadius = 8.5f;
        tactical.loseRadius = 12.0f;
        tactical.attackRadius = 6.0f;
        tactical.preferredRange = 4.5f;
        tactical.speed = 2.15f;
        tactical.damage = 8.0f;
        tactical.attackCooldown = 1.15f;
        tactical.awarenessGain = 48.0f;
        tactical.awarenessDecay = 14.0f;
        tactical.usesRangedAttack = true;
        tactical.bodyColor = sf::Color(235, 170, 70);
        tactical.shapePoints = 4;
        tactical.textureAssetPath = "assets/enemies/supermutant.png";
        s_Profiles.push_back(tactical);

        // 4. Тяжёлые роботы-боссы (Robot Control / Вражеский Аналог Титана)
        HostileProfile robot;
        robot.kind = HostileKind::RobotControl;
        robot.detectRadius = 9.0f;
        robot.loseRadius = 13.5f;
        robot.attackRadius = 5.5f;
        robot.preferredRange = 4.0f;
        robot.speed = 1.85f;
        robot.damage = 12.0f;
        robot.attackCooldown = 1.35f;
        robot.awarenessGain = 62.0f;
        robot.awarenessDecay = 10.0f;
        robot.isMechanical = true;
        robot.usesRangedAttack = true;
        robot.bodyColor = sf::Color(135, 170, 230);
        robot.shapePoints = 6;
        robot.textureAssetPath = "assets/enemies/hostile_titan_mech.png";
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