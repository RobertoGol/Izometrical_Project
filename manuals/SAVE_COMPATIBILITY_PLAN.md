# SAVE COMPATIBILITY PLAN

Цель: спроектировать устойчивую систему сохранений для `Izometrical_Project`, чтобы перенос механик из `GMyGameDoNotTouch` и `Video_Game_Izom` не ломал прохождения, сюжет и состояние мира.

---

## Главный принцип

Нельзя хранить сейв как “случайный снимок текущих struct” без явной версии и схемы.

После переноса:
- AI,
- техники,
- стресса,
- погоды,
- сюжетных флагов,
- строительства,
- breakables,
- inventory/loot,
- новых UI/terminal state hooks

старый неверсированный сейв почти гарантированно станет ломким.

---

# 1. Цели системы сохранений

Система должна:

1. корректно сохранять состояние игры;
2. корректно восстанавливать состояние игры;
3. переживать добавление новых полей;
4. поддерживать versioning;
5. поддерживать migration хотя бы для ближайших версий;
6. не терять сюжетный прогресс;
7. не дублировать лут/разрушения/постройки после загрузки;
8. быть достаточно понятной для отладки.

---

# 2. Что именно должно сохраняться

Сейв должен охватывать минимум следующие блоки:

1. Save metadata
2. Player state
3. Inventory / items / ammo
4. World state
5. Story / quest / progression flags
6. Hostile/entity persistent state
7. Vehicle/Titan state
8. Weather/environment state
9. Breakables state
10. Build/C.A.M.P. state
11. Session/runtime state
12. Optional profile-linked references

---

# 3. Save Metadata

## Обязательные поля
- `saveFormatVersion`
- `buildVersion` или `contentVersion`
- `timestamp`
- `playtimeSeconds`
- `currentZoneId`
- `currentSpawnOrCheckpointId`
- `saveType`:
  - manual
  - auto
  - checkpoint
  - quicksave
- optional `safeModeFlags`

## Зачем это нужно
- понимать, какой миграцией пользоваться;
- понимать, из какой версии игры сейв пришёл;
- иметь диагностическую информацию.

---

# 4. Player State

## Что сохранять
- позиция;
- ориентация;
- текущее состояние player state machine;
- HP / armor / stress;
- second wind / soul line related flags;
- активные статусы;
- активное оружие;
- прогресс reload/heal action, если поддерживается;
- mounted state;
- active vehicle seat;
- cooldowns, если это влияет на честность восстановления;
- важные runtime capability flags.

## Особое внимание
Если игрок в технике на момент сейва:
- нужно хранить связь player ↔ vehicle;
- нужно знать seat role;
- после загрузки не должно быть дубликатов игрока и техники.

---

# 5. Inventory / Items / Ammo

## Что сохранять
- все item stacks;
- equipped items;
- ammo pools;
- consumables;
- rations;
- service kits;
- tapes;
- vehicle modules если они инвентарные;
- hotbar/quick slots при наличии.

## Требование
Все предметы должны сохраняться через устойчивые:
- `itemDefinitionId`
- количество
- optional per-instance state

Не через “индекс в локальном массиве”, который может измениться.

---

# 6. World State

## Что сохранять
- активная зона;
- открытые/закрытые двери;
- активированные терминалы;
- контейнеры и состояние их лута;
- интерактивные world objects;
- активированные триггеры;
- world flags;
- текущие scripted progression anchors.

## Проблема
Если сохранять мир слишком “по месту” без стабильных world object ids,
после изменения карты сейвы станут ломаться.

## Решение
У значимых world objects должны быть устойчивые id:
- `worldObjectId`
- `zoneObjectId`
- или другой стабильный persistence key

---

# 7. Story / Quest / Progression State

## Что сохранять
- все story flags;
- все zone event flags;
- progression stages;
- clearance level/state;
- BT-72 recovery progression;
- archive/garage/exterior/return progression;
- terminal sync state;
- одноразовые катсценоподобные события, если они есть.

## Почему это критично
Если хотя бы часть флагов забыть:
- события повторятся,
- события не сработают,
- игрок получит soft lock.

## Требование
Сюжетные флаги должны жить в централизованном реестре, а не быть размазаны по коду.

---

# 8. Hostile / Persistent Entity State

## Что сохранять
Только для сущностей, которые должны переживать загрузку:

- entity persistent id;
- тип;
- позиция/состояние;
- HP;
- awareness/aggro state при необходимости;
- disabled parts;
- robot/mechanical subsystem damage;
- alive/dead/disabled;
- loot-generated state если нужен;
- ownership/faction если это динамично.

## Необязательно сохранять
Не все временные сущности обязаны сериализоваться.
Например:
- transient projectiles
- короткоживущие FX entities

---

# 9. Vehicle / Titan State

## Что сохранять
- persistent vehicle id;
- позиция/ориентация;
- hull/core health;
- subsystem states:
  - sensors
  - weapon
  - mobility
  - other cores/modules
- thermal load;
- equipped utility modules;
- repair state;
- mounted crew relation;
- disabled state;
- hangar-related modification state.

## Особая опасность
Если техника и игрок сохраняются раздельно без связки:
- возможен дубликат;
- игрок окажется “и внутри, и снаружи”;
- seat ownership сломается.

---

# 10. Weather / Environment State

## Что сохранять
- активный weather type;
- интенсивность/таймеры;
- зоны воздействия;
- environmental hazard flags;
- visibility modifiers, если они persistent enough.

## Примеры
- EtherFog active
- AcidRain active
- lingering environmental severity

---

# 11. Breakables State

## Что сохранять
- список разрушенных объектов;
- степень повреждения, если нужно;
- spawned remains/debris только если это реально влияет на gameplay;
- drop-consumed state.

## Главное
Не обязательно сохранять всю физическую мелочь.
Важно сохранить gameplay-смысл:
- объект цел / разрушен;
- лут уже выпал / уже подобран.

---

# 12. Build / C.A.M.P. State

## Что сохранять
- список построенных объектов;
- их тип;
- позиция;
- ориентация;
- состояние;
- owner/faction если применимо;
- ammo/health у турелей при необходимости;
- connection/snap metadata, если она нужна.

## Требование
Каждый build object должен иметь persistent id.

---

# 13. Session / Runtime State

Это не всё то же самое, что профиль.

## Что сюда может входить
- текущий активный чекпоинт;
- последнее безопасное место;
- runtime phase markers;
- локальные cooldown snapshots;
- текущие world modifiers;
- активная музыка/радио не обязательно, но иногда полезно;
- некоторые “мягкие” сессионные данные.

## Важно
Надо отделять:
- обязательно сериализуемое,
- восстанавливаемое по косвенным данным,
- вообще не нужное в сейве.

---

# 14. Profile Data vs Save Data

## Profile Data
Это мета-данные аккаунта/игрока:
- настройки;
- глобальные разблокировки;
- статистика;
- возможно achievements;
- persistent options.

## Save Data
Это конкретное прохождение:
- где игрок;
- что с сюжетом;
- что в инвентаре;
- что разрушено;
- какая погода;
- в каком состоянии техника.

## Правило
Нельзя хаотично смешивать profile и save state.

---

# 15. Versioning

## Обязательное поле
Каждый сейв должен содержать:
- `saveFormatVersion`

## Дополнительно полезно
- `contentVersion`
- `schemaVersion`
- `buildVersion`

## Правило
Любое изменение формата сейва должно сопровождаться:
1. увеличением версии;
2. обновлением миграции или fallback-логики;
3. записью в changelog/save notes.

---

# 16. Migration Strategy

## Подход
Для каждой старой версии сейва нужно понимать:
- что изменилось;
- какие поля добавлены;
- какие поля переименованы;
- какие поля теперь вычисляются иначе;
- какие поля можно заполнить значениями по умолчанию.

## Примерный pipeline
1. прочитать metadata;
2. определить версию;
3. если версия старая — прогнать миграции по цепочке;
4. получить актуальную внутреннюю структуру;
5. валидировать;
6. если возможно — загрузить;
7. если нет — safe fail с диагностикой.

## Принцип
Лучше:
- загрузить старый сейв с частью safe defaults,

чем:
- молча загрузить сломанное состояние.

---

# 17. Safe Defaults

Для новых полей нужно определить значения по умолчанию.

## Примеры
- новый weather state → `None`
- новый skill flag → `false`
- новый vehicle subsystem → `full health`
- новый story flag → `not triggered`
- новый module slot → `empty`

## Важно
Safe default не должен:
- ломать сюжет;
- давать чит;
- убивать игрока сразу после загрузки.

---

# 18. Validation on Load

После загрузки сейва нужно проверять:

## Player
- валидна ли позиция;
- валиден ли HP диапазон;
- нет ли невозможного state machine state;
- нет ли mounted state без vehicle.

## Vehicle
- существует ли vehicle id;
- корректны ли subsystem ranges;
- нет ли seat occupancy conflicts.

## World
- существует ли текущая зона;
- валидны ли object ids;
- нет ли неизвестных object states.

## Story
- флаги согласованы;
- текущий progression stage допустим;
- нет ли взаимоисключающих состояний.

## Inventory
- item ids существуют;
- counts допустимы;
- equipped items валидны.

---

# 19. Failure Modes

Нужно заранее определить поведение, если сейв частично повреждён.

## Допустимые стратегии
1. отказ в загрузке с понятной ошибкой;
2. safe repair с предупреждением;
3. fallback на checkpoint;
4. сброс только повреждённой подсистемы, если это реально безопасно.

## Недопустимо
- молча грузить мусор;
- спавнить дубликаты игрока/техники;
- продолжать игру с неконсистентным story state без предупреждения.

---

# 20. Save Granularity

## Manual Save
Полный сейв.

## Auto Save
Полный или почти полный сейв в безопасных точках.

## Checkpoint Save
Можно хранить компактнее, но лучше не делать отдельную несовместимую схему без большой причины.

## Quick Save
Тоже должен использовать ту же базовую save schema.

---

# 21. Suggested Save Structure

Пример логического деления:

- `SaveMetadata`
- `PlayerSaveData`
- `InventorySaveData`
- `WorldSaveData`
- `StorySaveData`
- `VehicleSaveData`
- `WeatherSaveData`
- `PersistentEntitiesSaveData`
- `BreakablesSaveData`
- `BuildSaveData`
- `SessionSaveData`

---

# 22. Test Matrix for Saves

## Базовые тесты
1. save/load in idle state
2. save/load after movement
3. save/load after combat
4. save/load after taking damage
5. save/load after heal
6. save/load after reload
7. save/load while mounted
8. save/load after dismount
9. save/load with damaged vehicle
10. save/load during/after weather change
11. save/load after story trigger
12. save/load after terminal interaction
13. save/load after loot pickup
14. save/load after breakable destruction
15. save/load after build placement

## Расширенные тесты
1. load old-version save into new build
2. missing optional field handling
3. unknown deprecated flag handling
4. invalid item id recovery
5. invalid object id recovery
6. corrupted mounted relation recovery

---

# 23. Save Compatibility Policy

Рекомендуемая политика:

## На ранних фазах
Можно честно писать:
- формат нестабилен,
- backward compatibility ограничена.

## После стабилизации ядра
Нужно:
- фиксировать versioning;
- поддерживать миграции хотя бы для последних версий;
- не ломать сейвы при каждом втором коммите.

---

# 24. Minimum Immediate Actions

1. Ввести `saveFormatVersion`.
2. Разделить profile/save/session.
3. Вынести story flags в централизованный блок.
4. Определить persistent ids для:
   - world objects
   - vehicles
   - hostile entities
   - build objects
5. Определить safe defaults для новых систем.
6. Подготовить load validation.
7. Добавить базовый журнал изменений формата.

---

# 25. Что считать хорошей системой сохранений

Система хорошая, если:

- можно безопасно добавлять новые механики;
- сейвы не разваливаются от каждого нового поля;
- story/world/vehicle state восстанавливаются корректно;
- ошибочные сейвы диагностируются;
- нет дублирования сущностей и событий после загрузки.

---

# Следующий файл

После этого нужен:

`ASSET_PATH_NORMALIZATION.md`

В нём нужно описать:
- единый стандарт имён папок и файлов;
- исправление `Enemies/Vehicles` регистра;
- правила путей для текстур/моделей/аудио;
- как избежать битых путей после слияния трёх репозиториев.