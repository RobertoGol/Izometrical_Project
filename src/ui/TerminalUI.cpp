#include "ui/TerminalUI.hpp"
#include "core/IsoMath.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>

namespace bunker
{
    namespace
    {
        constexpr const char* TerminalCrtShader = R"(
            uniform sampler2D texture;
            uniform float u_time;
            uniform vec2 u_resolution;

            void main()
            {
                vec2 uv = gl_TexCoord[0].xy;
                vec2 centered = uv - vec2(0.5);
                float vignette = 1.0 - dot(centered, centered) * 0.65;
                float scanline = 0.88 + 0.12 * sin((uv.y * u_resolution.y + u_time * 80.0) * 3.14159);
                float jitter = sin((uv.y * 180.0) + u_time * 18.0) * 0.0015;
                vec4 base = texture2D(texture, vec2(uv.x + jitter, uv.y));
                vec3 phosphor = vec3(base.r * 0.70, base.g * 1.08, base.b * 0.72);
                gl_FragColor = vec4(phosphor * scanline * vignette, base.a);
            }
        )";
    }

    TerminalManager::TerminalManager()
    {
        spawnDefaultTerminals();
    }

    void TerminalManager::spawnDefaultTerminals()
    {
        m_Terminals.clear();

        m_Terminals.push_back(
            {1,
             {8.0f, 6.0f, 0.0f},
             "ВАХТОВЫЙ ЖУРНАЛ КП #17",
             "ЗАПИСЬ 2077: Система вентиляции зафиксировала выброс эфирного газа. Протокол изоляции активирован.",
             1,
             false});

        m_Terminals.push_back(
            {2,
             {13.0f, 12.0f, 0.0f},
             "ТЕХНИЧЕСКИЙ ТЕРМИНАЛ АНГАР-4",
             "БТ-7274 переведен в режим энергосбережения. Котёл реактора законсервирован под давлением 350 Бар.",
             1,
             false});

        m_Terminals.push_back(
            {3,
             {6.0f, 14.0f, 0.0f},
             "СЕКРЕТНЫЙ АРХИВ ПРОЕКТА 'АВАНГАРД'",
             "ДОПУСК подтвержден. Кооп-синхронизация турелей Vanguard разблокирована для Пилотов класса 2.",
             2,
             false});
    }

    const BunkerTerminal* TerminalManager::getNearestTerminal(const Vector3D& playerPos, float maxRange) const
    {
        float maxRangeSq = maxRange * maxRange;
        const BunkerTerminal* nearest = nullptr;
        float bestDistSq = maxRangeSq;

        for (const auto& term : m_Terminals)
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

    bool TerminalManager::tryInteractTerminal(GameState& gs)
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
            bunker::logInfo() << "[TERMINAL] Подключение к терминалу: '" << m_Terminals[bestIdx].title << "'"
                              << std::endl;
            return true;
        }
        return false;
    }

    void TerminalManager::handleWindowEvents(sf::RenderWindow& window, GameState& gs)
    {
        if (!m_IsOpen)
            return;

        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                gs.isRunning = false;

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                switch (keyPressed->code)
                {
                case sf::Keyboard::Key::Escape:
                case sf::Keyboard::Key::E:
                    close();
                    break;
                case sf::Keyboard::Key::Space:
                case sf::Keyboard::Key::Enter:
                    if (m_ActiveTerminalIndex >= 0)
                    {
                        auto& term = m_Terminals[m_ActiveTerminalIndex];
                        if (!term.isSynced)
                        {
                            term.isSynced = true;
                            gs.score += 250;
                            Progression::awardXP(gs, 150);
                            bunker::logInfo()
                                << "[TERMINAL SYNC] Архивные записи загружены в Pip-Pad! +150 XP" << std::endl;
                        }
                    }
                    break;
                default:
                    break;
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

    void TerminalManager::render(sf::RenderWindow& window, const GameState& gs) const
    {
        (void)gs;
        if (!m_IsOpen || m_ActiveTerminalIndex < 0)
            return;

        const auto& term = m_Terminals[m_ActiveTerminalIndex];
        if (ensureCrtResources(window.getSize()))
        {
            m_CrtSurface.clear(sf::Color::Transparent);
            renderTerminalContent(m_CrtSurface, term);
            m_CrtSurface.display();

            sf::Sprite terminalSprite(m_CrtSurface.getTexture());
            m_CrtShader.setUniform("u_time", m_CrtScanlineTimer);
            m_CrtShader.setUniform("u_resolution",
                                   sf::Glsl::Vec2(static_cast<float>(window.getSize().x),
                                                  static_cast<float>(window.getSize().y)));
            window.draw(terminalSprite, &m_CrtShader);
            return;
        }

        renderTerminalContent(window, term);
    }

    bool TerminalManager::ensureCrtResources(sf::Vector2u size) const
    {
        if (!sf::Shader::isAvailable())
        {
            return false;
        }

        if (!m_CrtSurfaceReady || m_CrtSurface.getSize() != size)
        {
            m_CrtSurfaceReady = m_CrtSurface.resize(size);
        }
        if (!m_CrtSurfaceReady)
        {
            return false;
        }

        if (!m_CrtShaderReady)
        {
            m_CrtShaderReady = m_CrtShader.loadFromMemory(TerminalCrtShader, sf::Shader::Type::Fragment);
        }

        return m_CrtShaderReady;
    }

    void TerminalManager::renderTerminalContent(sf::RenderTarget& target, const BunkerTerminal& term) const
    {
        float W = static_cast<float>(target.getSize().x);
        float H = static_cast<float>(target.getSize().y);
        sf::RenderTarget& window = target;

        sf::RectangleShape crtBg({W, H});
        crtBg.setFillColor(sf::Color(10, 20, 12, 245));
        target.draw(crtBg);

        sf::RectangleShape border({W - 120, H - 120});
        border.setPosition({60.0f, 60.0f});
        border.setFillColor(sf::Color(15, 30, 18));
        border.setOutlineThickness(3.0f);
        border.setOutlineColor(sf::Color(50, 220, 80));
        target.draw(border);

        if (m_FontLoaded)
        {
            drawText(window, "ROBCO INDUSTRIES (TM) TERMLINK — VAULT 17 REGIONAL NET", 80, 80, 16,
                     sf::Color(50, 255, 90));
            drawText(window, "=======================================================", 80, 105, 16,
                     sf::Color(50, 220, 80));

            drawText(window, "ФАЙЛ: " + term.title, 80, 140, 18, sf::Color(100, 255, 120));
            drawText(window,
                     "УРОВЕНЬ ДОПУСКА: ТИР-" + std::to_string(term.clearanceTier) +
                         (term.isSynced ? " [СИНХРОНИЗИРОВАНО]" : " [ТРЕБУЕТСЯ СИНХРОНИЗАЦИЯ]"),
                     80, 170, 14, term.isSynced ? sf::Color(100, 255, 100) : sf::Color(255, 200, 50));

            drawText(window, term.logContent, 80, 220, 15, sf::Color(180, 240, 190));

            drawText(window,
                     term.isSynced ? ">> ДАННЫЕ АРХИВА В БЕЗОПАСНОСТИ <<"
                                   : ">> НАЖМИТЕ [SPACE] ИЛИ [ENTER] ДЛЯ СИНХРОНИЗАЦИИ АРХИВА (+150 XP) <<",
                     80, H - 140, 14, term.isSynced ? sf::Color(100, 255, 120) : sf::Color(255, 255, 100));
            drawText(window, "[ESC] Выйти из терминала", 80, H - 100, 13, sf::Color(120, 180, 130));
        }
    }

    void TerminalManager::drawText(sf::RenderWindow& window, const std::string& str, float x, float y, int size,
                                   sf::Color color) const
    {
        if (!m_FontLoaded)
            return;
        sf::Text text(m_Font, str, static_cast<unsigned int>(size));
        text.setFillColor(color);
        text.setPosition({x, y});
        window.draw(text);
    }

    void TerminalManager::drawText(sf::RenderTarget& target, const std::string& str, float x, float y, int size,
                                   sf::Color color) const
    {
        if (!m_FontLoaded)
            return;
        sf::Text text(m_Font, str, static_cast<unsigned int>(size));
        text.setFillColor(color);
        text.setPosition({x, y});
        target.draw(text);
    }

} // namespace bunker
