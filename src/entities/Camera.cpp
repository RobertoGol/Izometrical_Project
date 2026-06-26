#include "entities/Collisions.hpp"
#include "gameplay/DamageSystem.hpp"
#include "core/Constants.hpp"
#include <algorithm>
#include <cmath>

namespace bunker {

bool Collisions::checkWorldCollision(const GameState& gs, float nextX, float nextY, float radius) {
    int checkPoints[4][2] = {
        { static_cast<int>(nextX - radius), static_cast<int>(nextY - radius) },
        { static_cast<int>(nextX + radius), static_cast<int>(nextY - radius) },
        { static_cast<int>(nextX - radius), static_cast<int>(nextY + radius) },
        { static_cast<int>(nextX + radius), static_cast<int>(nextY + radius) }
    };

    for (int i = 0; i < 4; ++i) {
        int tx = checkPoints[i][0];
        int ty = checkPoints[i][1];
        if (tx < 0 || tx >= Config::MAP_WIDTH || ty < 0 || ty >= Config::MAP_HEIGHT)
            return true;
        if (gs.sectorMap[tx][ty] == 1)
            return true;
    }
    return false;
}

float Collisions::getMassWeight(PhysicalMassClass massClass) {
    switch (massClass) {
        case PhysicalMassClass::StaticWall:    return 1000000.0f;
        case PhysicalMassClass::VanguardTitan: return 40000.0f;
        case PhysicalMassClass::HumanPilot:    return 100.0f;
        case PhysicalMassClass::VerminSwarm:   return 15.0f;
        default: return 1.0f;
    }
}

bool Collisions::resolveCircleVsCircle(CollisionBody& bodyA, CollisionBody& bodyB) {
    float dx = bodyB.position->x - bodyA.position->x;
    float dy = bodyB.position->y - bodyA.position->y;

    float distanceSq = dx * dx + dy * dy;
    float minDist = bodyA.radius + bodyB.radius;
    float minDistSq = minDist * minDist;

    if (distanceSq >= minDistSq || distanceSq < 0.0001f)
        return false;

    float distance = std::sqrt(distanceSq);
    float overlap = minDist - distance;

    float nx = dx / distance;
    float ny = dy / distance;

    float massA = getMassWeight(bodyA.massClass);
    float massB = getMassWeight(bodyB.massClass);
    float totalMass = massA + massB;

    float pushFactorA = massB / totalMass;
    float pushFactorB = massA / totalMass;

    bodyA.position->x -= nx * overlap * pushFactorA;
    bodyA.position->y -= ny * overlap * pushFactorA;

    bodyB.position->x += nx * overlap * pushFactorB;
    bodyB.position->y += ny * overlap * pushFactorB;

    return true;
}

bool Collisions::resolveCircleVsTileWall(Vector3D& objPos, float radius, int tileX, int tileY) {
    float minX = static_cast<float>(tileX);
    float maxX = static_cast<float>(tileX) + 1.0f;
    float minY = static_cast<float>(tileY);
    float maxY = static_cast<float>(tileY) + 1.0f;

    float closestX = std::clamp(objPos.x, minX, maxX);
    float closestY = std::clamp(objPos.y, minY, maxY);

    float dx = objPos.x - closestX;
    float dy = objPos.y - closestY;
    float distanceSq = dx * dx + dy * dy;

    if (distanceSq >= radius * radius || distanceSq < 0.0001f)
        return false;

    float distance = std::sqrt(distanceSq);
    float overlap = radius - distance;

    float nx = dx / distance;
    float ny = dy / distance;

    objPos.x += nx * overlap;
    objPos.y += ny * overlap;

    return true;
}

void Collisions::resolveAllCollisions(GameState& gs) {
    // 1. Враги между собой
    for (size_t i = 0; i < gs.enemies.size(); ++i) {
        if (!gs.enemies[i].isAlive) continue;

        for (size_t j = i + 1; j < gs.enemies.size(); ++j) {
            if (!gs.enemies[j].isAlive) continue;

            CollisionBody bodyA = { &gs.enemies[i].position, gs.enemies[i].radius, PhysicalMassClass::VerminSwarm };
            CollisionBody bodyB = { &gs.enemies[j].position, gs.enemies[j].radius, PhysicalMassClass::VerminSwarm };
            resolveCircleVsCircle(bodyA, bodyB);
        }

        // 2. Враг vs Игрок/Титан
        PhysicalMassClass playerMass = (gs.playerMode == UnitMode::Titan)
            ? PhysicalMassClass::VanguardTitan
            : PhysicalMassClass::HumanPilot;
        float pRadius = (gs.playerMode == UnitMode::Titan) ? Config::TITAN_RADIUS : Config::PLAYER_RADIUS;

        CollisionBody enemyBody = { &gs.enemies[i].position, gs.enemies[i].radius, PhysicalMassClass::VerminSwarm };
        CollisionBody playerBody = { &gs.playerPos, pRadius, playerMass };

        if (resolveCircleVsCircle(playerBody, enemyBody)) {
            if (gs.playerMode == UnitMode::Scout) {
                DamageSystem::applyPlayerDamage(gs, 2.0f * gs.deltaTime, DamageType::Contact);
            }
        }

        // 3. Враги vs стены
        int ex = static_cast<int>(gs.enemies[i].position.x);
        int ey = static_cast<int>(gs.enemies[i].position.y);

        for (int nx = ex - 1; nx <= ex + 1; ++nx) {
            for (int ny = ey - 1; ny <= ey + 1; ++ny) {
                if (nx >= 0 && nx < Config::MAP_WIDTH && ny >= 0 && ny < Config::MAP_HEIGHT) {
                    if (gs.sectorMap[nx][ny] == 1) {
                        resolveCircleVsTileWall(gs.enemies[i].position, gs.enemies[i].radius, nx, ny);
                    }
                }
            }
        }
    }
}

}  // namespace bunker
