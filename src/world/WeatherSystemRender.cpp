#include "world/WeatherSystem.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace bunker
{

    void WeatherSystem::renderWorldOverlay(sf::RenderWindow& window) const
    {
        if (m_State.intensity <= 0.01f || m_State.current == WeatherType::Clear)
            return;

        sf::View oldView = window.getView();
        window.setView(window.getDefaultView());

        const float W = static_cast<float>(Config::SCREEN_WIDTH);
        const float H = static_cast<float>(Config::SCREEN_HEIGHT);
        const float I = std::clamp(m_State.intensity, 0.0f, 1.0f);

        if (m_State.current == WeatherType::EtherFog || m_State.current == WeatherType::EtherStorm)
        {
            drawFogOverlay(window, W, H, I);
        }

        if (m_State.current == WeatherType::AcidRain || m_State.current == WeatherType::EtherStorm)
        {
            drawAcidRainOverlay(window, W, H, I);
        }

        if (m_State.flashAlpha > 0.01f)
        {
            sf::RectangleShape flash({W, H});
            flash.setFillColor(
                sf::Color(215, 190, 255, static_cast<std::uint8_t>(std::clamp(m_State.flashAlpha, 0.0f, 160.0f))));
            window.draw(flash);
        }

        window.setView(oldView);
    }

    void WeatherSystem::renderHUD(sf::RenderWindow& window, const sf::Font* font) const
    {
        if (!font)
            return;

        sf::Text text(*font, "", 13);
        text.setFillColor(hudColor());
        text.setPosition({12.0f, 96.0f});

        std::string line =
            "WEATHER: " + label() + "  INT " + std::to_string(static_cast<int>(m_State.intensity * 100.0f)) + "%";
        switch (m_State.current)
        {
        case WeatherType::EtherFog:
            line += "  VIS -" + std::to_string(static_cast<int>((1.0f - m_State.visibilityMultiplier) * 100.0f)) + "%";
            break;
        case WeatherType::AcidRain:
            line += "  ACID " + oneDecimal(m_State.acidDamagePerSecond) + "/s";
            break;
        case WeatherType::EtherStorm:
            line += "  VIS -" + std::to_string(static_cast<int>((1.0f - m_State.visibilityMultiplier) * 100.0f)) + "%";
            line += "  ACID " + oneDecimal(m_State.acidDamagePerSecond) + "/s";
            break;
        default:
            break;
        }

        text.setString(line);
        window.draw(text);
    }


} // namespace bunker
