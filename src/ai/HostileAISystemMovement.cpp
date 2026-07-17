#include "ai/HostileAISystem.hpp"
#include "ai/EnemyArchetypeRegistry.hpp"
#include "ai/PerceptionSystem.hpp"
#include "engine/Log.hpp"
#include "gameplay/DamageSystem.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace bunker
{

    Vector3D HostileAISystem::chooseTarget(const GameState& gs, const Enemy& e) const
    {
        if (gs.playerMode == UnitMode::Titan || gs.titan.isPiloted)
        {
            return gs.titan.position;
        }

        float pdx = gs.playerPos.x - e.position.x;
        float pdy = gs.playerPos.y - e.position.y;
        float tdx = gs.titan.position.x - e.position.x;
        float tdy = gs.titan.position.y - e.position.y;
        float playerDistSq = pdx * pdx + pdy * pdy;
        float titanDistSq = tdx * tdx + tdy * tdy;

        if (gs.titan.health > 0.0f && titanDistSq + 4.0f < playerDistSq)
        {
            return gs.titan.position;
        }

        return gs.playerPos;
    }

    void HostileAISystem::moveToward(GameState& gs, Enemy& e, const Vector3D& target, float speed, float dt) const
    {
        float dx = target.x - e.position.x;
        float dy = target.y - e.position.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f)
            return;
        tryMove(gs, e, dx / len, dy / len, speed, dt);
    }

    void HostileAISystem::moveAway(GameState& gs, Enemy& e, const Vector3D& target, float speed, float dt) const
    {
        float dx = e.position.x - target.x;
        float dy = e.position.y - target.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f)
            return;
        tryMove(gs, e, dx / len, dy / len, speed, dt);
    }

    void HostileAISystem::strafeAround(GameState& gs, Enemy& e, const Vector3D& target, float speed, float sign,
                                       float dt) const
    {
        float dx = target.x - e.position.x;
        float dy = target.y - e.position.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.001f)
            return;

        float nx = dx / len;
        float ny = dy / len;

        tryMove(gs, e, -ny * sign, nx * sign, speed, dt);
    }

    void HostileAISystem::tryMove(GameState& gs, Enemy& e, float dirX, float dirY, float speed, float dt) const
    {
        float nx = e.position.x + dirX * speed * dt;
        float ny = e.position.y + dirY * speed * dt;

        if (!Collisions::checkWorldCollision(gs, nx, ny, e.radius))
        {
            e.position.x = nx;
            e.position.y = ny;
            return;
        }

        if (!Collisions::checkWorldCollision(gs, nx, e.position.y, e.radius))
        {
            e.position.x = nx;
        }
        if (!Collisions::checkWorldCollision(gs, e.position.x, ny, e.radius))
        {
            e.position.y = ny;
        }
    }

    float HostileAISystem::random01()
    {
        return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }


} // namespace bunker
