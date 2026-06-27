#include "engine/TimeShift.hpp"
#include "ai/EnemySpawner.hpp"
#include <cmath>
#include <algorithm>

namespace bunker
{

    void TimeShift::initialize(GameState &gs, EnemySpawner &spawner)
    {
        savePresentFromGameState(gs);
        generatePastTimeline(gs, spawner);
        m_CurrentTimeline = Timeline::Present;
        m_Initialized = true;
        std::cout << "[TIMESHIFT] Два таймлайна инициализированы." << std::endl;
    }

    bool TimeShift::tryShift(GameState &gs)
    {
        if (!m_Initialized)
            return false;
        if (m_ShiftCooldown > 0.0f)
            return false;
        if (m_IsTransitioning)
            return false;

        if (m_CurrentTimeline == Timeline::Present && m_DeviceCharge < 5.0f)
        {
            std::cout << "[TIMESHIFT] Устройство разряжено!" << std::endl;
            return false;
        }

        m_IsTransitioning = true;
        m_TransitionTimer = m_TransitionDuration;

        if (m_CurrentTimeline == Timeline::Present)
        {
            savePresentFromGameState(gs);
            loadPastToGameState(gs);
            m_CurrentTimeline = Timeline::Past;
            std::cout << "[TIMESHIFT] ПЕРЕНОС В ПРОШЛОЕ (Целое Убежище 17)" << std::endl;
        }
        else
        {
            savePastFromGameState(gs);
            loadPresentToGameState(gs);
            m_CurrentTimeline = Timeline::Present;
            std::cout << "[TIMESHIFT] ВОЗВРАТ В НАСТОЯЩЕЕ (Разрушенное Убежище 17)" << std::endl;
        }

        m_ShiftCooldown = m_ShiftCooldownMax;
        return true;
    }

    void TimeShift::update(float dt)
    {
        if (dt <= 0.0f)
            return;
        if (dt > 0.1f)
            dt = 0.1f;

        if (m_ShiftCooldown > 0.0f)
        {
            m_ShiftCooldown = std::max(0.0f, m_ShiftCooldown - dt);
        }

        if (m_IsTransitioning)
        {
            m_TransitionTimer -= dt;
            if (m_TransitionTimer <= 0.0f)
            {
                m_IsTransitioning = false;
            }
        }

        if (m_CurrentTimeline == Timeline::Past)
        {
            m_DeviceCharge = std::max(0.0f, m_DeviceCharge - m_DrainRate * dt);
            if (m_DeviceCharge <= 0.0f)
            {
                std::cout << "[TIMESHIFT] Критический разряд! Автоматический возврат в настоящее." << std::endl;
            }
        }
        else
        {
            m_DeviceCharge = std::min(m_MaxCharge, m_DeviceCharge + m_RechargeRate * dt);
        }
    }

    void TimeShift::renderTransitionEffect(sf::RenderWindow &window) const
    {
        if (!m_IsTransitioning)
            return;

        float alpha = (m_TransitionTimer / m_TransitionDuration) * 180.0f;
        alpha = std::clamp(alpha, 0.0f, 255.0f);

        sf::RectangleShape flash({static_cast<float>(Config::SCREEN_WIDTH),
                                  static_cast<float>(Config::SCREEN_HEIGHT)});
        flash.setFillColor(sf::Color(100, 200, 255, static_cast<sf::Uint8>(alpha)));
        window.draw(flash);
    }

    void TimeShift::renderHUD(sf::RenderWindow &window, const sf::Font *font) const
    {
        if (!font)
            return;

        float W = static_cast<float>(Config::SCREEN_WIDTH);

        sf::Text text;
        text.setFont(*font);
        text.setCharacterSize(12);
        text.setFillColor(isPast() ? sf::Color(100, 220, 255) : sf::Color(255, 180, 50));
        text.setPosition(W / 2 - 60, 15);

        std::string line = isPast() ? "TIMELINE: PAST (Pre-War)" : "TIMELINE: PRESENT";
        text.setString(line);
        window.draw(text);

        float pct = getChargePercent();
        sf::RectangleShape bg({120.0f, 6.0f});
        bg.setPosition(W / 2 - 60, 32);
        bg.setFillColor(sf::Color(40, 40, 40, 150));
        bg.setOutlineThickness(1.0f);
        bg.setOutlineColor(sf::Color::Black);
        window.draw(bg);

        sf::RectangleShape fill({120.0f * pct, 6.0f});
        fill.setPosition(W / 2 - 60, 32);
        fill.setFillColor(isPast() ? sf::Color(80, 200, 255) : sf::Color(255, 160, 40));
        window.draw(fill);

        if (m_ShiftCooldown > 0.0f)
        {
            sf::Text cd;
            cd.setFont(*font);
            cd.setCharacterSize(9);
            cd.setString("CD:" + std::to_string(static_cast<int>(m_ShiftCooldown * 10) / 10) + "s");
            cd.setFillColor(sf::Color(150, 150, 150));
            cd.setPosition(W / 2 - 15, 45);
            window.draw(cd);
        }
    }

    void TimeShift::savePresentFromGameState(const GameState &gs)
    {
        m_PresentSnapshot.sectorMap = gs.sectorMap;
        m_PresentSnapshot.wallDurability = gs.wallDurability;
        m_PresentSnapshot.erosionMap = gs.etherErosionMap;
        m_PresentSnapshot.enemies = gs.enemies;
        m_PresentSnapshot.lootContainers = gs.lootContainers;
    }

    void TimeShift::savePastFromGameState(const GameState &gs)
    {
        m_PastSnapshot.sectorMap = gs.sectorMap;
        m_PastSnapshot.wallDurability = gs.wallDurability;
        m_PastSnapshot.erosionMap = gs.etherErosionMap;
        m_PastSnapshot.enemies = gs.enemies;
        m_PastSnapshot.lootContainers = gs.lootContainers;
    }

    void TimeShift::loadPresentToGameState(GameState &gs)
    {
        gs.sectorMap = m_PresentSnapshot.sectorMap;
        gs.wallDurability = m_PresentSnapshot.wallDurability;
        gs.etherErosionMap = m_PresentSnapshot.erosionMap;
        gs.enemies = m_PresentSnapshot.enemies;
        gs.lootContainers = m_PresentSnapshot.lootContainers;
    }

    void TimeShift::loadPastToGameState(GameState &gs)
    {
        gs.sectorMap = m_PastSnapshot.sectorMap;
        gs.wallDurability = m_PastSnapshot.wallDurability;
        gs.etherErosionMap = m_PastSnapshot.erosionMap;
        gs.enemies = m_PastSnapshot.enemies;
        gs.lootContainers = m_PastSnapshot.lootContainers;
    }

    void TimeShift::generatePastTimeline(GameState &gs, EnemySpawner &)
    {
        for (int x = 0; x < Config::MAP_WIDTH; ++x)
        {
            for (int y = 0; y < Config::MAP_HEIGHT; ++y)
            {
                m_PastSnapshot.sectorMap[x][y] = 0;
                m_PastSnapshot.wallDurability[x][y] = 0;
                m_PastSnapshot.erosionMap[x][y] = 0.0f;
            }
        }

        for (int x = 3; x < 17; ++x)
        {
            for (int y = 3; y < 17; ++y)
            {
                if (x == 3 || x == 16 || y == 3 || y == 16)
                {
                    m_PastSnapshot.sectorMap[x][y] = 1;
                    m_PastSnapshot.wallDurability[x][y] = 200;
                }
            }
        }

        for (int y = 3; y < 17; ++y)
        {
            m_PastSnapshot.sectorMap[8][y] = 1;
            m_PastSnapshot.wallDurability[8][y] = 150;
            m_PastSnapshot.sectorMap[12][y] = 1;
            m_PastSnapshot.wallDurability[12][y] = 150;
        }
        for (int x = 3; x < 17; ++x)
        {
            m_PastSnapshot.sectorMap[x][8] = 1;
            m_PastSnapshot.wallDurability[x][8] = 150;
            m_PastSnapshot.sectorMap[x][12] = 1;
            m_PastSnapshot.wallDurability[x][12] = 150;
        }

        int doors[][2] = {
            {8, 5}, {8, 10}, {8, 14}, {12, 5}, {12, 10}, {12, 14}, {5, 8}, {10, 8}, {14, 8}, {5, 12}, {10, 12}, {14, 12}, {9, 3}, {9, 16}};
        for (auto &d : doors)
        {
            m_PastSnapshot.sectorMap[d[0]][d[1]] = 0;
        }

        m_PastSnapshot.enemies.clear();

        Vector3D robotPositions[] = {
            {5.0f, 5.0f, 0.0f}, {10.0f, 5.0f, 0.0f}, {14.0f, 5.0f, 0.0f}, {5.0f, 10.0f, 0.0f}, {14.0f, 10.0f, 0.0f}, {5.0f, 14.0f, 0.0f}, {10.0f, 14.0f, 0.0f}, {14.0f, 14.0f, 0.0f}};

        for (const auto &pos : robotPositions)
        {
            Enemy robot;
            robot.position = pos;
            robot.health = 80.0f;
            robot.speed = 1.8f;
            robot.radius = 0.4f;
            robot.isAlive = true;
            m_PastSnapshot.enemies.push_back(robot);
        }

        m_PastSnapshot.lootContainers.clear();

        LootContainer pastCrate1;
        pastCrate1.position = {6.0f, 6.0f, 0.0f};
        pastCrate1.type = LootContainerType::IronSafe;
        pastCrate1.isOpened = false;
        pastCrate1.containsItems.push_back({401, ItemType::Resource, 10, 0.5f, "PRE-WAR REPAIR KIT"});
        m_PastSnapshot.lootContainers.push_back(pastCrate1);

        LootContainer pastCrate2;
        pastCrate2.position = {13.0f, 13.0f, 0.0f};
        pastCrate2.type = LootContainerType::IronSafe;
        pastCrate2.isOpened = false;
        pastCrate2.containsItems.push_back({102, ItemType::Weapon, 1, 2.0f, "PROTOTYPE XO-16"});
        m_PastSnapshot.lootContainers.push_back(pastCrate2);

        std::cout << "[TIMESHIFT] Прошлое сгенерировано: "
                  << m_PastSnapshot.enemies.size() << " роботов, "
                  << m_PastSnapshot.lootContainers.size() << " контейнеров." << std::endl;
    }

} // namespace bunker
