#include "world/WeatherSystem.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace bunker
{

    void WeatherSystem::applyWorldEffects(GameState& gs, float dt)
    {
        gs.worldVisibilityModifier = m_State.visibilityMultiplier;

        if (m_State.erosionBoost <= 0.001f)
            return;

        const float add = m_State.erosionBoost * 4.0f * dt;
        for (int x = 1; x < Config::MAP_WIDTH - 1; ++x)
        {
            for (int y = 1; y < Config::MAP_HEIGHT - 1; ++y)
            {
                if (gs.etherErosionMap[x][y] > 1.0f)
                {
                    gs.etherErosionMap[x][y] = std::min(100.0f, gs.etherErosionMap[x][y] + add);
                }
            }
        }
    }

    void WeatherSystem::applyDamageTick(GameState& gs, float elapsed)
    {
        if (m_State.acidDamagePerSecond <= 0.001f)
            return;

        const float dmg = m_State.acidDamagePerSecond * elapsed;

        if (gs.playerMode == UnitMode::Titan || gs.titan.isPiloted)
        {
            DamageSystem::applyTitanDamage(gs, dmg * 0.45f, DamageType::Acid);
        }
        else
        {
            DamageSystem::applyPlayerDamage(gs, dmg, DamageType::Acid);
        }

        // ── Разъедание кислотным дождём баррикад базового лагеря C.A.M.P. ──
        int erosionTick = std::max(1, static_cast<int>(dmg * 2.0f));
        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                if (gs.sectorMap[x][y] == 1 && gs.wallDurability[x][y] > 0)
                {
                    gs.wallDurability[x][y] = std::max(0, gs.wallDurability[x][y] - erosionTick);
                    if (gs.wallDurability[x][y] <= 0)
                    {
                        gs.sectorMap[x][y] = 0; // Баррикада разрушена кислотной средой
                    }
                }
            }
        }
    }


} // namespace bunker
