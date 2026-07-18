#include "render/ShaderManager.hpp"
#include "engine/Log.hpp"

#include <glad/glad.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

namespace bunker
{
    std::uint32_t ShaderManager::loadProgram(const std::string& vertexPath, const std::string& fragmentPath)
    {
        const std::string vertexSource = readTextFile(vertexPath);
        const std::string fragmentSource = readTextFile(fragmentPath);
        if (vertexSource.empty() || fragmentSource.empty())
        {
            logError() << "[ShaderManager] Shader files are missing: " << vertexPath << " / " << fragmentPath
                       << std::endl;
            return 0;
        }

        const std::uint32_t vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource.c_str(), "Vertex shader");
        const std::uint32_t fragmentShader =
            compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str(), "Fragment shader");
        if (vertexShader == 0 || fragmentShader == 0)
        {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return 0;
        }

        const std::uint32_t program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        if (!checkProgramStatus(program, "Shader program"))
        {
            glDeleteProgram(program);
            return 0;
        }

        return program;
    }

    std::string ShaderManager::readTextFile(const std::string& path)
    {
        std::ifstream file(path);
        if (!file)
        {
            return {};
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::uint32_t ShaderManager::compileShader(unsigned int type, const char* source, const char* label)
    {
        const std::uint32_t shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        if (!checkShaderStatus(shader, label))
        {
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    bool ShaderManager::checkShaderStatus(std::uint32_t shader, const char* label)
    {
        GLint status = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status == GL_TRUE)
        {
            return true;
        }

        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<std::size_t>(std::max(logLength, 1)));
        glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());
        logError() << "[ShaderManager] " << label << " failed: " << log.data() << std::endl;
        return false;
    }

    bool ShaderManager::checkProgramStatus(std::uint32_t program, const char* label)
    {
        GLint status = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status == GL_TRUE)
        {
            return true;
        }

        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(static_cast<std::size_t>(std::max(logLength, 1)));
        glGetProgramInfoLog(program, static_cast<GLsizei>(log.size()), nullptr, log.data());
        logError() << "[ShaderManager] " << label << " failed: " << log.data() << std::endl;
        return false;
    }

} // namespace bunker
