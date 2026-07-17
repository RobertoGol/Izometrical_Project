# Анализ кодовых стандартов

Проверено локально в ветке:

```text
codex/standards-cleanup
```

## Базовые правила

В корень проекта добавлены файлы для единых правил форматирования и статического анализа:

```text
.editorconfig
.clang-format
.clang-tidy
```

В CMake включен экспорт базы компиляции:

```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

Для актуальных targets добавлены предупреждения компилятора:

```text
MSVC: /W4 /permissive-
GCC/Clang: -Wall -Wextra -Wpedantic
```

## Текущий результат скана

Сканировались проектные файлы `*.cpp` и `*.hpp` в `src`, `include`, `tools/MapEditor`, без vendored-кода `source/SFML-3.1.0`, `source/ImGui` и build-папок.

```text
else if occurrences: 62
enum-like else-if switch candidates: 1
switch blocks without default: 0
global using namespace occurrences: 0
```

Единственный оставшийся enum-like hit:

```text
src/ai/BossAISystem.cpp:80
```

Это не кандидат на `switch`: логика завязана на диапазоны здоровья (`hpRatio <= 0.65f`, `hpRatio <= 0.30f`) и дополнительную проверку текущей фазы. Здесь `if / else if` корректнее, чем искусственный `switch`.

## Исправленные `default`

Проблема `switch` без `default` исправлена. Повторный brace-aware скан реальных `switch (...)` блоков показывает:

```text
missing_default_count 0
```

Исправление выполнено консервативно: добавлены `default: break;` там, где это сохраняет существующее fallback-поведение и не меняет игровую логику.

## Исправленные `if / else if` и вложенность

Выполнен отдельный проход по вложенным условиям, цепочкам `if / else if` и местам, где лучше подходят `switch` или lookup tables.

Исправлено:

```text
tools/MapEditor/src/main.cpp        string command dispatch -> lookup tables
tools/MapEditor/src/main.cpp        PlacementCategory branch -> switch
src/ai/EnemySpawner.cpp             guard clauses + type lookup table
src/ai/HostileAISystem.cpp          awareness branch flattened with guard returns
src/vehicles/VehicleManager.cpp     flattened hover-drive branch
src/gameplay/BulletSystem.cpp       flattened projectile hit loop
src/gameplay/DamageSystem.cpp       DamageType if/else -> switch
src/gameplay/StatefulActionsController.cpp  PilotActionState if/else -> switch
src/gameplay/AdvancedPilotMovement.cpp      PilotLocomotionMode if/else -> switch
src/world/WeatherSystem.cpp         WeatherType HUD if/else -> switch
src/gameplay/AdvancedMechanics.cpp  enum rotation chains -> switch
src/render/GameRenderer.cpp         CampObjectType and hostile alert branches -> switch
src/ui/HUD.cpp                      player mode branch -> switch
src/engine/GameApplication.cpp      player mode branches -> switch
src/engine/AudioController.cpp      voice dispatch -> lookup tables
tools/MapEditor/src/main.cpp        console parser -> MapEditorConsole module
tools/MapEditor/src/main.cpp        map geometry/rendering -> MapEditorView module
tools/MapEditor/src/main.cpp        placement state/actions/panel -> MapEditorPlacement module
tools/MapEditor/src/main.cpp        camera/map input -> MapEditorInput module
tools/MapEditor/src/main.cpp        console/instructions UI -> MapEditorPanels module
```

`src/engine/AudioController.cpp` также был приведен к SFML 3 API при прямой проверке dormant translation unit:

```text
setPosition(x, y) -> setPosition({x, y})
sf::Text default constructor -> sf::Text(font, string, size)
```

## Проверка

Выполнено:

```text
cl /std:c++17 /EHsc /Iinclude /Iinclude/core /Isource/SFML-3.1.0/include /c src/gameplay/StatefulActionsController.cpp
cl /std:c++17 /EHsc /Iinclude /Iinclude/core /Isource/SFML-3.1.0/include /c src/gameplay/AdvancedPilotMovement.cpp
cl /std:c++17 /EHsc /Iinclude /Iinclude/core /Isource/SFML-3.1.0/include /c src/ai/HostileAISystem.cpp
cmake --build build-codex-ninja
cmake --build build-codex-mapeditor
```

Финальный запуск через Visual Studio developer environment успешно собрал и слинковал:

```text
build-codex-ninja: BunkerProtocolISO.exe
build-codex-mapeditor: MapEditor.exe
```

Файлы оставлены в CRLF, согласно `.editorconfig`. Локальная настройка Git whitespace в этой рабочей копии воспринимает CRLF как `trailing whitespace`, поэтому `git diff --check` здесь не является полезным сигналом без настройки `cr-at-eol`.

## Оставшиеся стандарты

1. Крупные файлы все еще стоит дробить по подсистемам, особенно `src/gameplay/AdvancedMechanics.cpp`, `include/gameplay/AdvancedMechanics.hpp`, `src/render/GameRenderer.cpp` и `tools/MapEditor/src/main.cpp`.
2. В проекте остаются нормальные `else if`, которые проверяют диапазоны, compound conditions или последовательные guard-сценарии. Массово заменять их на `switch` нельзя без ухудшения читаемости.
3. Есть header-only реализации, которые лучше вынести в `.cpp`, например `include/content/TextureGenerator.hpp`.
4. Следующий полезный проход: уменьшение размера MapEditor `main.cpp` через выделение command parser, camera controls, placement logic и rendering helpers.

## Дополнительный проход: MapEditor main.cpp

Начато дробление `tools/MapEditor/src/main.cpp`.

Вынесено:

```text
tools/MapEditor/include/MapEditorConsole.hpp
tools/MapEditor/src/MapEditorConsole.cpp
tools/MapEditor/include/MapEditorView.hpp
tools/MapEditor/src/MapEditorView.cpp
tools/MapEditor/include/MapEditorPlacement.hpp
tools/MapEditor/src/MapEditorPlacement.cpp
tools/MapEditor/include/MapEditorInput.hpp
tools/MapEditor/src/MapEditorInput.cpp
tools/MapEditor/include/MapEditorPanels.hpp
tools/MapEditor/src/MapEditorPanels.cpp
```

После этого `tools/MapEditor/src/main.cpp` уменьшен примерно до 65 строк. Console command parsing, отрисовка map canvas, placement UI/actions, camera/map input и вспомогательные панели больше не живут в `main.cpp`.

Проверка:

```text
cmake --build build-codex-mapeditor
```

Результат: `MapEditor.exe` успешно собран и слинкован.

## Дополнительный проход: обязательные скобки и точечные switch-кандидаты

Выполнен проход по `src/gameplay/AdvancedMechanics.cpp` и связанным inline-заголовкам:

```text
src/gameplay/AdvancedMechanics.cpp
include/gameplay/AdvancedMechanics.hpp
include/core/Types.hpp
include/core/IsoMath.hpp
```

Что исправлено:

```text
readability-braces-around-statements: добавлены скобки к однострочным if/for/else
ToolGun prefab tile parser: char if/else chain -> switch
```

Отдельно проверено, что случайно затронутый vendored SFML-файл `source/SFML-3.1.0/include/SFML/Graphics/Transform.inl` был восстановлен и больше не имеет локального diff.

Оставшиеся `else if` в `src/gameplay/AdvancedMechanics.cpp` проверены вручную:

```text
ReactiveWorldSystem: условие здоровья/типа breakable, не enum-dispatch
ObjModelLoader: парсинг строковых OBJ-тегов, switch по std::string в C++17 невозможен
LanlineServices: строковый payload, switch неприменим без отдельного enum/lookup слоя
```

Проверка:

```text
brace-aware scan по измененным файлам: braceless control bodies не найдены
literal `r`n scan: артефактов не найдено
cmake --build build-codex-ninja: BunkerProtocolISO.exe успешно собран
cmake --build build-codex-mapeditor: no work to do, конфигурация остается собираемой
```

Ограничение проверки: корневой `CMakeLists.txt` сейчас собирает только `src/main.cpp` и vendored ImGui/SFML glue. Большая часть `src/gameplay`, включая `AdvancedMechanics.cpp`, остается вне root target, поэтому успешная сборка root target не доказывает компиляцию всех gameplay translation units.

## Дополнительный проход: покрытие CMake, логирование и switch-диспетчеризация

Выполнены пункты 1, 3 и 4 из последнего плана.

### 1. Покрытие CMake

Корневой `CMakeLists.txt` расширен: добавлена статическая библиотека `game_core`, в которую включена большая часть `src/**/*.cpp` за исключением `src/main.cpp` и кода инструментов. Исполняемый файл теперь линкуется с `game_core`, поэтому gameplay/render/ui/world/ai/persistence/engine-код реально проходит компиляцию, а не только лежит в дереве проекта.

После расширения покрытия были исправлены обнаруженные компилятором проблемы совместимости с SFML 3:

```text
InputManager / TerminalUI: новый API pollEvent(), getIf<>(), is<>()
GameRenderer / HUD / PipPadUI / MapScreen / WeatherSystem: vector-аргументы setPosition/setOrigin
GameRenderer / WeatherSystem: sf::PrimitiveType::Lines, sf::Shader::Type::Fragment, aggregate sf::Vertex
Text rendering: конструкторы sf::Text(font, string, size) вместо default constructor + setFont
ProceduralSoundEffects: std::int16_t/std::uint64_t, sf::SoundChannel::Mono, явный ignore для [[nodiscard]]
TextureGenerator: SFML 3 Image API и явный ignore для сохранения/копирования там, где результат пока только диагностический
```

Также исправлена связанная проблема с workstation-кодом: `include/content/Workstations.hpp` больше не содержит полную реализацию UI, реализация вынесена в `src/ui/WorkstationUI.cpp`.

### 3. Логирование вместо прямого std::cout/std::cerr

Добавлен `include/engine/Log.hpp` с проектными точками входа:

```text
bunker::logInfo()
bunker::logError()
```

Прямые `std::cout`, `std::cerr`, `std::clog` в основном коде `src/` и `include/` заменены на эти функции. Прямой вывод оставлен только в отдельном инструменте `tools/MapEditor/src/main.cpp`, где консольная ошибка инициализации является нормальным поведением tool executable.

Проверка:

```text
rg "std::(cout|cerr|clog)|#include <iostream>" src include tools
```

Остатки после прохода:

```text
include/engine/Log.hpp
tools/MapEditor/src/main.cpp
```

То есть в проектном runtime-коде прямого console logging больше нет.

### 4. Повторный scan else-if и switch-кандидатов

Дополнительно переведены реальные дискретные dispatch-сценарии:

```text
src/ui/TerminalUI.cpp: обработка клавиш Escape/E/Space/Enter -> switch
src/ui/PipPadUI.cpp: выбор вкладки m_ActiveTab -> switch
src/ui/PipPadUI.cpp: itemID-спецметки inventory -> switch с default
src/render/GameRenderer.cpp: TankUtilityMode -> switch
```

Оставшиеся `else if` проверены и не переводились механически, потому что это не switch-кандидаты:

```text
диапазоны и пороги: weather roll, HUD cooldown, erosion, boss HP phase, movement distance
геометрические/направленные условия: hitDir, local hit zones, camp adjacency
строковые парсеры конфигов: VehicleManager, EnemySpawner, TextureGenerator, OBJ tag parser
командные prefix-проверки: DevMenu command parser
guard/validation chains: BulletSystem, AdvancedMechanics
```

Для строковых парсеров возможен отдельный следующий шаг: заменить цепочки `key == "..."` на таблицы обработчиков или предварительную нормализацию ключей в enum. Это уже не простая замена на `switch`, потому что C++17 не поддерживает `switch` по `std::string`.

### Проверка после прохода

Команды сборки запускались через установленный Visual Studio CMake:

```text
cmake --build build-codex-ninja
cmake --build build-codex-mapeditor
```

Текущий результат:

```text
build-codex-ninja: успешно, отфильтрованных MSVC warning/error нет
build-codex-mapeditor: успешно, отфильтрованных MSVC warning/error нет
```

## Дополнительный проход: табличные парсеры конфигов

После локального checkpoint-коммита выполнен следующий шаг по оставшимся string parser chains.

Изменено:

```text
src/vehicles/VehicleManager.cpp: key/value parser -> таблица VehicleConfigSetter
src/ai/EnemySpawner.cpp: key/value parser -> таблица EnemyConfigSetter
include/content/TextureGenerator.hpp: cfg.method dispatch -> таблица VariationApplier
```

Смысл изменения: убрать длинные цепочки `if/else if (key == "...")` и `cfg.method == "..."` там, где это действительно дискретная dispatch-логика. Поведение неизвестных ключей и неизвестных методов сохранено: они просто игнорируются, как и раньше.

Проверка:

```text
cmake --build build-codex-ninja
cmake --build build-codex-mapeditor
```

Результат: обе сборки успешны, отфильтрованных MSVC warning/error нет.

## Дополнительный проход: дробление AdvancedMechanics.cpp

Выполнен следующий локальный шаг по крупным файлам: `src/gameplay/AdvancedMechanics.cpp` разделен по уже существующим подсистемным секциям.

Добавлены файлы:

```text
src/gameplay/AdvancedMechanicsSurvival.cpp
src/gameplay/AdvancedMechanicsWorld.cpp
src/gameplay/AdvancedMechanicsServices.cpp
```

После разделения `src/gameplay/AdvancedMechanics.cpp` содержит только фасад `AdvancedMechanics::initialize`, `AdvancedMechanics::update` и `AdvancedMechanics::onExplosion`. Код подсистем был перенесен без изменения логики:

```text
AdvancedMechanicsSurvival.cpp: RadioTapeSystem, SurvivalSystem, TankUtilitySystem
AdvancedMechanicsWorld.cpp: ReactiveWorldSystem, StoryRouteSystem, SkillSystem, LootGenerator, CampSystem, ToolGunSystem
AdvancedMechanicsServices.cpp: ObjModel, ObjModelLoader, LanlineServices, ProfileSessionSystem
```

`CMakeLists.txt` обновлен вручную, потому что список исходников для `game_core` явный, а не glob-based.

Проверка:

```text
cmake --build build-codex-ninja
cmake --build build-codex-mapeditor
```

Результат: обе сборки успешны, отфильтрованных MSVC warning/error нет.

## Дополнительный проход: дробление GameRenderer.cpp

Следующий крупный `.cpp` файл также разделен без изменения поведения: `src/render/GameRenderer.cpp`.

Добавлены файлы:

```text
src/render/GameRendererEntities.cpp
src/render/GameRendererAdvancedWorld.cpp
src/render/GameRendererAdvancedHUD.cpp
```

После разделения `src/render/GameRenderer.cpp` оставлен только для `GameRenderer::renderFloor`. Остальные методы перенесены по зонам ответственности:

```text
GameRendererEntities.cpp: GameRenderer::renderEntities
GameRendererAdvancedWorld.cpp: GameRenderer::renderAdvancedWorld
GameRendererAdvancedHUD.cpp: GameRenderer::renderAdvancedHUD
```

`CMakeLists.txt` обновлен, потому что `game_core` использует явный список исходников.

Проверка:

```text
cmake --build build-codex-ninja
cmake --build build-codex-mapeditor
```

Результат: обе сборки успешны, отфильтрованных MSVC warning/error нет.
## Дополнительный проход: дробление ProceduralSoundEffects.cpp

Разделен еще один крупный `.cpp` файл: `src/engine/ProceduralSoundEffects.cpp`.

Оставлено в исходном файле:

```text
SoundEffectsBank::s_Buffers
SoundEffectsBank::s_SoundPlayer
SoundEffectsBank::initializeFast
SoundEffectsBank::play
```

Добавлены файлы с генераторами PCM по категориям:

```text
src/engine/ProceduralSoundEffectsPilot.cpp
src/engine/ProceduralSoundEffectsMechanical.cpp
src/engine/ProceduralSoundEffectsWeapons.cpp
src/engine/ProceduralSoundEffectsEnvironment.cpp
```

Разделение выполнено как перенос реализаций `make...()` без изменения алгоритмов генерации звука.

Проверка:

```text
cmake --build build-codex-ninja
cmake --build build-codex-mapeditor
```

Результат: обе сборки успешны, отфильтрованных MSVC warning/error нет.
## Дополнительный проход: дробление TitanAI.cpp

Разделен `src/ai/TitanAI.cpp` без изменения поведения. Исходный файл оставлен для конструктора, посадки/высадки и верхнего `TitanAI::update`.

Добавлены файлы:

```text
src/ai/TitanAICockpit.cpp
src/ai/TitanAISystems.cpp
src/ai/TitanAIMovement.cpp
```

Перенос по зонам ответственности:

```text
TitanAICockpit.cpp: режимы огня, fireFromCockpit, autoFire, стабилизация ввода
TitanAISystems.cpp: loadout, boiler, pilot stress, vortex shield, core overdrive
TitanAIMovement.cpp: autonomous movement, piloted movement, combat anchor
```

`CMakeLists.txt` обновлен для явного списка исходников `game_core`.

Проверка:

```text
cmake --build build-codex-ninja
cmake --build build-codex-mapeditor
```

Результат: обе сборки успешны, отфильтрованных MSVC warning/error нет.
## Дополнительный проход: дробление Types.hpp

Самый маленький файл из оставшегося списка крупных кандидатов, `include/core/Types.hpp`, разделен на небольшие тематические заголовки. Старый файл оставлен как compatibility umbrella, поэтому существующие `#include "Types.hpp"` и `#include "core/Types.hpp"` продолжают работать.

Добавлены файлы:

```text
include/core/types/Enums.hpp
include/core/types/Vectors.hpp
include/core/types/GameplayTypes.hpp
include/core/types/PersistenceTypes.hpp
include/core/types/RenderStoryTypes.hpp
```

После разделения `include/core/Types.hpp` содержит только подключения новых заголовков. Самый большой новый заголовок: `GameplayTypes.hpp`, примерно 9.5 KB вместо прежних примерно 18 KB в одном файле.

Проверка:

```text
cmake --build build-codex-ninja
cmake --build build-codex-mapeditor
```

Результат: обе сборки успешны, отфильтрованных MSVC warning/error нет.
## Дополнительный проход: последние 4 файла из списка

Выполнен проход по последним четырем файлам из оставшегося списка крупных кандидатов:

```text
src/gameplay/BulletSystem.cpp
src/ai/HostileAISystem.cpp
src/entities/PlayerController.cpp
src/world/WeatherSystem.cpp
```

Что изменено:

```text
BulletSystem.cpp -> update-логика оставлена в основном файле
BulletSystemFire.cpp -> fireScoutWeapon, fireTitanWeapon, fireTitanMissiles
BulletSystemEffects.cpp -> splash damage и debug chain lightning
BulletSystemRender.cpp -> render и small helpers

HostileAISystem.cpp -> spawn/update/damage entry points
HostileAISystemProfiles.cpp -> profile/health/radius/runtime state helpers
HostileAISystemBehavior.cpp -> awareness, idle/search/aggro/ranged/melee behavior
HostileAISystemMovement.cpp -> target choice, movement helpers, random helper

WeatherSystem.cpp -> initialization/update/label entry points
WeatherSystemLogic.cpp -> weather selection, intensity, thunder, derived values
WeatherSystemEffects.cpp -> world effects and damage ticks
WeatherSystemRender.cpp -> public render methods
WeatherSystemDraw.cpp -> overlay draw helpers, colors, names, formatting/random helper

PlayerController.cpp -> длинный update разделен на приватные helper-методы
PlayerController.hpp -> добавлены приватные helper-declarations
```

`CMakeLists.txt` обновлен для новых `.cpp` файлов в явном списке `game_core`.

Проверка:

```text
cmake --build build-codex-ninja
cmake --build build-codex-mapeditor
```

Результат: обе сборки успешны, отфильтрованных MSVC warning/error нет.
## Дополнительный проход: два крупнейших файла

Выполнен проход по двум крупнейшим оставшимся файлам:

```text
include/content/TextureGenerator.hpp
src/gameplay/AdvancedMechanicsWorld.cpp
```

Что изменено:

```text
include/content/TextureGenerator.hpp -> compatibility umbrella
include/content/TextureGeneratorCore.hpp -> TextureGenConfig и TextureGenerator
include/content/TextureAtlasBaker.hpp -> AtlasSubRect и MegaTileAtlasBaker

src/gameplay/AdvancedMechanicsWorld.cpp -> ReactiveWorldSystem
src/gameplay/AdvancedMechanicsProgression.cpp -> StoryRouteSystem, SkillSystem, LootGenerator
src/gameplay/AdvancedMechanicsCamp.cpp -> CampSystem
src/gameplay/AdvancedMechanicsToolGun.cpp -> PrefabLibrary, ToolGunSystem
```

`CMakeLists.txt` обновлен для новых `.cpp` файлов AdvancedMechanics. Для заголовков TextureGenerator изменения CMake не требуются: старый путь `content/TextureGenerator.hpp` сохранен как umbrella include.

Проверка:

```text
cmake --build build-codex-ninja
cmake --build build-codex-mapeditor
```

Результат: обе сборки успешны, отфильтрованных MSVC warning/error нет.
## Дополнительный проход: дробление AdvancedMechanics.hpp

Следующий крупнейший файл, `include/gameplay/AdvancedMechanics.hpp`, разделен на тематические заголовки. Старый путь оставлен как compatibility umbrella, поэтому существующие include не менялись.

Добавлены файлы:

```text
include/gameplay/advanced/AdvancedMechanicsCommon.hpp
include/gameplay/advanced/AdvancedMechanicsSurvivalSystems.hpp
include/gameplay/advanced/AdvancedMechanicsWorldSystems.hpp
include/gameplay/advanced/AdvancedMechanicsProgressionSystems.hpp
include/gameplay/advanced/AdvancedMechanicsCampSystems.hpp
include/gameplay/advanced/AdvancedMechanicsToolGunSystems.hpp
include/gameplay/advanced/AdvancedMechanicsServices.hpp
include/gameplay/advanced/AdvancedMechanicsFacade.hpp
```

После разделения `include/gameplay/AdvancedMechanics.hpp` содержит только подключение facade-заголовка. Самый большой новый заголовок меньше 7 KB.

Проверка:

```text
cmake --build build-codex-ninja
cmake --build build-codex-mapeditor
```

Результат: обе сборки успешны, отфильтрованных MSVC warning/error нет.
## Дополнительный проход: дробление TextureGeneratorCore.hpp

Следующий крупнейший файл, `include/content/TextureGeneratorCore.hpp`, преобразован из header-only монолита в декларационный заголовок с companion inline-файлом.

Изменено:

```text
include/content/TextureGeneratorCore.hpp -> TextureGenConfig и декларация TextureGenerator
include/content/TextureGeneratorCore.inl -> inline-реализации методов TextureGenerator
```

Поведение осталось header-only: `TextureGeneratorCore.hpp` подключает `.inl` в конце, поэтому отдельный `.cpp` и изменения CMake не требуются. Размер основного `.hpp` уменьшен примерно с 28 KB до 1.8 KB.

Проверка:

```text
cmake --build build-codex-ninja
cmake --build build-codex-mapeditor
```

Результат: обе сборки успешны, отфильтрованных MSVC warning/error нет.