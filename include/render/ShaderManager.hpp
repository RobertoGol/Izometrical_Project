#pragma once

#include <cstdint>
#include <string>

namespace bunker
{
    class ShaderManager
    {
      public:
        static std::uint32_t loadProgram(const std::string& vertexPath, const std::string& fragmentPath);

      private:
        static std::string readTextFile(const std::string& path);
        static std::uint32_t compileShader(unsigned int type, const char* source, const char* label);
        static bool checkShaderStatus(std::uint32_t shader, const char* label);
        static bool checkProgramStatus(std::uint32_t program, const char* label);
    };

} // namespace bunker
