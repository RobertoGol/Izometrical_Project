#pragma once

#include <iostream>
#include <ostream>

namespace bunker
{
    inline std::ostream& logInfo()
    {
        return std::cout;
    }

    inline std::ostream& logError()
    {
        return std::cerr;
    }
} // namespace bunker
