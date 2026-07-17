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

- [ ] **Shader Manager:** не хардкодить шейдеры, а загружать их из `assets/`.
- [ ] **Post-Processing Chain:** FBO / RenderTexture, чтобы сначала рисовать сцену в текстуру.
- [ ] **Terminal Shader:** CRT-эффект для `TerminalUI`.
- [ ] **Lighting:** базовое PBR-освещение.
- [ ] **Atmospheric Fog:** туман в `base.frag` или отдельном post-process pass.

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
