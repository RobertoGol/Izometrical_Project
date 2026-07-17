#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "core/Constants.hpp"

namespace bunker
{

    enum class PhysicalMassClass
    {
        StaticWall,
        VanguardTitan,
        HumanPilot,
        VerminSwarm
    };

    struct CollisionBody
    {
        Vector3D *position;
        float radius;
        PhysicalMassClass massClass;
    };

    class Collisions
    {
    public:
        static bool checkWorldCollision(const GameState &gs, float nextX, float nextY, float radius);
        static bool resolveCircleVsCircle(CollisionBody &bodyA, CollisionBody &bodyB);
        static bool resolveCircleVsTileWall(Vector3D &objPos, float radius, int tileX, int tileY);
        static void resolveAllCollisions(GameState &gs);

    private:
        static float getMassWeight(PhysicalMassClass massClass);
    };

} // namespace bunker