#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "physics/PhysicsWorld.hpp"

namespace bunker
{
    struct CollisionProbe
    {
        Vector3D position{};
        float radius = 0.0f;
        float height = 1.8f;
    };

    struct CollisionMoveResult
    {
        Vector3D position{};
        Vector3D attemptedDelta{};
        Vector3D appliedDelta{};
        bool hitWall = false;
        bool hitCeiling = false;
        bool onGround = true;
    };

    class CollisionSystem
    {
      public:
        static bool checkWorldCollision(const GameState& gameState, float x, float y, float radius);
        static CollisionMoveResult sweepAndSlide(const PhysicsWorld& world,
                                                 const CollisionProbe& probe,
                                                 const Vector3D& desiredDelta);
        static bool isOnGround(const PhysicsWorld& world, const CollisionProbe& probe);
        static bool hasCeilingContact(const PhysicsWorld& world, const CollisionProbe& probe);
        static void resolveDynamicCollisions(GameState& gameState);
    };
} // namespace bunker
