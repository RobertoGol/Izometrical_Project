#include "engine/AudioController.hpp"
#include "core/Constants.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <filesystem>
#include <map>
#include <utility>

namespace bunker
{

    static std::string resolveAudioPath(const std::string& name)
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
            bunker::logError() << "[AUDIO] Ошибка загрузки аудиофайла: " << path << std::endl;
            return false;
        }

        m_VoiceStream.play();
        m_CurrentSubtitle = getSubtitleText(ev);
        m_SubtitleTimer = 7.5f;
        bunker::logInfo() << "[VOICEOVER] Воспроизведение (" << static_cast<int>(m_VoiceLang) << "): " << path
                          << std::endl;
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

    void AudioController::renderSubtitlesHUD(sf::RenderWindow& window, const sf::Font* font) const
    {
        if (m_CurrentSubtitle.empty() || m_SubtitleTimer <= 0.0f)
            return;

        float W = static_cast<float>(Config::SCREEN_WIDTH);

        sf::RectangleShape bg({W - 200.0f, 32.0f});
        bg.setPosition({100.0f, 40.0f});
        bg.setFillColor(sf::Color(10, 25, 15, 215));
        bg.setOutlineThickness(2.0f);
        bg.setOutlineColor(sf::Color(50, 220, 90));
        window.draw(bg);

        if (font)
        {
            sf::Text txt(*font, ">> [SUBTITLE] " + m_CurrentSubtitle, 14);
            txt.setFillColor(sf::Color(140, 255, 160));
            txt.setPosition({115.0f, 46.0f});
            window.draw(txt);
        }
    }

    std::string AudioController::getAssetPath(VoiceEvent ev) const
    {
        using AssetKey = std::pair<VoiceEvent, VoiceLanguage>;
        static const std::map<AssetKey, const char*> assetPaths = {
            {{VoiceEvent::TitanActivation, VoiceLanguage::EnglishUS}, "bt_activation_en_us.ogg"},
            {{VoiceEvent::TitanActivation, VoiceLanguage::Russian}, "bt_activation_ru_ru.ogg"},
            {{VoiceEvent::TitanActivation, VoiceLanguage::German}, "bt_activation_de_de.ogg"},
            {{VoiceEvent::TitanActivation, VoiceLanguage::EnglishAU}, "bt_activation_en_au.ogg"},
            {{VoiceEvent::TitanActivation, VoiceLanguage::EnglishGB}, "bt_activation_en_gb.ogg"},
            {{VoiceEvent::PipBoyBoot, VoiceLanguage::EnglishUS}, "pipboy_boot_en_us.ogg"},
            {{VoiceEvent::PipBoyBoot, VoiceLanguage::Russian}, "pipboy_boot_ru_ru.ogg"},
            {{VoiceEvent::PipBoyBoot, VoiceLanguage::German}, "pipboy_boot_de_de.ogg"},
            {{VoiceEvent::PipBoyBoot, VoiceLanguage::EnglishAU}, "pipboy_boot_en_au.ogg"},
            {{VoiceEvent::PipBoyBoot, VoiceLanguage::EnglishGB}, "pipboy_boot_ru_ru.ogg"},
            {{VoiceEvent::RaySwarmAlert, VoiceLanguage::EnglishUS}, "ray_alert_en_us.ogg"},
            {{VoiceEvent::RaySwarmAlert, VoiceLanguage::Russian}, "ray_alert_ru.ogg"},
            {{VoiceEvent::RaySwarmAlert, VoiceLanguage::German}, "ray_alert_de.ogg"},
            {{VoiceEvent::RaySwarmAlert, VoiceLanguage::EnglishAU}, "ray_alert_en_au.ogg"},
            {{VoiceEvent::RaySwarmAlert, VoiceLanguage::EnglishGB}, "ray_alert_en_gb.ogg"},
            {{VoiceEvent::RaySwarmDestroy, VoiceLanguage::EnglishUS}, "ray_destroy_en_us.ogg"},
            {{VoiceEvent::RaySwarmDestroy, VoiceLanguage::Russian}, "ray_destroy_ru.ogg"},
            {{VoiceEvent::RaySwarmDestroy, VoiceLanguage::German}, "ray_destroy_de.ogg"},
            {{VoiceEvent::RaySwarmDestroy, VoiceLanguage::EnglishAU}, "ray_destroy_en_au.ogg"},
            {{VoiceEvent::RaySwarmDestroy, VoiceLanguage::EnglishGB}, "ray_destroy_en_gb.ogg"},
        };

        const auto asset = assetPaths.find({ev, m_VoiceLang});
        return resolveAudioPath(asset == assetPaths.end() ? "pippad_boot.ogg" : asset->second);
    }

    std::string AudioController::getSubtitleText(VoiceEvent ev) const
    {
        using SubtitleKey = std::pair<VoiceEvent, VoiceLanguage>;
        static const std::map<SubtitleKey, const char*> subtitles = {
            {{VoiceEvent::TitanActivation, VoiceLanguage::EnglishUS},
             "BT-7274: System online. Vault 17 isolation confirmed. Pressure at 350 Bar. Welcome aboard, Pilot!"},
            {{VoiceEvent::TitanActivation, VoiceLanguage::EnglishGB},
             "BT-7274: System online. Vault 17 isolation confirmed. Pressure at 350 Bar. Welcome aboard, Pilot!"},
            {{VoiceEvent::TitanActivation, VoiceLanguage::EnglishAU},
             "BT-7274 (AU): Righto mate. System online. Isolation confirmed. Pressure at 350 Bar. Welcome aboard "
             "mate!"},
            {{VoiceEvent::TitanActivation, VoiceLanguage::Russian},
             "Р‘Рў-7274: РЎРёСЃС‚РµРјР° Р°РєС‚РёРІРёСЂРѕРІР°РЅР°. РџСЂРѕС‚РѕРєРѕР» РёР·РѕР»СЏС†РёРё РЈР±РµР¶РёС‰Р° 17 "
             "РїРѕРґС‚РІРµСЂР¶РґРµРЅ. Р”Р°РІР»РµРЅРёРµ 350 Р‘Р°СЂ. РџСЂРёРІРµС‚СЃС‚РІСѓСЋ!"},
            {{VoiceEvent::TitanActivation, VoiceLanguage::German},
             "BT-7274: System online. Bunker 17 Isolationsprotokoll bestГ¤tigt. Hydraulikdruck 350 Bar. Willkommen!"},
            {{VoiceEvent::RaySwarmAlert, VoiceLanguage::Russian},
             "RAY Р”СЂРѕРЅ: Р—Р°С„РёРєСЃРёСЂРѕРІР°РЅР° С†РµР»СЊ. Р‘РѕРµРІРѕР№ СЂРµР¶РёРј!"},
            {{VoiceEvent::RaySwarmDestroy, VoiceLanguage::Russian},
             "RAY Р”СЂРѕРЅ: РљСЂРёС‚РёС‡РµСЃРєРёР№ РѕС‚РєР°Р· СЏРґСЂР°. Р”РµР°РєС‚РёРІР°С†РёСЏ."},
            {{VoiceEvent::PipBoyBoot, VoiceLanguage::Russian},
             "РџРёРї-Р‘РѕР№ II: РџСЂРёР±РѕСЂ Р·Р°РїСѓС‰РµРЅ. РЎРµС‚СЊ Р РѕР±РљРѕ РїРѕРґРєР»СЋС‡РµРЅР°. Р›РёРЅРёРё "
             "Р Р°Р·СѓРјР° Рё Р”СѓС€Рё СЃС‚Р°Р±РёР»СЊРЅС‹. РџСЂРѕС‚РѕРєРѕР» #325-7-2."},
            {{VoiceEvent::PipPadBoot, VoiceLanguage::Russian},
             "РџРёРї-Р‘РѕР№ II: РџСЂРёР±РѕСЂ Р·Р°РїСѓС‰РµРЅ. РЎРµС‚СЊ Р РѕР±РљРѕ РїРѕРґРєР»СЋС‡РµРЅР°. Р›РёРЅРёРё "
             "Р Р°Р·СѓРјР° Рё Р”СѓС€Рё СЃС‚Р°Р±РёР»СЊРЅС‹. РџСЂРѕС‚РѕРєРѕР» #325-7-2."},
        };

        const auto subtitle = subtitles.find({ev, m_SubLang});
        if (subtitle != subtitles.end())
            return subtitle->second;

        if (ev == VoiceEvent::RaySwarmAlert)
            return "RAY Drone: Target acquired. Combat mode engaged!";
        if (ev == VoiceEvent::RaySwarmDestroy)
            return "RAY Drone: Core failure. Deactivating.";

        return "Pip-Boy II: Life support online. RobCo network connected. Sanity & Soul stable. Protocol #325-7-2.";
    }

} // namespace bunker
