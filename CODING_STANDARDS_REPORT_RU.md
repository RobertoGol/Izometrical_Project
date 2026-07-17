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
