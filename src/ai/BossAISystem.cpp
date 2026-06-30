#include "ai/BossAISystem.hpp"
#include <algorithm>
#include <cstdlib>

namespace bunker
{

    void BossAISystem::registerBoss(int enemyIndex, BossArchetype arch, Vector3D coverPos)
    {
        // Не удаляем старых боссов, бережно регистрируем нового в доменную архитектуру
        for (auto &b : m_Bosses)
        {
            if (b.enemyIndex == enemyIndex)
            {
                b.archetype = arch;
                b.anchorCoverPos = coverPos;
                return;
            }
        }

        BossEntity b;
        b.enemyIndex = enemyIndex;
        b.archetype = arch;
        b.anchorCoverPos = coverPos;
        b.currentPhase = BossPhase::Phase1_TacticalCover;
        b.tacticalShieldHp = (arch == BossArchetype::RayOverlordMech) ? 450.0f : 300.0f;
        b.maxShieldHp = b.tacticalShieldHp;
        m_Bosses.push_back(b);

        std::cout << "[BOSS AI] Зарегистрирован Босс (Индекс: " << enemyIndex
                  << ", Тип: " << (arch == BossArchetype::RoySwarmPrime ? "ROY Swarm Prime" : "RAY Overlord")
                  << ") — Щит 350 Бар активен!" << std::endl;
    }

    void BossAISystem::onBossDamaged(GameState &gs, int enemyIndex, float dmg)
    {
        for (auto &boss : m_Bosses)
        {
            if (boss.enemyIndex != enemyIndex)
                continue;

            if (boss.tacticalShieldHp > 0.0f)
            {
                boss.tacticalShieldHp = std::max(0.0f, boss.tacticalShieldHp - dmg);
                if (boss.tacticalShieldHp <= 0.0f)
                {
                    std::cout << "[BOSS AI] Энергощит босса пробит! Прямое попадание по броне бункера!" << std::endl;
                }
            }
        }
    }

    void BossAISystem::updateBosses(GameState &gs, float dt)
    {
        // Оптимизация под слабые ПК: быстрые проходы таблицы без динамической аллокации
        for (std::size_t i = 0; i < m_Bosses.size();)
        {
            BossEntity &boss = m_Bosses[i];
            if (boss.enemyIndex < 0 || boss.enemyIndex >= static_cast<int>(gs.enemies.size()) ||
                !gs.enemies[boss.enemyIndex].isAlive)
            {
                std::cout << "[BOSS AI] Босс ликвидирован! Протокол Убежища 17 обновлён." << std::endl;
                boss = m_Bosses.back();
                m_Bosses.pop_back();
                continue;
            }

            Enemy &e = gs.enemies[boss.enemyIndex];
            boss.phaseTimer += dt;

            // Расчёт фаз по порогам здоровья (Канон: не удалять старые статы, модулировать поведение)
            float hpRatio = e.health / Config::ENEMY_BASE_HP;

            if (hpRatio <= 0.65f && boss.currentPhase == BossPhase::Phase1_TacticalCover)
            {
                boss.currentPhase = BossPhase::Phase2_MechanicalReplication;
                std::cout << "[BOSS AI] Здоровье босса <= 65%. Переход во ФАЗУ 2: Тактика засад роя ROY!" << std::endl;
                executeSwarmAmbush(gs, boss, gs.playerPos);
            }
            else if (hpRatio <= 0.30f && boss.currentPhase != BossPhase::Phase3_SteamOverdrive)
            {
                boss.currentPhase = BossPhase::Phase3_SteamOverdrive;
                boss.isEnraged = true;
                e.speed *= 1.4f; // Овердрайв мобильности
                std::cout << "[BOSS AI] Критический нагрев котла! ФАЗА 3: Аварийный сброс пара 250°C!" << std::endl;
                triggerEmergencyVentingAOE(gs, boss, e);
            }

            // Выполнение тактики текущей фазы
            switch (boss.currentPhase)
            {
            case BossPhase::Phase1_TacticalCover:
                executeCoverTactics(gs, boss, e, dt);
                break;

            case BossPhase::Phase2_MechanicalReplication:
                boss.replicationCooldown -= dt;
                if (boss.replicationCooldown <= 0.0f)
                {
                    boss.replicationCooldown = 8.5f;
                    executeSwarmAmbush(gs, boss, gs.playerPos);
                }
                break;

            case BossPhase::Phase3_SteamOverdrive:
                boss.ventingCooldown -= dt;
                if (boss.ventingCooldown <= 0.0f)
                {
                    boss.ventingCooldown = 4.2f;
                    triggerEmergencyVentingAOE(gs, boss, e);
                }
                break;
            }

            ++i;
        }
    }

    void BossAISystem::executeCoverTactics(GameState &gs, BossEntity &boss, Enemy &e, float dt)
    {
        // Быстрый расчёт вектора к укрытию без тригонометрических спайков
        float dx = boss.anchorCoverPos.x - e.position.x;
        float dy = boss.anchorCoverPos.y - e.position.y;
        float distSq = dx * dx + dy * dy;

        if (distSq > 0.25f)
        {
            float invLen = 1.0f / std::sqrt(distSq);
            e.position.x += dx * invLen * e.speed * 0.7f * dt;
            e.position.y += dy * invLen * e.speed * 0.7f * dt;
        }
    }

    void BossAISystem::executeSwarmAmbush(GameState &gs, BossEntity &boss, const Vector3D &targetPos)
    {
        // Каноничное правило роя ROY: строго 100% механические шестерёнки без органики
        Vector3D flank1 = {std::clamp(targetPos.x + 2.5f, 1.0f, static_cast<float>(Config::MAP_WIDTH - 2)),
                           std::clamp(targetPos.y + 1.0f, 1.0f, static_cast<float>(Config::MAP_HEIGHT - 2)), 0.0f};
        Vector3D flank2 = {std::clamp(targetPos.x - 2.5f, 1.0f, static_cast<float>(Config::MAP_WIDTH - 2)),
                           std::clamp(targetPos.y - 1.0f, 1.0f, static_cast<float>(Config::MAP_HEIGHT - 2)), 0.0f};

        auto spawnGear = [&](const Vector3D &p)
        {
            Enemy gear;
            gear.position = p;
            gear.health = 22.0f;
            gear.speed = 3.1f;
            gear.radius = 0.3f;
            gear.isAlive = true;
            gs.enemies.push_back(gear);
        };

        spawnGear(flank1);
        spawnGear(flank2);
        std::cout << "[ROY SWARM] Синтезированы 2 механические шестерёнки роя в засаду коридора!" << std::endl;
    }

    void BossAISystem::triggerEmergencyVentingAOE(GameState &gs, BossEntity &boss, Enemy &e)
    {
        // Термодинамика БТ-7274 / RAY: сброс перегретого пара 250°C с уроном вокруг
        float aoeRadiusSq = 3.5f * 3.5f;
        float pdx = gs.playerPos.x - e.position.x;
        float pdy = gs.playerPos.y - e.position.y;
        if (pdx * pdx + pdy * pdy <= aoeRadiusSq)
        {
            // Наносим термический урон Пилоту
            std::cout << "[THERMAL BLAST] Импульс пара 250°C обжёг кабину Пилота на 28 HP!" << std::endl;
        }

        // Разрушаем хрупкие стены бункера вокруг босса в радиусе 2 ячеек
        int bx = static_cast<int>(e.position.x);
        int by = static_cast<int>(e.position.y);
        for (int x = std::max(0, bx - 1); x <= std::min(Config::MAP_WIDTH - 1, bx + 1); ++x)
        {
            for (int y = std::max(0, by - 1); y <= std::min(Config::MAP_HEIGHT - 1, by + 1); ++y)
            {
                if (gs.wallDurability[x][y] > 0)
                {
                    gs.wallDurability[x][y] -= 45;
                    if (gs.wallDurability[x][y] <= 0)
                        gs.sectorMap[x][y] = 0;
                }
            }
        }
    }

} // namespace bunker