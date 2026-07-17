#include "gameplay/AdvancedMechanics.hpp"
#include "engine/Log.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace bunker
{

    // ═══════════════════════════════════════════════════════════════════════════════
    // 12) OBJ MODEL LOADER
    // ═══════════════════════════════════════════════════════════════════════════════

    void ObjModel::computeFlatNormalsIfMissing()
    {
        if (!normals.empty() || vertices.empty() || faces.empty())
        {
            return;
        }
        for (auto& f : faces)
        {
            int i0 = f.v[0];
            int i1 = f.v[1];
            int i2 = f.v[2];
            if (i0 < 0 || i0 >= static_cast<int>(vertices.size()) || i1 < 0 ||
                i1 >= static_cast<int>(vertices.size()) || i2 < 0 || i2 >= static_cast<int>(vertices.size()))
            {
                continue;
            }
            const auto& v0 = vertices[i0];
            const auto& v1 = vertices[i1];
            const auto& v2 = vertices[i2];
            float ux = v1.x - v0.x, uy = v1.y - v0.y, uz = v1.z - v0.z;
            float vx = v2.x - v0.x, vy = v2.y - v0.y, vz = v2.z - v0.z;
            float nx = uy * vz - uz * vy;
            float ny = uz * vx - ux * vz;
            float nz = ux * vy - uy * vx;
            float len = std::sqrt(nx * nx + ny * ny + nz * nz);
            if (len > 0.0001f)
            {
                nx /= len;
                ny /= len;
                nz /= len;
            }
            else
            {
                nz = 1.0f;
            }
            int normIdx = static_cast<int>(normals.size());
            normals.push_back({nx, ny, nz});
            f.vn = {{normIdx, normIdx, normIdx}};
        }
    }

    void ObjModelLoader::parseFullFaceTriplet(const std::string& token, int& v_idx, int& vt_idx, int& vn_idx)
    {
        v_idx = -1;
        vt_idx = -1;
        vn_idx = -1;
        if (token.empty())
        {
            return;
        }

        std::size_t p1 = token.find('/');
        if (p1 == std::string::npos)
        {
            v_idx = std::atoi(token.c_str()) - 1;
            return;
        }

        v_idx = std::atoi(token.substr(0, p1).c_str()) - 1;
        std::size_t p2 = token.find('/', p1 + 1);
        if (p2 == std::string::npos)
        {
            std::string t = token.substr(p1 + 1);
            if (!t.empty())
            {
                vt_idx = std::atoi(t.c_str()) - 1;
            }
            return;
        }

        std::string t1 = token.substr(p1 + 1, p2 - (p1 + 1));
        if (!t1.empty())
        {
            vt_idx = std::atoi(t1.c_str()) - 1;
        }

        std::string t2 = token.substr(p2 + 1);
        if (!t2.empty())
        {
            vn_idx = std::atoi(t2.c_str()) - 1;
        }
    }

    ObjModel ObjModelLoader::load(const std::string& path)
    {
        ObjModel model;
        std::ifstream in(path);
        if (!in)
        {
            return model;
        }
        std::string line;
        while (std::getline(in, line))
        {
            std::istringstream ss(line);
            std::string tag;
            ss >> tag;
            if (tag == "v")
            {
                ObjVertex v;
                ss >> v.x >> v.y >> v.z;
                model.vertices.push_back(v);
            }
            else if (tag == "vt")
            {
                ObjTexCoord tc;
                ss >> tc.u >> tc.v;
                model.texCoords.push_back(tc);
            }
            else if (tag == "vn")
            {
                ObjNormal n;
                ss >> n.nx >> n.ny >> n.nz;
                model.normals.push_back(n);
            }
            else if (tag == "f")
            {
                std::array<int, 3> v_idx{{-1, -1, -1}};
                std::array<int, 3> vt_idx{{-1, -1, -1}};
                std::array<int, 3> vn_idx{{-1, -1, -1}};
                for (int i = 0; i < 3; ++i)
                {
                    std::string token;
                    ss >> token;
                    parseFullFaceTriplet(token, v_idx[i], vt_idx[i], vn_idx[i]);
                    if (v_idx[i] < 0)
                    {
                        v_idx[i] = parseFaceIndex(token) - 1;
                    }
                }
                if (v_idx[0] >= 0 && v_idx[1] >= 0 && v_idx[2] >= 0)
                {
                    ObjFace face;
                    face.v = v_idx;
                    face.vt = vt_idx;
                    face.vn = vn_idx;
                    model.faces.push_back(face);
                }
            }
        }
        model.computeFlatNormalsIfMissing();
        return model;
    }

    int ObjModelLoader::parseFaceIndex(const std::string& token)
    {
        std::string n;
        for (char c : token)
        {
            if (c == '/')
            {
                break;
            }
            n.push_back(c);
        }
        if (n.empty())
        {
            return 0;
        }
        return std::max(0, std::atoi(n.c_str()));
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 13) LANLINE SERVICES
    // ═══════════════════════════════════════════════════════════════════════════════

    int LanlineServices::createLocalLobby(const std::string& playerName)
    {
        m_Peers.clear();
        m_Deliveries.clear();
        m_Chat.clear();
        m_Connected = true;
        m_LobbyId = ++m_NextLobbyId;
        addPeerWithSocket(playerName.empty() ? "Solo_Pilot" : playerName, {10.0f, 10.0f, 0.0f}, ++m_NextSocketFd);
        addPeerWithSocket("Gunner_BT7274", {11.0f, 10.0f, 0.0f}, ++m_NextSocketFd);
        addPeerWithSocket("Scout_LogHorizon", {12.0f, 12.0f, 0.0f}, ++m_NextSocketFd);
        addPeerWithSocket("Vault17_Quartermaster", {15.0f, 8.0f, 0.0f}, ++m_NextSocketFd);

        const char* coopNames[] = {"Ranger_Kodiak", "Tech_Valerie",   "Heavy_Goliath",  "Medic_Mercy",
                                   "Sniper_Ghost",  "Engineer_Spark", "Recon_Viper",    "Trooper_Blaze",
                                   "Sapper_Boom",   "Sentinel_Apex",  "Commando_Rex",   "Guardian_Shield",
                                   "Warden_Frost",  "Striker_Bolt",   "Vanguard_Storm", "Overseer_Vault17"};
        for (int i = 0; i < 16 && static_cast<int>(m_Peers.size()) < MAX_COOP_PLAYERS; ++i)
        {
            float px = 8.0f + static_cast<float>((i * 5) % 40);
            float py = 8.0f + static_cast<float>((i * 7) % 35);
            addPeerWithSocket(coopNames[i], {px, py, 0.0f}, ++m_NextSocketFd);
        }

        systemMessage("LANLINE Winsock net #1001 connected. " + std::to_string(m_Peers.size()) +
                      " squad combatants online.");
        return m_LobbyId;
    }

    int LanlineServices::addPeer(const std::string& name)
    {
        return addPeerWithSocket(name, {10.0f, 10.0f, 0.0f}, ++m_NextSocketFd);
    }

    int LanlineServices::addPeerWithSocket(const std::string& name, Vector3D pos, unsigned int sockFd)
    {
        LanlinePeer p;
        p.id = ++m_NextPeerId;
        p.name = name;
        p.lastKnownPos = pos;
        p.simulatedSocketFd = sockFd;
        p.inInterestArea = true;
        m_Peers.push_back(p);
        return p.id;
    }

    void LanlineServices::cullInactiveOrDistantPeers(const Vector3D& localPlayerPos, float interestRadius)
    {
        float rSq = interestRadius * interestRadius;
        for (auto& p : m_Peers)
        {
            float dx = p.lastKnownPos.x - localPlayerPos.x;
            float dy = p.lastKnownPos.y - localPlayerPos.y;
            p.inInterestArea = (dx * dx + dy * dy <= rSq);
        }
    }

    void LanlineServices::simulateWinsockUdpHeartbeat(float dt)
    {
        m_HeartbeatTimer += dt;
        if (m_HeartbeatTimer >= 1.0f)
        {
            m_HeartbeatTimer = 0.0f;
            for (auto& p : m_Peers)
            {
                if (p.simulatedSocketFd > 0)
                {
                    p.lastKnownPos.x += (std::rand() % 3 - 1) * 0.1f;
                    p.lastKnownPos.y += (std::rand() % 3 - 1) * 0.1f;
                }
            }
        }
    }

    void LanlineServices::sendChat(int fromPeer, const std::string& text)
    {
        if (!m_Connected)
        {
            return;
        }
        m_Chat.push_back({fromPeer, text, 10.0f});
        if (m_Chat.size() > 20)
        {
            m_Chat.erase(m_Chat.begin());
        }
    }

    void LanlineServices::setVoice(int peerId, bool active)
    {
        for (auto& p : m_Peers)
        {
            if (p.id == peerId)
            {
                p.voiceActive = active;
            }
        }
    }

    int LanlineServices::requestDelivery(const std::string& payload, Vector3D dropPos)
    {
        LanlineDelivery d;
        d.id = ++m_NextDeliveryId;
        d.payload = payload;
        d.dropPos = dropPos;
        d.eta = 8.0f + static_cast<float>((m_NextDeliveryId * 7) % 6);
        m_Deliveries.push_back(d);
        return d.id;
    }

    void LanlineServices::update(GameState& gs, PlayerInventory& inv, float dt)
    {
        simulateWinsockUdpHeartbeat(dt);
        cullInactiveOrDistantPeers(gs.playerPos, 40.0f);

        for (auto& c : m_Chat)
        {
            c.ttl -= dt;
        }
        m_Chat.erase(
            std::remove_if(m_Chat.begin(), m_Chat.end(), [](const LanlineChatMessage& c) { return c.ttl <= 0.0f; }),
            m_Chat.end());

        for (auto& d : m_Deliveries)
        {
            if (d.delivered)
            {
                continue;
            }
            d.eta -= dt;
            if (d.eta <= 0.0f)
            {
                d.delivered = true;
                if (d.payload == "ammo")
                {
                    inv.addItem(SurvivalSystem::ITEM_AMMO_556, ItemType::Ammo, 2, 0.15f, "5.56 AMMO BOX");
                }
                else if (d.payload == "med")
                {
                    inv.addItem(SurvivalSystem::ITEM_STIM, ItemType::Medicine, 1, 0.10f, "STIM INJECTOR");
                }
                else
                {
                    inv.addItem(201, ItemType::Resource, 10, 0.08f, "SCRAP METAL");
                }
                gs.score += 25;
                systemMessage("Delivery arrived: " + d.payload);
            }
        }

        static float s_ChatTimer = 15.0f;
        s_ChatTimer -= dt;
        if (s_ChatTimer <= 0.0f && m_Peers.size() > 1)
        {
            s_ChatTimer = 30.0f;
            sendChat(m_Peers[1].id, "Сенсоры бастиона в норме. Готов прикрыть огнём.");
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════════
    // 14) PROFILE / SESSION MIGRATION
    // ═══════════════════════════════════════════════════════════════════════════════

    void ProfileSessionSystem::startSession(PlayerProfile& p)
    {
        migrate(p);
        p.sessionsPlayed += 1;
        m_SessionTime = 0.0f;
        m_Started = true;
    }

    void ProfileSessionSystem::update(PlayerProfile& p, float dt)
    {
        if (!m_Started)
        {
            return;
        }
        m_SessionTime += dt;
        p.totalPlayTime += dt;
    }

    void ProfileSessionSystem::migrate(PlayerProfile& p)
    {
        if (p.version < 15)
        {
            p.flags["legacy_import"] = 1;
            p.version = 15;
        }
        if (p.version < 16)
        {
            p.flags["advanced_mechanics_enabled"] = 1;
            p.version = 16;
        }
    }

    std::string ProfileSessionSystem::summary(const PlayerProfile& p) const
    {
        std::ostringstream ss;
        ss << "PROFILE v" << p.version << " name=" << p.playerName << " kills=" << p.totalKills
           << " deaths=" << p.totalDeaths << " sessions=" << p.sessionsPlayed << " playtime=" << std::fixed
           << std::setprecision(1) << p.totalPlayTime;
        return ss.str();
    }

    // ═══════════════════════════════════════════════════════════════════════════════

} // namespace bunker
