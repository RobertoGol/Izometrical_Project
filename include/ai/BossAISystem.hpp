#pragma once

#include "core/Types.hpp"
#include "gameplay/GameState.hpp"
#include "core/Constants.hpp"
#include <vector>
#include <string>
#include <cmath>
#include <iostream>

namespace bunker
{

    enum class BossArchetype
    {
        RoySwarmPrime,   // 100% МЕХАНИЧЕСКАЯ самореплицирующаяся боевая шестерёнка роя ROY
        RayOverlordMech, // Тяжёлый артиллерийский шагоход RAY
        OverseerCyborg   // Киборг-надзиратель Убежища 17
    };

    enum class BossPhase
    {
        Phase1_TacticalCover,         // Тактический отстрел из-за бастионов
        Phase2_MechanicalReplication, // Спавн механических дронов ROY в засаду
        Phase3_SteamOverdrive         // Яростный перегрев сбросом пара >= 250°C
    };

    struct BossEntity
    {
        int enemyIndex = -1;
        BossArchetype archetype = BossArchetype::RoySwarmPrime;
        BossPhase currentPhase = BossPhase::Phase1_TacticalCover;
        float tacticalShieldHp = 300.0f;
        float maxShieldHp = 300.0f;
        float phaseTimer = 0.0f;
        float replicationCooldown = 5.0f;
        float ventingCooldown = 4.0f;
        Vector3D anchorCoverPos = {12.0f, 12.0f, 0.0f};
        bool isEnraged = false;
    };

    class BossAISystem
    {
    public:
        BossAISystem() = default;

        void registerBoss(int enemyIndex, BossArchetype arch, Vector3D coverPos);
        void updateBosses(GameState &gs, float dt);
        void onBossDamaged(GameState &gs, int enemyIndex, float dmg);
        const std::vector<BossEntity> &activeBosses() const { return m_Bosses; }

    private:
        std::vector<BossEntity> m_Bosses;

        void executeCoverTactics(GameState &gs, BossEntity &boss, Enemy &e, float dt);
        void executeSwarmAmbush(GameState &gs, BossEntity &boss, const Vector3D &targetPos);
        void triggerEmergencyVentingAOE(GameState &gs, BossEntity &boss, Enemy &e);
    };

} // namespace bunker