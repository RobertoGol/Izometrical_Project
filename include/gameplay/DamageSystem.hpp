#pragma once

#include "core/Types.hpp"
#include "core/Constants.hpp"
#include "core/IsoMath.hpp"
#include "gameplay/GameState.hpp"

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
    // Реализует требования TARGET_ARCHITECTURE.md (§7.3)
    // ═══════════════════════════════════════════════════════
    class DamageSystem
    {
    public:
        DamageSystem() = default;

        static DamageResult applyPlayerDamage(GameState &gs, float rawDamage, DamageType type);
        static bool applyEnemyDamage(GameState &gs, Enemy &enemy, float rawDamage, DamageType type);
        static DamageResult applyTitanDamage(GameState &gs, float rawDamage, DamageType type);
        static void applyTowerDamage(GameState &gs, float rawDamage);
        static void applyRadiusDamage(GameState &gs, const Vector3D &origin, float maxRadius,
                                      float maxDamage, DamageType type);
        static void applySubsystemCrippling(TitanAlly &titan, DamageType type, float rawDamage);
    };

} // namespace bunker