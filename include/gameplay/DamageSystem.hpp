#pragma once

#include "core/Types.hpp"
#include "core/Constants.hpp"
#include "core/IsoMath.hpp"
#include "gameplay/GameState.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace bunker
{

    // ═══════════════════════════════════════════════════════
    // Классификация урона движка Bunker Protocol ISO
    // Объединяет механики из GMyGameDoNotTouch и Video_Game_Izom
    // ═══════════════════════════════════════════════════════
    enum class DamageType
    {
        Kinetic,       // Стандартные пули (XO-16, карабины)
        Explosive,     // Взрывы (миномёт Скорча, ракеты, бочки)
        Thermal,       // Огонь / термиты
        Electric,      // Цепная молния / шоковые импульсы
        Acid,          // Кислотный дождь / агрессивная среда
        Environmental, // Эфирная эрозия / падения
        Contact        // Таранный щит / физический контакт
    };

    // ═══════════════════════════════════════════════════════
    // Результат расчёта повреждения
    // ═══════════════════════════════════════════════════════
    struct DamageResult
    {
        float actualDamage = 0.0f;
        bool wasFatal = false;
        bool secondWind = false;   // Сработал ли Second Wind
        bool partCrippled = false; // Была ли повреждена подсистема
    };

    // ═══════════════════════════════════════════════════════
    // Единая подсистема применения урона (DamageSystem)
    //
    // Реализует требования TARGET_ARCHITECTURE.md (§7.3):
    //   1. Обычный и взрывной урон с расчётом дистанции (falloff).
    //   2. Модификаторы типов урона по броне и шасси.
    //   3. Повреждение подсистем техники (гусеницы, сенсоры, орудия).
    //   4. Влияние урона на Эфирную Эрозию Пилота.
    //   5. Механика Second Wind (шанс избежать смертельного удара).
    //   6. Переходы состояний смерти (score, деактивация ИИ).
    // ═══════════════════════════════════════════════════════

    class DamageSystem
    {
    public:
        DamageSystem() = default;

        // ── Урон Игроку (Scout / Пеший Пилот) ──
        static DamageResult applyPlayerDamage(GameState &gs, float rawDamage, DamageType type)
        {
            DamageResult res;
            if (!gs.isRunning || gs.playerHealth <= 0.0f)
                return res;

            // Модификаторы сопротивлений Пилота
            float multiplier = 1.0f;
            switch (type)
            {
            case DamageType::Environmental:
                multiplier = 0.75f;
                break;
            case DamageType::Acid:
                multiplier = 1.20f;
                break;
            case DamageType::Explosive:
                multiplier = 1.10f;
                break;
            default:
                break;
            }

            float finalDmg = rawDamage * multiplier;

            // Механика Second Wind: если удар смертелен, но здоровье было > 25%
            if (gs.playerHealth - finalDmg <= 0.0f && gs.playerHealth > gs.playerMaxHealth * 0.25f)
            {
                gs.playerHealth = gs.playerMaxHealth * 0.15f; // Оставляем 15% HP
                res.secondWind = true;
                std::cout << "[SURVIVAL] !! SECOND WIND !! Смертельный удар заблокирован адреналином!" << std::endl;
            }
            else
            {
                gs.playerHealth = std::max(0.0f, gs.playerHealth - finalDmg);
            }

            res.actualDamage = finalDmg;

            // Нарастание Эфирной Эрозии при ранениях
            if (type != DamageType::Environmental)
            {
                gs.playerErosionLevel = std::min(100.0f, gs.playerErosionLevel + finalDmg * 0.06f);
            }

            if (gs.playerHealth <= 0.0f)
            {
                res.wasFatal = true;
                std::cout << "[DAMAGE] Пилот погиб в секторе Убежища 17." << std::endl;
            }

            return res;
        }

        // ── Урон противнику Роя (Вермины, Гули, Роботы, Тактики) ──
        static bool applyEnemyDamage(GameState &gs, Enemy &enemy, float rawDamage, DamageType type)
        {
            if (!enemy.isAlive)
                return false;

            float dmg = rawDamage;

            // Особенности урона по единицам Роя
            if (type == DamageType::Explosive || type == DamageType::Thermal)
            {
                dmg *= 1.25f; // Биомасса Роя крайне уязвима к огню и взрывам
            }
            else if (type == DamageType::Electric)
            {
                dmg *= 1.40f; // Перегрузка нервных узлов
            }

            enemy.health -= dmg;

            if (enemy.health <= 0.0f)
            {
                enemy.isAlive = false;
                gs.score += 100;
                return true; // Враг уничтожен
            }

            return false;
        }

        // ── Урон Танку БТ-7274 (Vanguard-class Titan) ──
        static DamageResult applyTitanDamage(GameState &gs, float rawDamage, DamageType type)
        {
            DamageResult res;
            if (gs.titan.health <= 0.0f)
                return res;

            float dmg = rawDamage;
            if (type == DamageType::Thermal)
                dmg *= 0.70f; // Тяжёлая термозащита реактора
            if (type == DamageType::Acid)
                dmg *= 1.30f; // Разъедание гидравлики

            gs.titan.health = std::max(0.0f, gs.titan.health - dmg);
            res.actualDamage = dmg;

            // Повреждение внутренних подсистем при сильном импульсе (> 18 ед.)
            if (dmg >= 18.0f)
            {
                gs.titan.systems.tracksCondition = std::max(0.0f, gs.titan.systems.tracksCondition - dmg * 0.25f);
                gs.titan.systems.sensorLink = std::max(0.0f, gs.titan.systems.sensorLink - dmg * 0.15f);
                gs.titan.systems.turretStatus = std::max(0.0f, gs.titan.systems.turretStatus - dmg * 0.10f);
                res.partCrippled = true;

                if (gs.titan.systems.tracksCondition < 40.0f)
                {
                    std::cout << "[BT-7274] Внимание: Критическое повреждение ходовой части!" << std::endl;
                }
            }

            if (gs.titan.health <= 0.0f)
            {
                res.wasFatal = true;
                gs.titan.isPiloted = false; // Автоматическое высаживание Пилота
                std::cout << "[DAMAGE] Ядро БТ-7274 деактивировано! Питание потеряно." << std::endl;
            }

            return res;
        }

        // ── Урон Вышке связи Убежища 17 ──
        static void applyTowerDamage(GameState &gs, float rawDamage)
        {
            if (gs.regionalGrid.towerHealth <= 0.0f)
                return;
            gs.regionalGrid.towerHealth = std::max(0.0f, gs.regionalGrid.towerHealth - rawDamage);
            if (gs.regionalGrid.towerHealth <= 0.0f)
            {
                gs.regionalGrid.towerSyncRecovered = false;
                std::cout << "[GRID] Региональная вышка связи разрушена!" << std::endl;
            }
        }

        // ── Взрыв по площади (AOE / Splash Damage) ──
        // Распределяет урон по всем сущностям в радиусе от точки взрыва
        static void applyRadiusDamage(GameState &gs, const Vector3D &origin, float maxRadius,
                                      float maxDamage, DamageType type)
        {
            if (maxRadius <= 0.001f || maxDamage <= 0.0f)
                return;
            const float maxRadiusSq = maxRadius * maxRadius;

            // 1. Враги
            for (auto &e : gs.enemies)
            {
                if (!e.isAlive)
                    continue;
                float dx = e.position.x - origin.x;
                float dy = e.position.y - origin.y;
                float distSq = dx * dx + dy * dy;

                if (distSq <= maxRadiusSq)
                {
                    float dist = std::sqrt(distSq);
                    float falloff = 1.0f - (dist / maxRadius);
                    float dmg = maxDamage * std::max(0.20f, falloff);
                    applyEnemyDamage(gs, e, dmg, type);
                }
            }

            // 2. Титан
            if (gs.titan.health > 0.0f)
            {
                float tdx = gs.titan.position.x - origin.x;
                float tdy = gs.titan.position.y - origin.y;
                float tDistSq = tdx * tdx + tdy * tdy;
                if (tDistSq <= maxRadiusSq)
                {
                    float dist = std::sqrt(tDistSq);
                    float falloff = 1.0f - (dist / maxRadius);
                    applyTitanDamage(gs, maxDamage * 0.6f * falloff, type);
                }
            }

            // 3. Игрок (если пешком)
            if (gs.playerMode != UnitMode::Titan && !gs.titan.isPiloted)
            {
                float pdx = gs.playerPos.x - origin.x;
                float pdy = gs.playerPos.y - origin.y;
                float pDistSq = pdx * pdx + pdy * pdy;
                if (pDistSq <= maxRadiusSq)
                {
                    float dist = std::sqrt(pDistSq);
                    float falloff = 1.0f - (dist / maxRadius);
                    applyPlayerDamage(gs, maxDamage * 0.5f * falloff, type);
                }
            }
        }
    };

} // namespace bunker
