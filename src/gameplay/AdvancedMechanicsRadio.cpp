#include "gameplay/AdvancedMechanics.hpp"

namespace bunker
{

    RadioTapeSystem::RadioTapeSystem()
    {
        m_Tapes.push_back({"CRYO_00", "Cryo Locker 00", "Если ты слышишь это - Убежище уже проснулось не по протоколу.",
                           false, false});
        m_Tapes.push_back({"GARAGE_BT", "Garage: BT-7274",
                           "Котел танка холодный. Нужен ручной запуск и синхронизация пилота.", false, false});
        m_Tapes.push_back({"SURFACE_17", "Surface weather",
                           "На поверхности эфирный туман режет дальность сенсоров почти вдвое.", false, false});
        m_Tapes.push_back({"LAB_60S", "RobCo Lab 60s Archive (v.97.A.V)",
                           "Диагностика главного фрейма Убежища 17. Синхронизация аудиобанка Vorbis завершена.", false,
                           false});
        m_Tapes.push_back({"MECH_WAR", "ROY/RAY Swarm War Log",
                           "ROY и RAY - 100% механические самореплицирующиеся дроны. Органика и гнезда исключены.",
                           false, false});
        m_Tapes.push_back({"PORT_16", "Space Port Sector 16",
                           "Шлюз бастиона 16 запечатан. Давление гидравлики БТ-7274 стабильно: 350 Бар.", false,
                           false});
        m_Tapes.push_back({"BATTLE_215", "Orbital Battle Log 215",
                           "Температура хладагента танка 180°C. При пороге в 250°C сработает аварийный сброс пара.",
                           false, false});

        m_Radio.push_back({4.0f, "V17", "...прием... Башня молчит. Найдите Pip-Pad и синхронизируйтесь.", false});
        m_Radio.push_back(
            {18.0f, "BT", "Пилот, корпус поврежден. Ангарные ремкомплекты восстановят подсистемы.", false});
        m_Radio.push_back({45.0f, "LAN", "Локальная сеть Lanline доступна: чат, заказы, доставки, отряд.", false});
    }

    void RadioTapeSystem::update(float dt)
    {
        m_Time += dt;
        m_LastSubtitle.clear();
        for (auto& msg : m_Radio)
        {
            if (!msg.fired && m_Time >= msg.atTime)
            {
                msg.fired = true;
                m_LastSubtitle = "[" + msg.channel + "] " + msg.text;
                m_Log.push_back(m_LastSubtitle);
                if (m_Log.size() > 12)
                {
                    m_Log.pop_front();
                }
                break;
            }
        }
    }

    void RadioTapeSystem::discoverTape(const std::string& id)
    {
        for (auto& tape : m_Tapes)
        {
            if (tape.id == id)
            {
                tape.found = true;
                m_LastSubtitle = "[TAPE FOUND] " + tape.title;
                m_Log.push_back(m_LastSubtitle);
                return;
            }
        }
    }

    std::string RadioTapeSystem::playNextUnplayed()
    {
        for (auto& tape : m_Tapes)
        {
            if (tape.found && !tape.played)
            {
                tape.played = true;
                m_LastSubtitle = "[TAPE] " + tape.title + ": " + tape.transcript;
                m_Log.push_back(m_LastSubtitle);
                return m_LastSubtitle;
            }
        }
        return "[TAPE] Нет новых записей.";
    }

} // namespace bunker
