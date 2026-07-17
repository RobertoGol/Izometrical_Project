#include "MapEditorConsole.hpp"

#include <map>
#include <sstream>

void MapEditorConsole::Execute(const std::string& cmd)
{
    history.push_back("> " + cmd);

    std::istringstream iss(cmd);
    std::string command;
    iss >> command;

    const std::map<std::string, std::string> directCommands = {
        {"help", "tile | obj | spawn | layer | map | trigger | nav | undo | redo | clear"},
        {"undo", "[Editor] Undo performed"},
        {"redo", "[Editor] Redo performed"},
        {"despawn", "[Spawn] Entities despawned"},
        {"spawn patrol", "[Spawn] Patrol route created"},
        {"stats", "[Stats] Objects: 124 | Tiles: 1024 | Entities: 37"},
        {"validate", "[Validate] Map validation passed"},
        {"random fill", "[Advanced] Random fill applied"},
        {"path create", "[Advanced] Path created between points"},
        {"area info", "[Advanced] Area information displayed"},
        {"export json", "[Export] Map exported as JSON"},
        {"import json", "[Import] JSON imported into map"},
    };

    const auto direct = directCommands.find(cmd);
    if (direct != directCommands.end())
    {
        history.push_back(direct->second);
        return;
    }

    if (command == "clear")
    {
        history.clear();
        return;
    }

    if (command == "spawn")
    {
        std::string type;
        std::string subtype;
        iss >> type >> subtype;
        history.push_back("[Spawn] " + type + " (" + subtype + ") spawned");
        return;
    }

    if (command == "layer")
    {
        std::string action;
        int id = 0;
        iss >> action >> id;
        const std::map<std::string, std::string> layerActions = {
            {"show", " shown"},
            {"hide", " hidden"},
            {"clear", " cleared"},
            {"lock", " locked"},
        };
        const auto layerAction = layerActions.find(action);
        history.push_back(layerAction == layerActions.end()
                              ? "[Layer] Unknown action"
                              : "[Layer] Layer " + std::to_string(id) + layerAction->second);
        return;
    }

    std::string action;
    iss >> action;

    const std::map<std::string, std::map<std::string, std::string>> actionCommands = {
        {"tile",
         {{"set", "[Tile] Single tile set"},
          {"fill", "[Tile] Area filled"},
          {"clear", "[Tile] Area cleared"},
          {"replace", "[Tile] Tiles replaced"},
          {"brush", "[Tile] Brush applied"},
          {"copy", "[Tile] Area copied to clipboard"},
          {"paste", "[Tile] Clipboard pasted"},
          {"random", "[Tile] Random tiles applied"},
          {"paint", "[Tile] Paint mode activated"},
          {"erase", "[Tile] Erase mode activated"},
          {"height", "[Tile] Heightmap modified"}}},
        {"obj",
         {{"place", "[Object] Object placed"},
          {"remove", "[Object] Object removed"},
          {"rotate", "[Object] Object rotated"},
          {"move", "[Object] Object moved"},
          {"scale", "[Object] Object scaled"},
          {"group", "[Object] Objects grouped"},
          {"ungroup", "[Object] Group removed"},
          {"select", "[Object] Selection mode"},
          {"copy", "[Object] Objects copied"},
          {"paste", "[Object] Objects pasted"}}},
        {"map",
         {{"new", "[Map] New map created"},
          {"resize", "[Map] Map resized"},
          {"save", "[Map] Map saved as .bwld"},
          {"load", "[Map] Map loaded"},
          {"export", "[Map] Exported to JSON"}}},
        {"trigger",
         {{"add", "[Trigger] Trigger created"},
          {"remove", "[Trigger] Trigger removed"},
          {"link", "[Trigger] Trigger linked to event"}}},
        {"nav",
         {{"bake", "[NavMesh] NavMesh baked"},
          {"clear", "[NavMesh] NavMesh cleared"},
          {"show", "[NavMesh] NavMesh visualization toggled"},
          {"obstacle", "[NavMesh] Obstacle added"}}},
        {"light",
         {{"add", "[Light] Light source added"},
          {"remove", "[Light] Light removed"},
          {"bake", "[Light] Lightmap baked"}}},
        {"fx", {{"add", "[Effect] Particle effect added"}, {"remove", "[Effect] Effect removed"}}},
        {"event",
         {{"create", "[Event] Custom event created"},
          {"link", "[Event] Event linked to trigger"},
          {"test", "[Event] Event test executed"}}},
    };

    const auto commandActions = actionCommands.find(command);
    if (commandActions == actionCommands.end())
    {
        history.push_back("[Console] Unknown command: " + command);
        return;
    }

    const auto actionMessage = commandActions->second.find(action);
    history.push_back(actionMessage == commandActions->second.end() ? "[" + command + "] Unknown action"
                                                                    : actionMessage->second);
}
