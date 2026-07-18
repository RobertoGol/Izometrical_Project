#include "gameplay/AdvancedMechanics.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace bunker
{

    void ObjModel::computeFlatNormalsIfMissing()
    {
        if (!normals.empty() || vertices.empty() || faces.empty())
        {
            return;
        }
        for (auto& f : faces)
        {
            const int i0 = f.v[0];
            const int i1 = f.v[1];
            const int i2 = f.v[2];
            if (i0 < 0 || i0 >= static_cast<int>(vertices.size()) || i1 < 0 ||
                i1 >= static_cast<int>(vertices.size()) || i2 < 0 || i2 >= static_cast<int>(vertices.size()))
            {
                continue;
            }
            const auto& v0 = vertices[i0];
            const auto& v1 = vertices[i1];
            const auto& v2 = vertices[i2];
            const float ux = v1.x - v0.x;
            const float uy = v1.y - v0.y;
            const float uz = v1.z - v0.z;
            const float vx = v2.x - v0.x;
            const float vy = v2.y - v0.y;
            const float vz = v2.z - v0.z;
            float nx = uy * vz - uz * vy;
            float ny = uz * vx - ux * vz;
            float nz = ux * vy - uy * vx;
            const float len = std::sqrt(nx * nx + ny * ny + nz * nz);
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
            const int normIdx = static_cast<int>(normals.size());
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

        const std::size_t p1 = token.find('/');
        if (p1 == std::string::npos)
        {
            v_idx = std::atoi(token.c_str()) - 1;
            return;
        }

        v_idx = std::atoi(token.substr(0, p1).c_str()) - 1;
        const std::size_t p2 = token.find('/', p1 + 1);
        if (p2 == std::string::npos)
        {
            const std::string t = token.substr(p1 + 1);
            if (!t.empty())
            {
                vt_idx = std::atoi(t.c_str()) - 1;
            }
            return;
        }

        const std::string t1 = token.substr(p1 + 1, p2 - (p1 + 1));
        if (!t1.empty())
        {
            vt_idx = std::atoi(t1.c_str()) - 1;
        }

        const std::string t2 = token.substr(p2 + 1);
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

} // namespace bunker
