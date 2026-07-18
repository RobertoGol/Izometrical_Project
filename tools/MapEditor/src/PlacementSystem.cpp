#include "PlacementSystem.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

PlacementSystem::PlacementSystem() {}

void PlacementSystem::PlaceWall(int x, int y, const std::string &material, int height, int layer,
                                const std::vector<std::string> &mods)
{
    RemoveObject(x, y, layer);

    PlacedObject wall;
    wall.id = "wall";
    wall.category = PlacementCategory::Wall;
    wall.x = x;
    wall.y = y;
    wall.height = height;
    wall.material = material;
    wall.mods = mods;
    wall.connectionMask = 0;
    wall.layer = layer;

    objects.push_back(wall);
    UpdateWallConnections(x, y);
}

void PlacementSystem::PlaceProp(int x, int y, const std::string &id, int layer, float rotation)
{
    RemoveObject(x, y, layer);

    PlacedObject prop;
    prop.id = id;
    prop.category = PlacementCategory::Prop;
    prop.x = x;
    prop.y = y;
    prop.rotation = rotation;
    prop.layer = layer;

    objects.push_back(prop);
}

void PlacementSystem::PlaceTile(int x, int y, const std::string &tileType, int layer)
{
    RemoveObject(x, y, layer);

    PlacedObject tile;
    tile.id = "tile_" + tileType;
    tile.category = PlacementCategory::Tile;
    tile.x = x;
    tile.y = y;
    tile.layer = layer;

    objects.push_back(tile);
}

std::string PlacementSystem::GetWallTexture(const PlacedObject &wall) const
{
    std::string texture = wall.id;

    if (!wall.material.empty())
        texture += "_" + wall.material;

    for (const auto &mod : wall.mods)
    {
        if (!mod.empty() && mod != "clean")
            texture += "_" + mod;
    }

    if (wall.connectionMask != 0)
        texture += "_connected";

    return texture + ".png";
}

std::string PlacementSystem::GetPropTexture(const PlacedObject &prop) const
{
    return prop.id + ".png";
}

void PlacementSystem::SnapToGrid(int &x, int &y)
{
    if (x < 0)
        x = 0;
    if (x >= 32)
        x = 31;
    if (y < 0)
        y = 0;
    if (y >= 32)
        y = 31;
}

void PlacementSystem::SnapToWall(int x, int y, int &outX, int &outY)
{
    outX = x;
    outY = y;

    for (const auto &obj : objects)
    {
        if (obj.category != PlacementCategory::Wall)
        {
            continue;
        }

        if (std::abs(obj.x - x) <= 1 && std::abs(obj.y - y) <= 1)
        {
            outX = obj.x;
            outY = obj.y;
            return;
        }
    }
}

void PlacementSystem::UpdateWallConnections(int x, int y)
{
    auto updateSingleCell = [this](int cx, int cy)
    {
        for (auto &obj : objects)
        {
            if (obj.category != PlacementCategory::Wall || obj.x != cx || obj.y != cy)
            {
                continue;
            }

            obj.connectionMask = 0;
            for (const auto &other : objects)
            {
                if (other.category != PlacementCategory::Wall)
                {
                    continue;
                }

                if (other.x == cx && other.y == cy - 1)
                    obj.connectionMask |= PlacementSystem::NORTH;
                if (other.x == cx && other.y == cy + 1)
                    obj.connectionMask |= PlacementSystem::SOUTH;
                if (other.x == cx - 1 && other.y == cy)
                    obj.connectionMask |= PlacementSystem::WEST;
                if (other.x == cx + 1 && other.y == cy)
                    obj.connectionMask |= PlacementSystem::EAST;
            }
        }
    };

    updateSingleCell(x, y);
    updateSingleCell(x, y - 1);
    updateSingleCell(x, y + 1);
    updateSingleCell(x - 1, y);
    updateSingleCell(x + 1, y);
}

PlacedObject *PlacementSystem::GetObjectAt(int x, int y, int layer)
{
    for (auto &obj : objects)
    {
        if (obj.x == x && obj.y == y && (layer == -1 || obj.layer == layer))
            return &obj;
    }
    return nullptr;
}

const std::vector<PlacedObject> &PlacementSystem::GetAllObjects() const
{
    return objects;
}

void PlacementSystem::RemoveObject(int x, int y, int layer)
{
    objects.erase(
        std::remove_if(objects.begin(), objects.end(),
                       [x, y, layer](const PlacedObject &obj)
                       {
                           return obj.x == x && obj.y == y && (layer == -1 || obj.layer == layer);
                       }),
        objects.end());

    UpdateWallConnections(x, y);
}

bool PlacementSystem::SaveToFile(const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
        return false;

    for (const auto &obj : objects)
    {
        file << static_cast<int>(obj.category) << " "
             << obj.x << " " << obj.y << " "
             << obj.layer << " " << obj.height << " " << obj.rotation << " "
             << (obj.id.empty() ? "NONE" : obj.id) << " "
             << (obj.material.empty() ? "NONE" : obj.material) << " ";

        if (obj.mods.empty())
            file << "NONE";
        else
        {
            for (size_t i = 0; i < obj.mods.size(); ++i)
            {
                file << obj.mods[i] << (i == obj.mods.size() - 1 ? "" : ",");
            }
        }
        file << "\n";
    }
    return true;
}

bool PlacementSystem::LoadFromFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;

    objects.clear();
    int catInt, x, y, layer, height;
    float rotation;
    std::string id, material, modsStr;

    while (file >> catInt >> x >> y >> layer >> height >> rotation >> id >> material >> modsStr)
    {
        PlacedObject obj;
        obj.category = static_cast<PlacementCategory>(catInt);
        obj.x = x;
        obj.y = y;
        obj.layer = layer;
        obj.height = height;
        obj.rotation = rotation;
        obj.id = (id == "NONE" ? "" : id);
        obj.material = (material == "NONE" ? "" : material);

        if (modsStr != "NONE")
        {
            std::stringstream ss(modsStr);
            std::string item;
            while (std::getline(ss, item, ','))
            {
                obj.mods.push_back(item);
            }
        }
        objects.push_back(obj);
    }

    for (const auto &obj : objects)
    {
        if (obj.category == PlacementCategory::Wall)
            UpdateWallConnections(obj.x, obj.y);
    }
    return true;
}
