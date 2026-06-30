#include "ai/PerceptionSystem.hpp"
#include "core/Constants.hpp"
#include <iostream>

namespace bunker
{

    static std::vector<SensoryStimulus> s_GlobalStimuliBuffer;

    const std::vector<SensoryStimulus> &PerceptionSystem::activeStimuli()
    {
        return s_GlobalStimuliBuffer;
    }

    void PerceptionSystem::emitSoundStimulus(const Vector3D &pos, StimulusType type, float intensity)
    {
        SensoryStimulus s;
        s.origin = pos;
        s.type = type;
        s.intensity = intensity;
        s.maxAge = (type == StimulusType::Explosion) ? 3.5f : 1.5f;
        s.currentAge = 0.0f;

        s_GlobalStimuliBuffer.push_back(s);
        if (s_GlobalStimuliBuffer.size() > 32)
        {
            s_GlobalStimuliBuffer.erase(s_GlobalStimuliBuffer.begin());
        }
    }

    void PerceptionSystem::updateStimuli(float dt)
    {
        for (auto &s : s_GlobalStimuliBuffer)
        {
            s.currentAge += dt;
        }
        s_GlobalStimuliBuffer.erase(
            std::remove_if(s_GlobalStimuliBuffer.begin(), s_GlobalStimuliBuffer.end(), [](const SensoryStimulus &s)
                           { return s.currentAge >= s.maxAge; }),
            s_GlobalStimuliBuffer.end());
    }

    float PerceptionSystem::calculateAcousticMuffling(const WorldGridState &grid, const Vector3D &soundPos, const Vector3D &listenerPos)
    {
        // Оптимизация под слабые ПК: целочисленное глушение стен бункера без тяжелых рейкастов
        float dx = listenerPos.x - soundPos.x;
        float dy = listenerPos.y - soundPos.y;
        int steps = std::max(1, static_cast<int>(std::sqrt(dx * dx + dy * dy)));
        float sx = dx / steps, sy = dy / steps;
        float cx = soundPos.x, cy = soundPos.y;

        float muffling = 1.0f;
        for (int i = 1; i < steps; ++i)
        {
            cx += sx;
            cy += sy;
            int tx = static_cast<int>(cx), ty = static_cast<int>(cy);
            if (tx >= 0 && tx < Config::MAP_WIDTH && ty >= 0 && ty < Config::MAP_HEIGHT)
            {
                if (grid.sectorMap[tx][ty] == 1)
                {
                    muffling *= 0.65f; // Каждая преграда гасит звук выстрела/шага на 35%
                }
            }
        }
        return muffling;
    }

    bool PerceptionSystem::evaluateSensoryAwareness(const GameState &gs, const Enemy &observer, PeripheralCone cone, float &outAwarenessDelta)
    {
        outAwarenessDelta = 0.0f;
        bool detectedAnything = false;

        // 1) Зрительный опрос в конусе 120° (Не удалять старые проверки, модулировать угол обзора)
        float pdx = gs.playerPos.x - observer.position.x;
        float pdy = gs.playerPos.y - observer.position.y;
        float distSq = pdx * pdx + pdy * pdy;

        if (distSq <= 100.0f) // В радиусе 10 тайлов
        {
            bool inCone = cone.isInsideFOV(observer.position, gs.playerPos);
            bool los = hasLineOfSight(gs, observer.position, gs.playerPos);

            if (los)
            {
                detectedAnything = true;
                if (inCone)
                {
                    outAwarenessDelta += 45.0f; // Прямой зрительный контакт перед глазами
                }
                else
                {
                    outAwarenessDelta += 8.0f; // Стелс-бонус: Пилот крадётся в слепой зоне за спиной врага
                }
            }
        }

        // 2) Акустический опрос шумов выстрелов и шагов
        for (const auto &stim : s_GlobalStimuliBuffer)
        {
            float sdx = stim.origin.x - observer.position.x;
            float sdy = stim.origin.y - observer.position.y;
            float sDistSq = sdx * sdx + sdy * sdy;
            float maxR = stim.intensity;

            if (sDistSq <= maxR * maxR)
            {
                float muffle = calculateAcousticMuffling(gs, stim.origin, observer.position);
                outAwarenessDelta += (stim.intensity * 3.0f * muffle);
                detectedAnything = true;
            }
        }

        return detectedAnything;
    }

    bool PerceptionSystem::hasLineOfSight(const WorldGridState &grid, const Vector3D &from, const Vector3D &to)
    {
        float dx = to.x - from.x;
        float dy = to.y - from.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= 0.05f)
            return true;

        int steps = std::max(1, static_cast<int>(dist * 3.0f));
        float stepX = dx / static_cast<float>(steps);
        float stepY = dy / static_cast<float>(steps);

        float currX = from.x;
        float currY = from.y;

        for (int i = 1; i < steps; ++i)
        {
            currX += stepX;
            currY += stepY;

            int tileX = static_cast<int>(currX);
            int tileY = static_cast<int>(currY);

            if (tileX >= 0 && tileX < Config::MAP_WIDTH && tileY >= 0 && tileY < Config::MAP_HEIGHT)
            {
                if (grid.sectorMap[tileX][tileY] == 1)
                {
                    return false;
                }
            }
        }

        return true;
    }

    float PerceptionSystem::calculateEffectiveDetectRadius(float baseRadius, float visibilityModifier)
    {
        return baseRadius * std::clamp(visibilityModifier, 0.20f, 1.0f);
    }

} // namespace bunker
