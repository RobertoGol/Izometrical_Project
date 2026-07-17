#include "render/Renderer3D.hpp"
#include "content/MaterialCatalog.hpp"
#include "core/Constants.hpp"
#include "engine/Log.hpp"
#include <glad/glad.h>
#include <algorithm>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace bunker {

    namespace {
        std::string readTextFile(const char* path) {
            std::ifstream file(path);
            if (!file) {
                return {};
            }

            std::ostringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }

        bool checkShaderStatus(GLuint shader, const char* label) {
            GLint status = GL_FALSE;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
            if (status == GL_TRUE) {
                return true;
            }

            GLint logLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(static_cast<size_t>(std::max(logLength, 1)));
            glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());
            logError() << "[Renderer3D] " << label << " failed: " << log.data() << std::endl;
            return false;
        }

        bool checkProgramStatus(GLuint program, const char* label) {
            GLint status = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &status);
            if (status == GL_TRUE) {
                return true;
            }

            GLint logLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(static_cast<size_t>(std::max(logLength, 1)));
            glGetProgramInfoLog(program, static_cast<GLsizei>(log.size()), nullptr, log.data());
            logError() << "[Renderer3D] " << label << " failed: " << log.data() << std::endl;
            return false;
        }

        GLuint compileShader(GLenum type, const char* source, const char* label) {
            GLuint shader = glCreateShader(type);
            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);

            if (!checkShaderStatus(shader, label)) {
                glDeleteShader(shader);
                return 0;
            }

            return shader;
        }
    }

    void Renderer3D::initialize() {
        static bool gladLoaded = false;
        if (!gladLoaded) {
            gladLoaded = gladLoadGL() != 0;
        }
        if (!gladLoaded) {
            m_initialized = false;
            return;
        }

        loadShaders();

        // Глобальные настройки стейта OpenGL
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS); // Рисуем только то, что ближе к камере

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        m_initialized = true;
    }

    void Renderer3D::renderScene(Registry& registry, const Camera& camera) {
        if (!m_initialized) {
            return;
        }

        // 1. Очистка буферов
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. Небо
        renderSkyDome();

        // 3. --- ДЕБАГ: ВКЛЮЧАЕМ СЕТКУ (Wireframe) ---
        // Это заставит OpenGL рисовать контуры треугольников (Wireframe),
        // даже если шейдеры еще не настроены или текстуры не привязаны.
        glPolygonMode(GL_FRONT_AND_BACK, m_wireframeEnabled ? GL_LINE : GL_FILL);

        if (m_shaderProgram != 0) {
            const float aspectRatio = static_cast<float>(Config::SCREEN_WIDTH) / static_cast<float>(Config::SCREEN_HEIGHT);
            const glm::mat4 view = camera.getViewMatrix();
            const glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);

            glUseProgram(m_shaderProgram);
            if (m_locView >= 0) {
                glUniformMatrix4fv(m_locView, 1, GL_FALSE, glm::value_ptr(view));
            }
            if (m_locProjection >= 0) {
                glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(projection));
            }
        }

        // 4. Отрисовка ECS объектов
        auto& transforms = registry.transforms.getRawData();
        auto& meshes = registry.meshes.getRawData();
        const auto drawCount = std::min(meshes.size(), transforms.size());

        for (size_t i = 0; i < drawCount; ++i) {
            const auto& mesh = meshes[i];
            const auto& transform = transforms[i];

            if (m_shaderProgram != 0 && m_locModel >= 0) {
                glm::mat4 model{1.0f};
                model = glm::translate(model, transform.position);
                model *= glm::mat4_cast(transform.rotation);
                model = glm::scale(model, transform.scale);
                glUniformMatrix4fv(m_locModel, 1, GL_FALSE, glm::value_ptr(model));
            }

            bindMaterial(mesh.materialID);
            glBindVertexArray(mesh.vaoID);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
        }

        // Возвращаем режим заливки, чтобы UI не превратился в сетку
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void Renderer3D::setWireframeEnabled(bool enabled) {
        m_wireframeEnabled = enabled;
    }

    void Renderer3D::renderSkyDome() {
        // Отключаем запись в буфер глубины, чтобы небо не перекрывало объекты
        glDepthMask(GL_FALSE);

        // glUseProgram(m_SkyShader);
        // glBindTexture(GL_TEXTURE_2D, m_SkyTextureID);

        // m_SkyMesh.draw();

        // Возвращаем запись глубины
        glDepthMask(GL_TRUE);
    }

    void Renderer3D::loadShaders() {
        const std::string vertexSource = readTextFile("assets/shaders/base.vert");
        const std::string fragmentSource = readTextFile("assets/shaders/base.frag");
        if (vertexSource.empty() || fragmentSource.empty()) {
            logError() << "[Renderer3D] Shader files are missing: assets/shaders/base.vert/base.frag" << std::endl;
            return;
        }

        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource.c_str(), "Vertex shader");
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str(), "Fragment shader");
        if (vertexShader == 0 || fragmentShader == 0) {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        if (!checkProgramStatus(program, "Shader program")) {
            glDeleteProgram(program);
            return;
        }

        if (m_shaderProgram != 0) {
            glDeleteProgram(m_shaderProgram);
        }

        m_shaderProgram = program;
        m_locView = glGetUniformLocation(m_shaderProgram, "u_view");
        m_locProjection = glGetUniformLocation(m_shaderProgram, "u_projection");
        m_locModel = glGetUniformLocation(m_shaderProgram, "u_model");
        m_locMaterialColor = glGetUniformLocation(m_shaderProgram, "u_materialColor");
    }

    void Renderer3D::bindMaterial(std::uint32_t materialID) {
        if (m_shaderProgram == 0 || m_locMaterialColor < 0) {
            return;
        }

        const auto& material = getMaterial(materialID);
        const glm::vec3 color = hexToLinearRgb(material.hex);
        glUniform3fv(m_locMaterialColor, 1, glm::value_ptr(color));
    }

} // namespace bunker
