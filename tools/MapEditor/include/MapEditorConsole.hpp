#pragma once

#include <string>
#include <vector>

class MapEditorConsole
{
  public:
    std::vector<std::string> history;
    std::string input;

    void Execute(const std::string& cmd);
};
