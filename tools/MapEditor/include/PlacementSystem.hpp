#pragma once

#include <string>
#include <vector>

enum class PlacementCategory
{
    Tile,
    Wall,
    Prop,
    Entity
};

struct PlacedObject
{
    std::string id;
    PlacementCategory category;
    int x, y;
    int layer;
    float rotation = 0.0f;
    int height = 1;
    std::string material;
    std::vector<std::string> mods;
    int connectionMask = 0;
};

class PlacementSystem
{
public:
    PlacementSystem();

    void PlaceWall(int x, int y, const std::string &material, int height, int layer,
                   const std::vector<std::string> &mods = {});

    void PlaceProp(int x, int y, const std::string &id, int layer, float rotation = 0.0f);
    void PlaceTile(int x, int y, const std::string &tileType, int layer);

    std::string GetWallTexture(const PlacedObject &wall) const;
    std::string GetPropTexture(const PlacedObject &prop) const;

    void SnapToGrid(int &x, int &y);
    void SnapToWall(int x, int y, int &outX, int &outY);
    void UpdateWallConnections(int x, int y);

    PlacedObject *GetObjectAt(int x, int y, int layer = -1);
    const std::vector<PlacedObject> &GetAllObjects() const;
    void RemoveObject(int x, int y, int layer = -1);

    bool SaveToFile(const std::string &filename);
    bool LoadFromFile(const std::string &filename);

private:
    std::vector<PlacedObject> objects;

    static constexpr int NORTH = 1;
    static constexpr int SOUTH = 2;
    static constexpr int EAST = 4;
    static constexpr int WEST = 8;
};