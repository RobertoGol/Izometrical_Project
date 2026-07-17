# Критический анализ проекта

Проверено локально в:

```text
C:\Users\RJ\Documents\rob\Izometrical_Project
```

Базовый коммит, от которого началась проверка:

```text
cde3041 update v.131
```

Рабочая ветка:

```text
codex/standards-cleanup
```

## Что исправлено

1. Корневой `CMakeLists.txt` теперь собирает актуальный `src/main.cpp` как отдельное приложение с vendored SFML 3.1.0 и vendored ImGui/ImGui-SFML.
2. `src/main.cpp` переведен на SFML 3 event API: `pollEvent()` возвращает optional event, а закрытие окна проверяется через `event->is<sf::Event::Closed>()`.
3. Исправлена несовместимость vendored `imgui-SFML.cpp` с новой texture API Dear ImGui: `ImTextureRef` проверяется через `GetTexID()`.
4. Добавлена явная линковка OpenGL через `find_package(OpenGL REQUIRED)` и `OpenGL::GL`.
5. `tools/MapEditor/CMakeLists.txt` больше не привязан к локальному пути `F:/vcpkg`; редактор собирается от vendored SFML/ImGui из репозитория.
6. Из MapEditor удалены вызовы ImGui docking API, потому что текущий vendored Dear ImGui не является docking-сборкой.
7. Исправлено предупреждение `IMGUI_API` macro redefinition в `source/ImGui/imgui-SFML_export.h`.
8. В `.gitignore` добавлены локальные CMake/build artefacts.

## Проверка сборки

Обе реальные точки входа теперь собираются локально через MSVC + Ninja:

```text
cmake -S . -B build-codex-ninja -G Ninja
cmake --build build-codex-ninja
```

Результат: `BunkerProtocolISO.exe` собирается.

```text
cmake -S tools/MapEditor -B build-codex-mapeditor -G Ninja
cmake --build build-codex-mapeditor
```

Результат: `MapEditor.exe` собирается.

## Важное уточнение по `main.cpp`

В проекте есть две реальные точки входа:

```text
src/main.cpp
tools/MapEditor/src/main.cpp
```

Сами по себе они не конфликтуют. Проблема была бы только если оба файла попали бы в один executable target. После исправлений они относятся к разным CMake-проектам/целям.

## Оставшиеся критические риски

1. Старый runtime-код в `src/` и `include/` не подключен к корневому executable. Сейчас корневая сборка сознательно собирает только актуальный ImGui/SFML launcher. Если цель проекта - вернуть полный игровой runtime, потребуется отдельная миграция CMake и SFML 2 -> SFML 3 API.
2. В репозитории все еще отслеживаются старые CMake artefacts: `CMakeCache.txt`, `CMakeFiles/`, `build/`, `compile_flags.txt`. Они добавлены в `.gitignore`, но уже отслеживаемые файлы нужно удалить из индекса отдельным коммитом.
3. Кодовая база содержит крупные модули и много условной логики. Это не ломает текущую сборку, но повышает риск регрессий при развитии runtime.

## Вывод

Главная практическая проблема была не в дубликате `main.cpp`, а в несогласованности CMake, SFML 3, ImGui-SFML и MapEditor. Эти проблемы исправлены и подтверждены сборкой.
