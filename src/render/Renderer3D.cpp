#include "render/Renderer3D.hpp"
#include "content/MaterialCatalog.hpp"
#include "core/Constants.hpp"
#include "engine/Log.hpp"
#include "render/ShaderManager.hpp"
#include <glad/glad.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/common.hpp>
#include <string>

namespace bunker {

    void Renderer3D::shutdown() {
        releasePostProcessTarget();
        if (m_fullscreenQuadVbo != 0) {
            glDeleteBuffers(1, &m_fullscreenQuadVbo);
            m_fullscreenQuadVbo = 0;
        }
        if (m_fullscreenQuadVao != 0) {
            glDeleteVertexArrays(1, &m_fullscreenQuadVao);
            m_fullscreenQuadVao = 0;
        }
        if (m_postShaderProgram != 0) {
            glDeleteProgram(m_postShaderProgram);
            m_postShaderProgram = 0;
        }
        if (m_shaderProgram != 0) {
            glDeleteProgram(m_shaderProgram);
            m_shaderProgram = 0;
        }
        m_initialized = false;
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
        loadPostProcessShader();
        createFullscreenQuad();
        std::string materialError;
        if (!validateMaterialCatalog(&materialError)) {
            logError() << "[Renderer3D] Material catalog invalid: " << materialError << std::endl;
        }

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

        const unsigned int targetWidth = static_cast<unsigned int>(Config::SCREEN_WIDTH);
        const unsigned int targetHeight = static_cast<unsigned int>(Config::SCREEN_HEIGHT);
        const bool usePostProcess =
            m_postShaderProgram != 0 && ensurePostProcessTarget(targetWidth, targetHeight);
        if (usePostProcess) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFramebuffer);
            glViewport(0, 0, static_cast<GLsizei>(targetWidth), static_cast<GLsizei>(targetHeight));
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // 1. Очистка буферов
        glClearColor(m_skyColor.r, m_skyColor.g, m_skyColor.b, 1.0f);
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
            if (m_locLightDirection >= 0) {
                const glm::vec3 lightDirection = glm::normalize(m_lightDirection);
                glUniform3fv(m_locLightDirection, 1, glm::value_ptr(lightDirection));
            }
            if (m_locAmbientColor >= 0) {
                glUniform3fv(m_locAmbientColor, 1, glm::value_ptr(m_ambientColor));
            }
            if (m_locLightColor >= 0) {
                glUniform3fv(m_locLightColor, 1, glm::value_ptr(m_lightColor));
            }
            if (m_locFogColor >= 0) {
                glUniform3fv(m_locFogColor, 1, glm::value_ptr(m_fogColor));
            }
            if (m_locDebugOverlay >= 0) {
                glUniform1i(m_locDebugOverlay, static_cast<int>(m_debugOverlay));
            }
        }

        // 4. Отрисовка ECS объектов
        const auto& meshes = registry.meshes.getRawData();
        const auto& meshEntities = registry.meshes.getDenseEntities();
        const auto drawCount = std::min(meshes.size(), meshEntities.size());

        for (size_t i = 0; i < drawCount; ++i) {
            const auto& mesh = meshes[i];
            const TransformComponent* transform = registry.transforms.get(meshEntities[i]);
            if (transform == nullptr || mesh.vaoID == 0 || mesh.indexCount == 0) {
                continue;
            }

            if (m_shaderProgram != 0 && m_locModel >= 0) {
                glm::mat4 model{1.0f};
                model = glm::translate(model, transform->position);
                model *= glm::mat4_cast(transform->rotation);
                model = glm::scale(model, transform->scale);
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

        if (usePostProcess) {
            renderPostProcess(targetWidth, targetHeight);
        }
    }

    void Renderer3D::setWireframeEnabled(bool enabled) {
        m_wireframeEnabled = enabled;
    }

    void Renderer3D::setDebugOverlay(RendererDebugOverlay overlay) {
        m_debugOverlay = overlay;
    }

    void Renderer3D::applyWeather(const WeatherRuntimeState& weather) {
        const float intensity = std::clamp(weather.intensity, 0.0f, 1.0f);

        glm::vec3 sky{0.10f, 0.11f, 0.14f};
        glm::vec3 fog{0.11f, 0.12f, 0.105f};
        glm::vec3 ambient{0.055f, 0.065f, 0.075f};
        glm::vec3 light{0.76f, 0.68f, 0.52f};
        glm::vec3 lightDirection{-0.35f, 0.85f, 0.28f};

        switch (weather.current)
        {
        case WeatherType::Clear:
            break;
        case WeatherType::EtherFog:
            sky = glm::mix(sky, glm::vec3{0.16f, 0.11f, 0.24f}, intensity);
            fog = glm::mix(fog, glm::vec3{0.34f, 0.20f, 0.52f}, intensity);
            ambient = glm::mix(ambient, glm::vec3{0.075f, 0.055f, 0.11f}, intensity);
            light = glm::mix(light, glm::vec3{0.54f, 0.48f, 0.68f}, intensity);
            break;
        case WeatherType::AcidRain:
            sky = glm::mix(sky, glm::vec3{0.08f, 0.12f, 0.08f}, intensity);
            fog = glm::mix(fog, glm::vec3{0.24f, 0.36f, 0.16f}, intensity);
            ambient = glm::mix(ambient, glm::vec3{0.045f, 0.075f, 0.045f}, intensity);
            light = glm::mix(light, glm::vec3{0.55f, 0.72f, 0.38f}, intensity);
            break;
        case WeatherType::AshStorm:
            sky = glm::mix(sky, glm::vec3{0.15f, 0.12f, 0.09f}, intensity);
            fog = glm::mix(fog, glm::vec3{0.38f, 0.30f, 0.20f}, intensity);
            ambient = glm::mix(ambient, glm::vec3{0.08f, 0.065f, 0.045f}, intensity);
            light = glm::mix(light, glm::vec3{0.68f, 0.48f, 0.30f}, intensity);
            lightDirection = glm::normalize(glm::mix(lightDirection, glm::vec3{-0.55f, 0.65f, 0.18f}, intensity));
            break;
        case WeatherType::EtherStorm:
            sky = glm::mix(sky, glm::vec3{0.07f, 0.055f, 0.12f}, intensity);
            fog = glm::mix(fog, glm::vec3{0.25f, 0.14f, 0.42f}, intensity);
            ambient = glm::mix(ambient, glm::vec3{0.045f, 0.035f, 0.085f}, intensity);
            light = glm::mix(light, glm::vec3{0.44f, 0.36f, 0.76f}, intensity);
            break;
        case WeatherType::AutumnBreeze:
            sky = glm::mix(sky, glm::vec3{0.18f, 0.13f, 0.08f}, intensity);
            fog = glm::mix(fog, glm::vec3{0.30f, 0.21f, 0.12f}, intensity);
            light = glm::mix(light, glm::vec3{0.86f, 0.56f, 0.32f}, intensity);
            break;
        case WeatherType::WinterBlizzard:
            sky = glm::mix(sky, glm::vec3{0.12f, 0.16f, 0.20f}, intensity);
            fog = glm::mix(fog, glm::vec3{0.58f, 0.68f, 0.74f}, intensity);
            ambient = glm::mix(ambient, glm::vec3{0.075f, 0.095f, 0.115f}, intensity);
            light = glm::mix(light, glm::vec3{0.72f, 0.82f, 0.92f}, intensity);
            break;
        default:
            break;
        }

        m_skyColor = glm::clamp(sky, glm::vec3{0.0f}, glm::vec3{1.0f});
        m_fogColor = glm::clamp(fog, glm::vec3{0.0f}, glm::vec3{1.0f});
        m_ambientColor = glm::clamp(ambient, glm::vec3{0.0f}, glm::vec3{1.0f});
        m_lightColor = glm::clamp(light, glm::vec3{0.0f}, glm::vec3{1.0f});
        m_lightDirection = lightDirection;
    }

    void Renderer3D::renderSkyDome() {
        // Sky is currently represented by the renderer clear color. Keep depth writes enabled
        // so the following world pass owns the depth buffer completely.
        glDepthMask(GL_FALSE);
        glDepthMask(GL_TRUE);
    }

    void Renderer3D::loadShaders() {
        const std::uint32_t program = ShaderManager::loadProgram("assets/shaders/base.vert", "assets/shaders/base.frag");
        if (program == 0) {
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
        m_locMaterialEmissive = glGetUniformLocation(m_shaderProgram, "u_materialEmissive");
        m_locMaterialRoughness = glGetUniformLocation(m_shaderProgram, "u_materialRoughness");
        m_locMaterialMetallic = glGetUniformLocation(m_shaderProgram, "u_materialMetallic");
        m_locLightDirection = glGetUniformLocation(m_shaderProgram, "u_lightDirection");
        m_locAmbientColor = glGetUniformLocation(m_shaderProgram, "u_ambientColor");
        m_locLightColor = glGetUniformLocation(m_shaderProgram, "u_lightColor");
        m_locFogColor = glGetUniformLocation(m_shaderProgram, "u_fogColor");
        m_locDebugOverlay = glGetUniformLocation(m_shaderProgram, "u_debugOverlay");
        m_locMaterialDebugColor = glGetUniformLocation(m_shaderProgram, "u_materialDebugColor");
    }

    void Renderer3D::loadPostProcessShader() {
        const std::uint32_t program =
            ShaderManager::loadProgram("assets/shaders/post.vert", "assets/shaders/post.frag");
        if (program == 0) {
            return;
        }

        if (m_postShaderProgram != 0) {
            glDeleteProgram(m_postShaderProgram);
        }

        m_postShaderProgram = program;
        m_locPostSceneTexture = glGetUniformLocation(m_postShaderProgram, "u_sceneTexture");
        m_locPostResolution = glGetUniformLocation(m_postShaderProgram, "u_resolution");
    }

    void Renderer3D::createFullscreenQuad() {
        if (m_fullscreenQuadVao != 0) {
            return;
        }

        constexpr float quadVertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f, 1.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
        };

        glGenVertexArrays(1, &m_fullscreenQuadVao);
        glGenBuffers(1, &m_fullscreenQuadVbo);
        glBindVertexArray(m_fullscreenQuadVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_fullscreenQuadVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              reinterpret_cast<void*>(2 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    bool Renderer3D::ensurePostProcessTarget(unsigned int width, unsigned int height) {
        if (width == 0 || height == 0) {
            return false;
        }

        if (m_sceneFramebuffer != 0 && m_sceneTargetWidth == width && m_sceneTargetHeight == height) {
            return true;
        }

        releasePostProcessTarget();
        m_sceneTargetWidth = width;
        m_sceneTargetHeight = height;

        glGenFramebuffers(1, &m_sceneFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_sceneFramebuffer);

        glGenTextures(1, &m_sceneColorTexture);
        glBindTexture(GL_TEXTURE_2D, m_sceneColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_sceneColorTexture, 0);

        glGenRenderbuffers(1, &m_sceneDepthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_sceneDepthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(width),
                              static_cast<GLsizei>(height));
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  m_sceneDepthRenderbuffer);

        const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (!complete) {
            logError() << "[Renderer3D] Post-process framebuffer is incomplete." << std::endl;
            releasePostProcessTarget();
            return false;
        }

        return true;
    }

    void Renderer3D::renderPostProcess(unsigned int width, unsigned int height) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glUseProgram(m_postShaderProgram);
        if (m_locPostSceneTexture >= 0) {
            glUniform1i(m_locPostSceneTexture, 0);
        }
        if (m_locPostResolution >= 0) {
            glUniform2f(m_locPostResolution, static_cast<float>(width), static_cast<float>(height));
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_sceneColorTexture);
        glBindVertexArray(m_fullscreenQuadVao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }

    void Renderer3D::releasePostProcessTarget() {
        if (m_sceneDepthRenderbuffer != 0) {
            glDeleteRenderbuffers(1, &m_sceneDepthRenderbuffer);
            m_sceneDepthRenderbuffer = 0;
        }
        if (m_sceneColorTexture != 0) {
            glDeleteTextures(1, &m_sceneColorTexture);
            m_sceneColorTexture = 0;
        }
        if (m_sceneFramebuffer != 0) {
            glDeleteFramebuffers(1, &m_sceneFramebuffer);
            m_sceneFramebuffer = 0;
        }
        m_sceneTargetWidth = 0;
        m_sceneTargetHeight = 0;
    }

    void Renderer3D::bindMaterial(std::uint32_t materialID) {
        if (m_shaderProgram == 0 || m_locMaterialColor < 0) {
            return;
        }

        const auto& material = getMaterial(materialID);
        const glm::vec3 color = hexToLinearRgb(material.hex);
        glUniform3fv(m_locMaterialColor, 1, glm::value_ptr(color));
        if (m_locMaterialDebugColor >= 0) {
            const glm::vec3 debugColor = materialDebugColor(materialID);
            glUniform3fv(m_locMaterialDebugColor, 1, glm::value_ptr(debugColor));
        }
        if (m_locMaterialEmissive >= 0) {
            const glm::vec3 emissive = hexToLinearRgb(material.emissiveHex);
            glUniform3fv(m_locMaterialEmissive, 1, glm::value_ptr(emissive));
        }
        if (m_locMaterialRoughness >= 0) {
            glUniform1f(m_locMaterialRoughness, material.roughness);
        }
        if (m_locMaterialMetallic >= 0) {
            glUniform1f(m_locMaterialMetallic, material.metallic);
        }
    }

    glm::vec3 Renderer3D::materialDebugColor(std::uint32_t materialID) {
        const std::uint32_t hash = materialID * 2654435761u;
        const float r = static_cast<float>((hash >> 16) & 0xFFu) / 255.0f;
        const float g = static_cast<float>((hash >> 8) & 0xFFu) / 255.0f;
        const float b = static_cast<float>(hash & 0xFFu) / 255.0f;
        return glm::clamp(glm::vec3{r, g, b} * 0.75f + glm::vec3{0.20f}, glm::vec3{0.0f}, glm::vec3{1.0f});
    }

} // namespace bunker
