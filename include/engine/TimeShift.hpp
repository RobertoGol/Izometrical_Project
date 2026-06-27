#pragma once

#include "Types.hpp"
#include "GameState.hpp"
#include "Constants.hpp"
#include "EnemySpawner.hpp"
#include <array>
#include <vector>
#include <iostream>

namespace bunker
{

    enum class Timeline
    {
        Present,
        Past
    };

    struct TimelineSnapshot
    {
        std::array<std::array<int, Config::MAP_HEIGHT>, Config::MAP_WIDTH> sectorMap{};
        std::array<std::array<int, Config::MAP_HEIGHT>, Config::MAP_WIDTH> wallDurability{};
        std::array<std::array<float, Config::MAP_HEIGHT>, Config::MAP_WIDTH> erosionMap{};
        std::vector<Enemy> enemies;
        std::vector<LootContainer> lootContainers;
    };

    class TimeShift
    {
    private:
        Timeline m_CurrentTimeline = Timeline::Present;
        TimelineSnapshot m_PresentSnapshot;
        TimelineSnapshot m_PastSnapshot;

        float m_ShiftCooldown = 0.0f;
        float m_ShiftCooldownMax = 1.5f;

        float m_DeviceCharge = 100.0f;
        float m_MaxCharge = 100.0f;
        float m_DrainRate = 8.0f;
        float m_RechargeRate = 5.0f;

        float m_TransitionTimer = 0.0f;
        float m_TransitionDuration = 0.4f;
        bool m_IsTransitioning = false;
        bool m_Initialized = false;

    public:
        TimeShift() = default;

        void initialize(GameState &gs, EnemySpawner &spawner);
        bool tryShift(GameState &gs);
        void update(float dt);
        void renderTransitionEffect(sf::RenderWindow &window) const;
        void renderHUD(sf::RenderWindow &window, const sf::Font *font) const;

        Timeline getCurrentTimeline() const { return m_CurrentTimeline; }
        bool isPast() const { return m_CurrentTimeline == Timeline::Past; }
        bool isPresent() const { return m_CurrentTimeline == Timeline::Present; }
        bool isTransitioning() const { return m_IsTransitioning; }
        float getDeviceCharge() const { return m_DeviceCharge; }
        float getMaxCharge() const { return m_MaxCharge; }
        float getChargePercent() const { return m_DeviceCharge / m_MaxCharge; }
        float getCooldown() const { return m_ShiftCooldown; }
        bool isReady() const { return m_ShiftCooldown <= 0.0f && m_DeviceCharge >= 5.0f; }
        bool isInitialized() const { return m_Initialized; }

    private:
        void savePresentFromGameState(const GameState &gs);
        void savePastFromGameState(const GameState &gs);
        void loadPresentToGameState(GameState &gs);
        void loadPastToGameState(GameState &gs);
        void generatePastTimeline(GameState &gs, EnemySpawner &spawner);
    };

} // namespace bunker
