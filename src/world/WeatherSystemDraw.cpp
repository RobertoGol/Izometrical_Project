#include "world/WeatherSystem.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>

namespace bunker
{

    void WeatherSystem::drawFogOverlay(sf::RenderWindow& window, float W, float H, float I) const
    {
        sf::RectangleShape fog({W, H});
        fog.setFillColor(sf::Color(115, 70, 180, static_cast<std::uint8_t>(30 + 95 * I)));
        window.draw(fog);

        for (int i = 0; i < 9; ++i)
        {
            const float phase = std::fmod(m_State.visualTimer * (14.0f + i * 1.7f) + i * 97.0f, H + 140.0f);
            const float y = phase - 70.0f;

            sf::RectangleShape band({W, 16.0f + 20.0f * I});
            band.setPosition({0.0f, y});
            band.setFillColor(sf::Color(180, 125, 255, static_cast<std::uint8_t>(15 + 30 * I)));
            window.draw(band);
        }
    }

    void WeatherSystem::drawAcidRainOverlay(sf::RenderWindow& window, float W, float H, float I) const
    {
        sf::RectangleShape tint({W, H});
        tint.setFillColor(sf::Color(70, 115, 30, static_cast<std::uint8_t>(18 + 55 * I)));
        window.draw(tint);

        const int drops = static_cast<int>(70 + 180 * I);
        const int Wi = std::max(1, static_cast<int>(W + 90.0f));
        const int Hi = std::max(1, static_cast<int>(H + 90.0f));
        const int tX = static_cast<int>(m_State.visualTimer * 235.0f);
        const int tY = static_cast<int>(m_State.visualTimer * 390.0f);

        for (int i = 0; i < drops; ++i)
        {
            const float x = static_cast<float>((i * 53 + tX) % Wi) - 45.0f;
            const float y = static_cast<float>((i * 89 + tY) % Hi) - 45.0f;

            sf::Vertex line[] = {{{x, y}, sf::Color(150, 255, 75, static_cast<std::uint8_t>(65 + 45 * I))},
                                 {{x - 8.0f, y + 24.0f}, sf::Color(150, 255, 75, 22)}};
            window.draw(line, 2, sf::PrimitiveType::Lines);
        }
    }

    sf::Color WeatherSystem::hudColor() const
    {
        switch (m_State.current)
        {
        case WeatherType::Clear:
            return sf::Color(150, 210, 160);
        case WeatherType::EtherFog:
            return sf::Color(210, 165, 255);
        case WeatherType::AcidRain:
            return sf::Color(165, 255, 90);
        case WeatherType::AshStorm:
        case WeatherType::EtherStorm:
            return sf::Color(235, 190, 255);
        default:
            break;
        }
        return sf::Color::White;
    }

    std::string WeatherSystem::weatherName(WeatherType t)
    {
        switch (t)
        {
        case WeatherType::Clear:
            return "CLEAR";
        case WeatherType::EtherFog:
            return "ETHER FOG";
        case WeatherType::AcidRain:
            return "ACID RAIN";
        case WeatherType::AshStorm:
            return "ASH STORM";
        case WeatherType::EtherStorm:
            return "ETHER STORM";
        default:
            break;
        }
        return "UNKNOWN";
    }

    std::string WeatherSystem::oneDecimal(float v)
    {
        int scaled = static_cast<int>(v * 10.0f + 0.5f);
        return std::to_string(scaled / 10) + "." + std::to_string(std::abs(scaled % 10));
    }

    float WeatherSystem::random01()
    {
        return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }


} // namespace bunker
