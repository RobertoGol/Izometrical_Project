# Bunker Protocol ISO: Industrial / Retro-Atmospheric Overhaul

Обязательно к выполнению.

Этот документ описывает технические и визуальные требования для достижения эстетики классических CRPG в духе Fallout 1/2 в современном 3D-движке.

## 1. Рендеринг и освещение

Чтобы картинка не выглядела как "пластиковый 3D", нужно имитировать освещение заброшенного индустриального бункера.

### PBR Workflow

- Внедрить шейдеры с поддержкой карт `Albedo`, `Normal`, `Roughness` и `Metallic`.
- Для Fallout-стиля материалы должны быть матовыми и грязными.
- Для бетона и ржавого металла целевой `Roughness`: `0.7-0.9`.

### Lighting Model

- **Global Ambient:** очень низкий холодный синий или серый оттенок.
- **Point Lights:** локальные источники света: лампы, огонь, аварийное освещение.
- **Flickering:** дрожание света через изменение интенсивности во времени, например `sin(time)`.
- **God Rays / Volumetric Light:** лучи света через вентиляционные отверстия и пыльный воздух бункера.

## 2. Пост-процессинг

Это ключевой слой, который превращает обычную 3D-графику в нужный ретро-индустриальный стиль.

### Color Grading / LUT

- Картинка должна быть десатурированной.
- Основные оттенки: оливковый, серый, сепия.
- Нужна LUT или эквивалентная цветокоррекция в пост-процессинге.

### Film Grain & Noise

- Добавить слабый пленочный шум поверх всей сцены.
- Шум должен скрывать цифровую стерильность рендера.

### Bloom

- Bloom должен быть грязным и мягким.
- Порог свечения должен быть низким, чтобы тусклые лампы оставляли ореолы.

### Chromatic Aberration

- Легкое смещение каналов `R/G/B` по краям экрана.
- Эффект должен быть слабым, иначе он начнет мешать чтению UI.

### CRT Distortion & Scanlines

- Для терминалов нужен отдельный CRT-шейдер.
- Эффекты: выпуклость экрана, scanlines, легкое мерцание, modulation brightness.

## 3. UI и терминалы

Интерфейс не должен выглядеть как стандартное desktop-окно. Он должен быть частью мира.

### Шрифты

- Использовать моноширинные шрифты.
- Подходящие варианты: `Courier New`, bitmap-fonts, кастомные raster-fonts в стиле Fallout.

### Стиль элементов

- Только монохромные схемы: amber, green или white.
- Анимация появления текста: печать по буквам.
- Ghosting: остаточное свечение текста при смене кадров.

### Pip-Pad Integration

- Pip-Pad должен рендериться в `FrameBuffer` / `RenderTexture`.
- Затем этот результат должен накладываться на 3D-модель планшета в руках персонажа.
- Цель: world-space UI, а не отдельное плоское окно.

## 4. Окружение

Атмосфера строится на деталях.

### Modular Asset System

- Генератор земли должен поддерживать грязевые пятна и decals.
- Для decals можно использовать `glPolygonOffset` или наложение плоскостей с текстурой грязи на пол.

### Clutter

- Нужна система авто-разброса мелких объектов вокруг рабочих мест.
- Примеры: бумага, кабели, гильзы, инструменты, мусор.
- Без clutter мир будет выглядеть пустым.

### Particle Effects

- **Dust Motes:** медленные частицы пыли в лучах света.
- **Steam / Smoke:** полупрозрачные спрайты дыма и пара из труб с медленным циклом жизни.

## 5. Техническая реализация

| Блок | Что нужно доделать |
| --- | --- |
| `Renderer3D` | Добавить FBO для bloom и post-processing chain. |
| `MeshBuilder` | Добавить tangent vectors для normal mapping. |
| `Shader System` | Создать shader manager, который обновляет `Time` и `Resolution` uniforms. |
| `Camera` | Добавить camera shake через смещение view matrix при взрывах. |
| `Material System` | Создать `Material`, хранящий пути к diffuse/normal/roughness/metallic textures. |

## 6. Приоритет реализации

- [x] **Shader Manager:** не хардкодить шейдеры, а загружать их из `assets/`.
- [ ] **Post-Processing Chain:** FBO / RenderTexture, чтобы сначала рисовать сцену в текстуру.
- [x] **Terminal Shader:** CRT-эффект для `TerminalUI`.
- [x] **Lighting:** базовое PBR-освещение.
- [x] **Atmospheric Fog:** туман в `base.frag` или отдельном post-process pass.

## 7. Найденные проблемы в текущем коде

### Critical

0. `cmake --build build-codex-ninja` сейчас не доходит до компиляции проекта.
   CMake падает на `source/SFML-3.1.0/lib/cmake/SFML/SFMLSystemSharedTargets.cmake`, потому что импортированная цель `SFML::System` ссылается на отсутствующий файл:

   ```text
   source/SFML-3.1.0/lib/sfml-system-d.lib
   ```

   Это блокирует нормальную проверку ветки в текущей конфигурации Debug/Ninja. Нужно либо восстановить debug-библиотеки SFML, либо переключить конфигурацию на имеющиеся release-библиотеки.

1. `src/render/Renderer3D.cpp` содержит строку `2. Небо` внутри C++ кода.
   Это не комментарий и не валидный C++ statement. Так как `src/render/Renderer3D.cpp` включен в `CMakeLists.txt`, сборка должна падать при компиляции этого translation unit.

2. `src/content/MeshBuilder.cpp`: `MeshBuilder::loadToGPU()` не создает `VAO/VBO/EBO`.
   Метод только заполняет `indexCount` и `materialID`, но не инициализирует `MeshComponent::vaoID`. Потом `Renderer3D::renderScene()` вызывает `glBindVertexArray(mesh.vaoID)`, что может использовать мусорное значение или `0`. В результате 3D-меши не будут надежно рендериться.

### High

3. `include/core/ECS.hpp`: `MeshComponent::vaoID`, `indexCount` и `materialID` не имеют default-инициализации.
   Если объект создается не через полностью корректный builder, в рендер попадут неинициализированные значения.

4. `include/core/ECS.hpp`: `ComponentArray::insert()` заполняет `components`, `entityToIndex` и `indexToEntity`, но не заполняет `denseEntities`.
   При этом `getDenseEntities()` возвращает `denseEntities`. Любой код, который попытается синхронизировать компоненты по dense entity list, получит пустой список.

5. `src/render/Renderer3D.cpp`: `Renderer3D::renderScene()` не применяет `TransformComponent`.
   Сейчас цикл проходит только по `registry.meshes.getRawData()` и рисует VAO без model matrix. Позиции, повороты и scale из ECS фактически игнорируются.

### Medium

6. `src/render/Renderer3D.cpp`: `renderScene()` принимает `const Camera& camera`, но не использует его.
   View/projection uniforms не обновляются, поэтому камера не управляет 3D-сценой.

7. `src/render/Renderer3D.cpp`: `m_shaderProgram`, `m_locView`, `m_locProjection`, `m_locModel` объявлены, но не инициализируются видимым образом.
   `loadShaders()` пока заглушка, поэтому shader pipeline из roadmap еще не реализован.

8. `src/render/Renderer3D.cpp`: wireframe включается всегда через `glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)`.
   Это полезно для отладки, но конфликтует с целью PBR/atmospheric rendering. Нужно вынести в debug flag.

## Update v.146o - not built, needs test

Status: code written, not built locally.

Changed files:

- `src/render/Renderer3D.cpp`
- `assets/shaders/base.vert`
- `assets/shaders/base.frag`

What was added:

- `Renderer3D::loadShaders()` now loads external shader files from `assets/shaders/base.vert` and `assets/shaders/base.frag`.
- The renderer compiles and links an OpenGL shader program.
- The renderer caches `u_model`, `u_view`, and `u_projection` uniform locations.
- Added a basic vertex shader using mesh position, normal, UV, model, view, and projection matrices.
- Added a basic fragment shader with simple directional lighting, dirty/grimy color variation, and light distance fog.

What needs checking:

- The game must be started from a working directory where `assets/shaders/base.vert` and `assets/shaders/base.frag` are reachable.
- If the screen is black, check console output for `[Renderer3D] Shader files are missing` or shader compile errors.
- Check that the cube/ground still render after shader loading.
- Check that camera movement changes the 3D view.
- Check that objects use their ECS transform positions and scale.
- Full build/test is still blocked locally until the SFML package/build environment is fixed.

## Deep Dive System Task List

Этот список сделан после повторного обзора всей системы: application loop, 3D renderer, ECS, legacy 2D renderer, gameplay, input, save/load, content/assets, materials, build/dependencies. Отмечай пункты по мере закрытия.

### P0 - runtime blockers

- [x] Убрать тестовый лимит `frameCount > 1000` из `GameApplication::run()`, чтобы игра не завершалась сама после demo-run.
- [x] Восстановить `updateMouseWorldPosition()`: экранная позиция мыши должна снова превращаться в world position для aim, toolgun, CAMP placement и interaction.
- [x] Восстановить `m_Camera.update(...)` или явно связать новую 3D camera с `GameState`, mouse aim и player/titan movement.
- [x] Вернуть рабочий UI/HUD слой в `renderGameplayFrame()`: сейчас HUD, PipPad, subtitles, terminal и ImGui временно отключены.
- [x] Убрать debug-magenta clear color из normal gameplay path и оставить его только как explicit debug mode.
- [x] Исправить `ResourceManager::ConsumeResources()`: параметр `circuits` скрывает поле класса, из-за чего списание circuits может работать неправильно.

### P1 - 3D renderer and ECS integration

- [x] `MeshBuilder::loadToGPU()` создает VAO/VBO/EBO и заполняет `MeshComponent::vaoID/indexCount`.
- [x] `Renderer3D::renderScene()` применяет `TransformComponent` через model matrix.
- [x] `Renderer3D::renderScene()` использует camera view/projection uniforms.
- [x] Shader files `assets/shaders/base.vert` и `assets/shaders/base.frag` загружаются из assets.
- [x] Добавить ownership/lifetime для GPU buffers: сейчас VBO/EBO создаются, но не хранятся и не освобождаются.
- [x] Перестать связывать `transforms` и `meshes` только по индексу массива; использовать entity IDs из ECS, чтобы transform и mesh не расходились.
- [x] Добавить normal/tangent support для будущего normal mapping.
- [x] Перенести sky clear color, lighting и fog в управляемые параметры `Renderer3D`/shader uniforms вместо пустого `renderSkyDome()`.
- [x] Добавить debug toggle для wireframe.
- [x] Связать sky/weather параметры с `WeatherSystem`.
- [x] Добавить debug overlays для normals, material IDs и camera frustum.

### P1 - gameplay to 3D migration

- [x] Решить источник истины для текущего прохода: старый `GameState` остается gameplay source of truth, ECS получает render anchors.
- [x] Связать player/titan с 3D entities через adapter layer между `GameState` и ECS.
- [x] Вернуть рендер существующих gameplay-сущностей после 3D pass: floor, enemies, loot, tower, player/titan и advanced world markers.
- [x] Обновить `SpatialGrid` регистрацию для player, titan и alive enemies.
- [x] Связать enemies/vehicles/workstations с pooled 3D ECS entities.
- [x] Проверить collision/interaction coordinates после перехода с isometric 2D на 3D camera.
- [x] Подключить CAMP validator и world containers к единому coordinate adapter.

### P1 - save/load and persistence

- [x] Расширить save/load на ECS render entities, transforms, mesh material IDs и generated terrain metadata tail.
- [x] Добавить versioned migration для save files перед следующими изменениями формата.
- [x] Проверять `read()` после каждого бинарного блока save file, чтобы corrupted/short saves не давали partially loaded state.
- [x] Сохранять runtime content decisions: placed CAMP objects and destructible state.

### P2 - content, assets, and materials

- [x] Добавлен быстрый catalog `materialID -> material definition`.
- [x] Цвет материала хранится как hex (`grass -> 0x...`), а не как ручные RGB float values.
- [x] `Renderer3D::bindMaterial()` отправляет material color в `base.frag` через `u_materialColor`.
- [x] Ground/cube/terrain получают первые material IDs.
- [x] Убрать путаницу двух `TextureGenerator`: старый `include/TextureGenerator.hpp`/`src/TextureGenerator.cpp` и новый `include/content/TextureGeneratorCore.hpp`.
- [x] Добавить material categories: terrain, metal, concrete, organic, debug.
- [x] Добавить material properties: roughness, metallic, emissive/fallback values.
- [x] Добавить texture slots: albedo, normal, roughness, metallic; связать их с generated textures.
- [x] Добавить validation material catalog: duplicate IDs, unknown ID fallback logging.
- [x] Добавить editor/debug UI для выбора и просмотра material ID/name/hex.
- [ ] Когда появятся texture bindings, вынести material binding из `Renderer3D` в dedicated material system.

### P2 - architecture and code health

- [x] Уменьшить `GameApplication.cpp`: сейчас это главный orchestration bottleneck и самый крупный файл runtime.
- [x] Разделить старый 2D renderer и новый 3D renderer по clear ownership: кто рисует world, кто UI, кто debug.
- [x] Убрать закомментированные рабочие блоки из runtime path: заменить на feature flags/debug config.
- [x] Продолжить дробление крупных модулей: `AdvancedMechanicsSurvival`, `AdvancedMechanicsServices`, `VehicleManager`, `DevMenu`.
- [x] Вынести `RadioTapeSystem` из `AdvancedMechanicsSurvival.cpp` в отдельный модуль `AdvancedMechanicsRadio.cpp`.
- [x] Вынести `TankUtilitySystem` из `AdvancedMechanicsSurvival.cpp` в отдельный модуль `AdvancedMechanicsTankUtility.cpp`.
- [x] Вынести OBJ model loader из `AdvancedMechanicsServices.cpp` в отдельный модуль `AdvancedMechanicsObjModel.cpp`.
- [x] Вынести `LanlineServices` из `AdvancedMechanicsServices.cpp` в отдельный модуль `AdvancedMechanicsLanline.cpp`.
- [x] Вынести `VehicleInstance` physics/drive methods из `VehicleManager.cpp` в отдельный модуль `VehicleInstance.cpp`.
- [x] Вынести Lua/console часть `DevMenu` в отдельный модуль `DevMenuConsole.cpp` и объявить скрытые DEV_BUILD методы/state в header.
- [x] Очистить mojibake/битую кодировку русских комментариев, чтобы документация и комментарии были читаемыми в IDE.

### P2 - build, dependencies, and verification

- [x] GLM подключен как `SYSTEM` include, чтобы не шуметь внешними MSVC warning.
- [ ] Заменить локально сгенерированные SFML import libs на нормальный воспроизводимый dependency setup.
- [x] Добавить documented build command для Visual Studio Build Tools/CMake на Windows.
- [x] Добавить smoke test или run-mode без ручного окна: build + shader file check + asset path check.
- [x] Проверить MapEditor после 3D/runtime изменений, чтобы editor branch не отстал от game assets.

### What to test after next changes

- [ ] Игра не закрывается сама через ~1000 кадров.
- [ ] Камера двигается/следит за игроком в 3D.
- [ ] Mouse aim/toolgun/CAMP placement используют правильную world position.
- [ ] HUD/PipPad/Terminal снова видны поверх 3D.
- [ ] Ground, terrain и cube имеют разные material colors.
- [ ] Save/load сохраняет не только старый 2D state, но и новые 3D/ECS данные.
