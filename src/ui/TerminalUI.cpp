#include "ui/TerminalUI.hpp"
#include "core/IsoMath.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace bunker
{

    TerminalManager::TerminalManager()
    {
        spawnDefaultTerminals();
    }

    void TerminalManager::spawnDefaultTerminals()
    {
        m_Terminals.clear();

        m_Terminals.push_back({1, {8.0f, 6.0f, 0.0f}, "ВАХТОВЫЙ ЖУРНАЛ КП #17", "ЗАПИСЬ 2077: Система вентиляции зафиксировала выброс эфирного газа. Протокол изоляции активирован.", 1, false});

        m_Terminals.push_back({2, {13.0f, 12.0f, 0.0f}, "ТЕХНИЧЕСКИЙ ТЕРМИНАЛ АНГАР-4", "БТ-7274 переведен в режим энергосбережения. Котёл реактора законсервирован под давлением 350 Бар.", 1, false});

        m_Terminals.push_back({3, {6.0f, 14.0f, 0.0f}, "СЕКРЕТНЫЙ АРХИВ ПРОЕКТА 'АВАНГАРД'", "ДОПУСК подтвержден. Кооп-синхронизация турелей Vanguard разблокирована для Пилотов класса 2.", 2, false});
    }

    const BunkerTerminal *TerminalManager::getNearestTerminal(const Vector3D &playerPos, float maxRange) const
    {
        float maxRangeSq = maxRange * maxRange;
        const BunkerTerminal *nearest = nullptr;
        float bestDistSq = maxRangeSq;

        for (const auto &term : m_Terminals)
        {
            float dx = term.position.x - playerPos.x;
            float dy = term.position.y - playerPos.y;
            float dSq = dx * dx + dy * dy;
            if (dSq < bestDistSq)
            {
                bestDistSq = dSq;
                nearest = &term;
            }
        }
        return nearest;
    }

    bool TerminalManager::tryInteractTerminal(GameState &gs)
    {
        if (m_IsOpen)
            return false;

        float bestDistSq = 2.25f;
        int bestIdx = -1;

        for (int i = 0; i < static_cast<int>(m_Terminals.size()); ++i)
        {
            float dx = m_Terminals[i].position.x - gs.playerPos.x;
            float dy = m_Terminals[i].position.y - gs.playerPos.y;
            float dSq = dx * dx + dy * dy;
            if (dSq < bestDistSq)
            {
                bestDistSq = dSq;
                bestIdx = i;
            }
        }

        if (bestIdx >= 0)
        {
            m_ActiveTerminalIndex = bestIdx;
            m_IsOpen = true;
            std::cout << "[TERMINAL] Подключение к терминалу: '" << m_Terminals[bestIdx].title << "'" << std::endl;
            return true;
        }
        return false;
    }

    void TerminalManager::handleWindowEvents(sf::RenderWindow &window, GameState &gs)
    {
        if (!m_IsOpen)
            return;

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                gs.isRunning = false;
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::E)
                {
                    close();
                }
                else if (event.key.code == sf::Keyboard::Space || event.key.code == sf::Keyboard::Enter)
                {
                    if (m_ActiveTerminalIndex >= 0)
                    {
                        auto &term = m_Terminals[m_ActiveTerminalIndex];
                        if (!term.isSynced)
                        {
                            term.isSynced = true;
                            gs.score += 250;
                            Progression::awardXP(gs, 150);
                            std::cout << "[TERMINAL SYNC] Архивные записи загружены в Pip-Pad! +150 XP" << std::endl;
                        }
                    }
                }
            }
        }
    }

    void TerminalManager::update(float dt)
    {
        if (m_IsOpen)
        {
            m_CrtScanlineTimer += dt * 5.0f;
        }
    }

    void TerminalManager::render(sf::RenderWindow &window, const GameState &gs) const
    {
        if (!m_IsOpen || m_ActiveTerminalIndex < 0)
            return;

        const auto &term = m_Terminals[m_ActiveTerminalIndex];
        float W = static_cast<float>(Config::SCREEN_WIDTH);
        float H = static_cast<float>(Config::SCREEN_HEIGHT);

        sf::RectangleShape crtBg({W, H});
        crtBg.setFillColor(sf::Color(10, 20, 12, 245));
        window.draw(crtBg);

        sf::RectangleShape border({W - 120, H - 120});
        border.setPosition(60, 60);
        border.setFillColor(sf::Color(15, 30, 18));
        border.setOutlineThickness(3.0f);
        border.setOutlineColor(sf::Color(50, 220, 80));
        window.draw(border);

        if (m_FontLoaded)
        {
            drawText(window, "ROBCO INDUSTRIES (TM) TERMLINK — VAULT 17 REGIONAL NET", 80, 80, 16, sf::Color(50, 255, 90));
            drawText(window, "=======================================================", 80, 105, 16, sf::Color(50, 220, 80));

            drawText(window, "ФАЙЛ: " + term.title, 80, 140, 18, sf::Color(100, 255, 120));
            drawText(window, "УРОВЕНЬ ДОПУСКА: ТИР-" + std::to_string(term.clearanceTier) + (term.isSynced ? " [СИНХРОНИЗИРОВАНО]" : " [ТРЕБУЕТСЯ СИНХРОНИЗАЦИЯ]"), 80, 170, 14, term.isSynced ? sf::Color(100, 255, 100) : sf::Color(255, 200, 50));

            drawText(window, term.logContent, 80, 220, 15, sf::Color(180, 240, 190));

            drawText(window, term.isSynced ? ">> ДАННЫЕ АРХИВА В БЕЗОПАСНОСТИ <<" : ">> НАЖМИТЕ [SPACE] ИЛИ [ENTER] ДЛЯ СИНХРОНИЗАЦИИ АРХИВА (+150 XP) <<", 80, H - 140, 14, term.isSynced ? sf::Color(100, 255, 120) : sf::Color(255, 255, 100));
            drawText(window, "[ESC] Выйти из терминала", 80, H - 100, 13, sf::Color(120, 180, 130));
        }
    }

    void TerminalManager::drawText(sf::RenderWindow &window, const std::string &str, float x, float y, int size, sf::Color color) const
    {
        if (!m_FontLoaded)
            return;
        sf::Text text;
        text.setFont(m_Font);
        text.setString(str);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setPosition(x, y);
        window.draw(text);
    }

} // namespace bunker
