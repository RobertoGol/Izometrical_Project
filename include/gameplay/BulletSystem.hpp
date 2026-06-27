#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "core/IsoMath.hpp"
#include "core/Constants.hpp"
#include <SFML/Graphics.hpp>

namespace bunker
{

    struct AdvancedMechanics;

    class BulletSystem
    {
    public:
        BulletSystem() = default;

        void update(GameState &gs, float dt);
        void update(GameState &gs, float dt, AdvancedMechanics *adv);
        void fireScoutWeapon(GameState &gs, bool isAiming);
        void fireScoutWeapon(GameState &gs, bool isAiming, AdvancedMechanics *adv);
        void fireTitanWeapon(GameState &gs);
        void fireTitanWeapon(GameState &gs, AdvancedMechanics *adv);
        void fireTitanMissiles(GameState &gs);
        void fireTitanMissiles(GameState &gs, AdvancedMechanics *adv);
        void processSplashDamage(GameState &gs, const Bullet &b, AdvancedMechanics *adv);
        void fireDebugGunChainLightning(GameState &gs);
        void render(const GameState &gs, sf::RenderWindow &window, const sf::View &view) const;

    private:
        static bool isExplosive(const Bullet &b);
        static float random01();
        static float randomSpread(float amount);
        static Vector3D normalizeDir(Vector3D dir);
    };

} // namespace bunker
