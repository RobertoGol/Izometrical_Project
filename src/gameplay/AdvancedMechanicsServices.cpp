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

{
    LanlineServices::LanlineServices() {
        m_UdpSocket.setBlocking(false);
    }

    LanlineServices::~LanlineServices() {
    disconnect();
    }

        bool LanlineServices::hostGame(unsigned short port) {
        if (m_Role != NetRole::Offline) disconnect();
        
        if (m_UdpSocket.bind(port) != sf::Socket::Done) return false;
        if (m_TcpListener.listen(port) != sf::Socket::Done) {
            m_UdpSocket.unbind();
            return false;
        }

        m_TcpListener.setBlocking(false);
        m_Selector.add(m_UdpSocket);
        m_Selector.add(m_TcpListener);

        m_Role = NetRole::Host;
        m_LocalPeerId = 1;
        m_Connected = true;
        systemMessage("HOST: P2P Session started. Waiting for pilots on port " + std::to_string(port));
        return true;
    }

    bool LanlineServices::joinGame(const std::string& hostIp, unsigned short port, const std::string& playerName) {
        if (m_Role != NetRole::Offline) disconnect();
        m_LocalPlayerName = playerName;

        if (m_UdpSocket.bind(sf::Socket::AnyPort) != sf::Socket::Done) return false;
        m_LocalUdpPort = m_UdpSocket.getLocalPort();

        if (m_HostTcpSocket.connect(sf::IpAddress(hostIp), port, sf::seconds(5)) != sf::Socket::Done) {
            m_UdpSocket.unbind();
            return false;
        }
        
        m_HostTcpSocket.setBlocking(false);
        m_Selector.add(m_UdpSocket);
        m_Selector.add(m_HostTcpSocket);

        sf::Packet handshake;
        handshake << static_cast<std::uint8_t>(PacketType::Handshake) 
                << static_cast<std::uint16_t>(m_LocalUdpPort) 
                << playerName;
        m_HostTcpSocket.send(handshake);

        m_Role = NetRole::Client;
        m_Connected = true;
        systemMessage("CLIENT: Successfully connected to Host " + hostIp);
        return true;
    }

    void LanlineServices::disconnect() {
        if (m_Role == NetRole::Offline) return;
        
        m_TcpListener.close();
        m_HostTcpSocket.disconnect();
        m_UdpSocket.unbind();
        m_Selector.clear();
        
        for(auto& p : m_Peers) {
            if(p.tcpSocket) { delete p.tcpSocket; p.tcpSocket = nullptr; }
        }
        m_Peers.clear();
        
        m_Role = NetRole::Offline;
        m_Connected = false;
        systemMessage("Lanline connection terminated.");
    }

    void LanlineServices::update(GameState& gs, PlayerInventory& inv, float dt) {
        gs.isNetworkGame = m_Connected;
        gs.netRole = m_Role;
        gs.localPeerId = m_LocalPeerId;

        if (m_Role == NetRole::Offline) return;

        if (m_Selector.wait(sf::milliseconds(1))) {
            if (m_Selector.isReady(m_UdpSocket)) {
                processUdpPacket(gs);
            }
            if (isHost() && m_Selector.isReady(m_TcpListener)) {
                acceptNewTcpClient();
            }
            for (auto& peer : m_Peers) {
                if (peer.tcpSocket && m_Selector.isReady(*peer.tcpSocket)) {
                    processTcpPacket(peer, gs);
                }
            }
        }
    }

    void LanlineServices::acceptNewTcpClient() {
        auto* clientSock = new sf::TcpSocket();
        if (m_TcpListener.accept(*clientSock) == sf::Socket::Done) {
            clientSock->setBlocking(false);
            m_Selector.add(*clientSock);

            RemotePlayer p;
            p.id = m_NextPeerId++;
            p.tcpSocket = clientSock;
            p.ip = clientSock->getRemoteAddress();
            m_Peers.push_back(p);
            
            systemMessage("Incoming TCP connection established.");
        } else {
            delete clientSock;
        }
    }

    void LanlineServices::processTcpPacket(RemotePlayer& peer, GameState& gs) {
        sf::Packet pkt;
        if (peer.tcpSocket->receive(pkt) == sf::Socket::Done) {
            std::uint8_t typeRaw;
            pkt >> typeRaw;
            if (static_cast<PacketType>(typeRaw) == PacketType::Handshake) {
                std::uint16_t udpPort;
                std::string pName;
                pkt >> udpPort >> pName;
                peer.udpPort = udpPort;
                peer.name = pName;
                systemMessage("Pilot " + pName + " joined the session.");
                
                sf::Packet ack;
                ack << static_cast<std::uint8_t>(PacketType::HandshakeAck) << static_cast<std::uint16_t>(peer.id);
                peer.tcpSocket->send(ack);
            }
        }
    }

    void LanlineServices::processUdpPacket(GameState& gs) {
        sf::IpAddress sender;
        unsigned short senderPort;
        sf::Packet pkt;

        while (m_UdpSocket.receive(pkt, sender, senderPort) == sf::Socket::Done) {
            std::uint8_t rawType;
            if (!(pkt >> rawType)) continue;
            PacketType type = static_cast<PacketType>(rawType);

            if (type == PacketType::PlayerState) {
                PlayerNetState s;
                pkt >> s.peerId >> s.x >> s.y >> s.z >> s.vx >> s.vy >> s.vz >> s.health >> s.playerMode >> s.facingAngle;

                auto it = std::find_if(gs.remotePlayers.begin(), gs.remotePlayers.end(), [&](const RemotePlayer& p){ return p.id == s.peerId; });
                if (it != gs.remotePlayers.end()) {
                    it->position = {s.x, s.y, s.z};
                    it->velocity = {s.vx, s.vy, s.vz};
                    it->health = s.health;
                    it->facingAngle = s.facingAngle;
                    it->mode = static_cast<UnitMode>(s.playerMode);
                } else {
                    RemotePlayer newRp;
                    newRp.id = s.peerId;
                    newRp.position = {s.x, s.y, s.z};
                    newRp.health = s.health;
                    gs.remotePlayers.push_back(newRp);
                }

                if (isHost()) {
                    sf::Packet relayPkt;
                    relayPkt << rawType << s.peerId << s.x << s.y << s.z << s.vx << s.vy << s.vz << s.health << s.playerMode << s.facingAngle;
                    broadcastUdp(relayPkt, s.peerId);
                }
            }
        }
    }

    void LanlineServices::sendPlayerState(const Vector3D& pos, const Vector3D& vel, float health, UnitMode mode, float facing) {
        if (m_Role == NetRole::Offline) return;

        sf::Packet pkt;
        pkt << static_cast<std::uint8_t>(PacketType::PlayerState);
        pkt << static_cast<std::uint16_t>(m_LocalPeerId);
        pkt << pos.x << pos.y << pos.z << vel.x << vel.y << vel.vz << health << static_cast<std::uint8_t>(mode) << facing;

        if (isHost()) {
            broadcastUdp(pkt, m_LocalPeerId);
        } else if (m_HostTcpSocket.getRemoteAddress() != sf::IpAddress::None) {
            m_UdpSocket.send(pkt, m_HostTcpSocket.getRemoteAddress(), DEFAULT_PORT);
        }
    }

    void LanlineServices::broadcastUdp(sf::Packet& packet, int excludePeerId) {
        for (const auto& peer : m_Peers) {
            if (peer.id != excludePeerId && peer.udpPort != 0) {
                m_UdpSocket.send(packet, peer.ip, peer.udpPort);
            }
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
