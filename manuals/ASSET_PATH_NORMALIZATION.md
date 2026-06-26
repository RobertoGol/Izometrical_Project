# ASSET PATH NORMALIZATION

Цель: унифицировать структуру ресурсов в `Izometrical_Project` после слияния `GMyGameDoNotTouch`, `Video_Game_Izom` и текущего целевого репозитория, чтобы избежать битых путей, проблем с регистром и скрытых platform-specific багов.

---

## Главный принцип

Пути к ассетам должны быть:

- стабильными;
- предсказуемыми;
- чувствительными к кроссплатформенности;
- не зависящими от случайного регистра букв;
- не захардкоженными хаотично по gameplay-коду.

---

# 1. Ключевая уже известная проблема

Сейчас подтверждён конфликт регистра директорий:

- `assets/Enemies/`
- `assets/Vehicles/`

в то время как ожидается:

- `assets/enemies/`
- `assets/vehicles/`

## Почему это критично
На системах с case-sensitive файловой системой:
- ресурсы не найдутся;
- загрузка может молча проваливаться;
- часть контента будет “работать у одного разработчика и ломаться у другого”.

---

# 2. Базовое правило именования

## Все директории и файлы ресурсов должны быть:
- в нижнем регистре;
- без пробелов;
- с `_` или `-` только при необходимости;
- без случайных разных вариантов одного и того же имени.

## Примеры правильных путей
- `assets/enemies/ghoul.png`
- `assets/vehicles/tank_body.png`
- `assets/ui/hud/frame.png`
- `assets/audio/radio/intercept_01.ogg`
- `assets/models/props/crate_a.obj`

## Примеры неправильных путей
- `assets/Enemies/Ghoul.png`
- `assets/Vehicles/TankBody.PNG`
- `Assets/UI/HUD Frame.png`
- `assets\\audio\\Radio\\Tape01.OGG`

---

# 3. Единый корень ресурсов

Рекомендуемый единый корень:

- `assets/`

Нельзя смешивать несколько корней без явной причины:
- `Assets/`
- `data/assets/`
- `res/`
- `resources/`

если это не поддерживается централизованным loader-слоем и не оформлено явно.

---

# 4. Рекомендуемая структура директорий

Пример целевой структуры:

- `assets/enemies/`
- `assets/vehicles/`
- `assets/player/`
- `assets/weapons/`
- `assets/projectiles/`
- `assets/world/`
- `assets/props/`
- `assets/buildings/`
- `assets/ui/`
- `assets/ui/hud/`
- `assets/ui/terminal/`
- `assets/ui/pip_pad/`
- `assets/audio/`
- `assets/audio/sfx/`
- `assets/audio/music/`
- `assets/audio/radio/`
- `assets/audio/tapes/`
- `assets/weather/`
- `assets/effects/`
- `assets/models/`
- `assets/models/vehicles/`
- `assets/models/props/`
- `assets/data/`
- `assets/data/loot/`
- `assets/data/prefabs/`
- `assets/data/skills/`
- `assets/data/story/`
- `assets/data/weather/`

---

# 5. Немедленные обязательные исправления

## 5.1. Переименовать директории
- `assets/Enemies/` → `assets/enemies/`
- `assets/Vehicles/` → `assets/vehicles/`

## 5.2. Найти все ссылки в коде и данных
Заменить все обращения вида:
- `"assets/Enemies/..."`
- `"assets/Vehicles/..."`
- `"Enemies/..."`
- `"Vehicles/..."`

на нормализованные варианты.

## 5.3. Проверить расширения файлов
Желательно унифицировать регистр расширений:
- `.png`
- `.jpg`
- `.ogg`
- `.wav`
- `.obj`
- `.json`
- `.txt`

а не:
- `.PNG`
- `.OBJ`
- `.WAV`

---

# 6. Правила для путей в коде

## Обязательные правила
1. Использовать `/`, а не `\`.
2. Не писать пути с разным регистром в разных местах.
3. Не собирать пути вручную во многих gameplay-файлах.
4. По возможности использовать централизованный resolver/helper.
5. Не хранить “магические строки” ассетов внутри AI/combat/story логики.

## Плохо
```cpp
loadTexture("assets/Enemies/Ghoul.PNG");

Лучше
C++

loadTexture(AssetPaths::enemy("ghoul.png"));
или

C++

loadTexture(assetResolver.resolve("enemies/ghoul.png"));
```

# 7. Централизованный Asset Resolver
Рекомендуется ввести один слой, отвечающий за построение и валидацию путей.

Его задачи
добавлять корень assets/;
нормализовать слэши;
опционально проверять регистр;
логировать missing resources;
поддерживать fallback assets;
упростить перенос старых путей.
Пример логических методов
enemy(name)
vehicle(name)
ui(name)
audio(name)
weather(name)
model(name)
data(name)