#include "engine/AudioController.hpp"
#include "core/Constants.hpp"
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace bunker
{

    static std::string resolveAudioPath(const std::string &name)
    {
        std::string oggName = name;
        if (oggName.size() >= 4 && oggName.substr(oggName.size() - 4) == ".mp3")
        {
            oggName.replace(oggName.size() - 4, 4, ".ogg");
        }
        if (std::filesystem::exists("assets/audio/" + oggName))
            return "assets/audio/" + oggName;
        if (std::filesystem::exists("assets/audio/" + name))
            return "assets/audio/" + name;
        return "assets/audio/legacy_archive/" + name;
    }

    bool AudioController::playVoiceEvent(VoiceEvent ev)
    {
        if (m_IsMuted)
            return false;

        std::string path = getAssetPath(ev);
        if (!m_VoiceStream.openFromFile(path))
        {
            std::cerr << "[AUDIO] Ошибка загрузки аудиофайла: " << path << std::endl;
            return false;
        }

        m_VoiceStream.play();
        m_CurrentSubtitle = getSubtitleText(ev);
        m_SubtitleTimer = 7.5f;
        std::cout << "[VOICEOVER] Воспроизведение (" << static_cast<int>(m_VoiceLang) << "): " << path << std::endl;
        return true;
    }

    void AudioController::update(float dt)
    {
        if (m_SubtitleTimer > 0.0f)
        {
            m_SubtitleTimer -= dt;
            if (m_SubtitleTimer <= 0.0f)
                m_CurrentSubtitle = "";
        }
    }

    void AudioController::renderSubtitlesHUD(sf::RenderWindow &window, const sf::Font *font) const
    {
        if (m_CurrentSubtitle.empty() || m_SubtitleTimer <= 0.0f)
            return;

        float W = static_cast<float>(Config::SCREEN_WIDTH);

        sf::RectangleShape bg({W - 200.0f, 32.0f});
        bg.setPosition(100.0f, 40.0f);
        bg.setFillColor(sf::Color(10, 25, 15, 215));
        bg.setOutlineThickness(2.0f);
        bg.setOutlineColor(sf::Color(50, 220, 90));
        window.draw(bg);

        if (font)
        {
            sf::Text txt;
            txt.setFont(*font);
            txt.setString(">> [SUBTITLE] " + m_CurrentSubtitle);
            txt.setCharacterSize(14);
            txt.setFillColor(sf::Color(140, 255, 160));
            txt.setPosition(115.0f, 46.0f);
            window.draw(txt);
        }
    }

    std::string AudioController::getAssetPath(VoiceEvent ev) const
    {
        if (ev == VoiceEvent::TitanActivation)
        {
            switch (m_VoiceLang)
            {
            case VoiceLanguage::EnglishUS:
                return resolveAudioPath("bt_activation_en_us.ogg");
            case VoiceLanguage::Russian:
                return resolveAudioPath("bt_activation_ru_ru.ogg");
            case VoiceLanguage::German:
                return resolveAudioPath("bt_activation_de_de.ogg");
            case VoiceLanguage::EnglishAU:
                return resolveAudioPath("bt_activation_en_au.ogg");
            case VoiceLanguage::EnglishGB:
                return resolveAudioPath("bt_activation_en_gb.ogg");
            }
        }
        else if (ev == VoiceEvent::PipBoyBoot)
        {
            switch (m_VoiceLang)
            {
            case VoiceLanguage::EnglishUS:
                return resolveAudioPath("pipboy_boot_en_us.ogg");
            case VoiceLanguage::Russian:
                return resolveAudioPath("pipboy_boot_ru_ru.ogg");
            case VoiceLanguage::German:
                return resolveAudioPath("pipboy_boot_de_de.ogg");
            case VoiceLanguage::EnglishAU:
                return resolveAudioPath("pipboy_boot_en_au.ogg");
            case VoiceLanguage::EnglishGB:
                return resolveAudioPath("pipboy_boot_ru_ru.ogg");
            }
        }
        else if (ev == VoiceEvent::RaySwarmAlert)
        {
            switch (m_VoiceLang)
            {
            case VoiceLanguage::EnglishUS:
                return resolveAudioPath("ray_alert_en_us.ogg");
            case VoiceLanguage::Russian:
                return resolveAudioPath("ray_alert_ru.ogg");
            case VoiceLanguage::German:
                return resolveAudioPath("ray_alert_de.ogg");
            case VoiceLanguage::EnglishAU:
                return resolveAudioPath("ray_alert_en_au.ogg");
            case VoiceLanguage::EnglishGB:
                return resolveAudioPath("ray_alert_en_gb.ogg");
            }
        }
        else if (ev == VoiceEvent::RaySwarmDestroy)
        {
            switch (m_VoiceLang)
            {
            case VoiceLanguage::EnglishUS:
                return resolveAudioPath("ray_destroy_en_us.ogg");
            case VoiceLanguage::Russian:
                return resolveAudioPath("ray_destroy_ru.ogg");
            case VoiceLanguage::German:
                return resolveAudioPath("ray_destroy_de.ogg");
            case VoiceLanguage::EnglishAU:
                return resolveAudioPath("ray_destroy_en_au.ogg");
            case VoiceLanguage::EnglishGB:
                return resolveAudioPath("ray_destroy_en_gb.ogg");
            }
        }
        return resolveAudioPath("pippad_boot.ogg");
    }

    std::string AudioController::getSubtitleText(VoiceEvent ev) const
    {
        if (ev == VoiceEvent::TitanActivation)
        {
            switch (m_SubLang)
            {
            case VoiceLanguage::EnglishUS:
            case VoiceLanguage::EnglishGB:
                return "BT-7274: System online. Vault 17 isolation confirmed. Pressure at 350 Bar. Welcome aboard, Pilot!";
            case VoiceLanguage::EnglishAU:
                return "BT-7274 (AU): Righto mate. System online. Isolation confirmed. Pressure at 350 Bar. Welcome aboard mate!";
            case VoiceLanguage::Russian:
                return "БТ-7274: Система активирована. Протокол изоляции Убежища 17 подтвержден. Давление 350 Бар. Приветствую!";
            case VoiceLanguage::German:
                return "BT-7274: System online. Bunker 17 Isolationsprotokoll bestätigt. Hydraulikdruck 350 Bar. Willkommen!";
            }
        }
        else if (ev == VoiceEvent::RaySwarmAlert)
        {
            return (m_SubLang == VoiceLanguage::Russian) ? "RAY Дрон: Зафиксирована цель. Боевой режим!" : "RAY Drone: Target acquired. Combat mode engaged!";
        }
        else if (ev == VoiceEvent::RaySwarmDestroy)
        {
            return (m_SubLang == VoiceLanguage::Russian) ? "RAY Дрон: Критический отказ ядра. Деактивация." : "RAY Drone: Core failure. Deactivating.";
        }
        else
        {
            switch (m_SubLang)
            {
            case VoiceLanguage::EnglishUS:
            case VoiceLanguage::EnglishAU:
            case VoiceLanguage::EnglishGB:
                return "Pip-Boy II: Life support online. RobCo network connected. Sanity & Soul stable. Protocol #325-7-2.";
            case VoiceLanguage::Russian:
                return "Пип-Бой II: Прибор запущен. Сеть РобКо подключена. Линии Разума и Души стабильны. Протокол #325-7-2.";
            case VoiceLanguage::German:
                return "Pip-Boy II: Lebenserhaltung online. RobCo Netzwerk verbunden. Verstand & Seele stabil. Protokoll #325-7-2.";
            }
        }
        return "";
    }

} // namespace bunker