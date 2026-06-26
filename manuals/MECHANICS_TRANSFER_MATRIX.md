# MECHANICS TRANSFER MATRIX

Цель: зафиксировать все ключевые механики, их источник, статус в `Izometrical_Project`, способ переноса и приоритет интеграции.

---

## Легенда статусов

- `ABSENT` — механики нет
- `PARTIAL` — есть частично
- `OUTDATED` — есть старая версия
- `BROKEN` — есть, но сломана
- `READY` — работает
- `UNKNOWN` — нужно уточнить по коду/веткам/коммитам

---

## Легенда действий

- `PORT` — перенос как базовой реализации
- `ADAPT` — перенос с адаптацией под новую архитектуру
- `REWRITE` — переписать по мотивам исходной реализации
- `DECOMPOSE` — сначала разломать giant-file реализацию на части
- `DEFER` — отложить до стабилизации ядра

---

## Легенда приоритетов

- `P0` — блокирует рабочую игру
- `P1` — сильно влияет на core gameplay
- `P2` — сильно улучшает глубину/системность
- `P3` — инструменты, расширения, поздние подсистемы

---

# 1. CORE / ENGINE

| Механика | Источник | Статус в Izometrical_Project | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| Main loop consistency | Все 3 репо | UNKNOWN | ADAPT | P0 | Нужно выбрать единый update/render lifecycle |
| Engine bootstrap | Все 3 репо | UNKNOWN | ADAPT | P0 | Инициализация subsystems должна быть унифицирована |
| Game state orchestration | GMyGameDoNotTouch + Izometrical_Project | UNKNOWN | REWRITE | P0 | Скорее всего размазано по runtime |
| Entity lifecycle | Все 3 репо | UNKNOWN | REWRITE | P0 | Нужен единый жизненный цикл объектов |
| Timing / timestep | Все 3 репо | UNKNOWN | ADAPT | P0 | Проверить fixed/semi-fixed time step |
| Asset bootstrap | Izometrical_Project + Video_Game_Izom | PARTIAL | ADAPT | P0 | Особо важно из-за путей и генерации ресурсов |

---

# 2. PLAYER

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| Базовое движение игрока | Izometrical_Project / Video_Game_Izom | PARTIAL | ADAPT | P0 | Проверить коллизии и состояние |
| Пеший режим | Все 3 репо | PARTIAL | ADAPT | P0 | Должен быть согласован с vehicle mode |
| Взаимодействие с объектами | GMyGameDoNotTouch | PARTIAL | ADAPT | P0 | Терминалы, контейнеры, world prompts |
| Лечение | GMyGameDoNotTouch | ABSENT | PORT | P1 | С анимационными/состоянийными переходами |
| Перезарядка | GMyGameDoNotTouch | ABSENT | PORT | P1 | Интегрировать с combat state |
| Rations / consumables | GMyGameDoNotTouch | ABSENT | PORT | P1 | Баффы и выживание |
| Стресс игрока | GMyGameDoNotTouch | ABSENT | ADAPT | P1 | Связать с survival и critical states |
| Second Wind | GMyGameDoNotTouch | ABSENT | PORT | P1 | Производная от stress/critical damage |
| SoulLine | GMyGameDoNotTouch | ABSENT | PORT | P1 | Финальный шанс/срыв смерти |
| Mount / Dismount | GMyGameDoNotTouch + TitanAI changes | OUTDATED | ADAPT | P0 | Уже явно упоминалось |
| Seat Swap | GMyGameDoNotTouch | ABSENT | PORT | P1 | Водитель ↔ стрелок |

---

# 3. WEAPON / COMBAT

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| Базовая стрельба | Все 3 репо | PARTIAL | ADAPT | P0 | Нужно унифицировать pipeline |
| Damage pipeline | Все 3 репо | UNKNOWN | REWRITE | P0 | Один из самых опасных конфликтов |
| Reload logic | GMyGameDoNotTouch | ABSENT | PORT | P1 | Не только UI, но и state timing |
| Heal action integration | GMyGameDoNotTouch | ABSENT | ADAPT | P1 | Завязать на item/action state |
| Explosion damage | GMyGameDoNotTouch | PARTIAL | ADAPT | P1 | Проверить радиусы/LOS |
| Shock Wave | GMyGameDoNotTouch | ABSENT | PORT | P1 | Взрывная ударная волна |
| Recoil / aim disruption | GMyGameDoNotTouch | PARTIAL | ADAPT | P1 | Связать с shock wave и damage |
| Thermal load (tank weapon) | GMyGameDoNotTouch | ABSENT | PORT | P1 | Перегрев и темп стрельбы |
| fireFromCockpit | TitanAI newer version | OUTDATED | PORT | P1 | Уже упоминалось в нужной версии |
| Ammo management | Все 3 репо | PARTIAL | ADAPT | P0 | Привести к единой item/ammo модели |

---

# 4. ENEMY / AI

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| TitanAI baseline | Izometrical_Project / newer TitanAI | OUTDATED | ADAPT | P0 | Обновить до новой версии |
| HostileAI framework | GMyGameDoNotTouch | ABSENT | DECOMPOSE | P0 | Вероятно сидит в giant runtime |
| VerminRush | GMyGameDoNotTouch | ABSENT | PORT | P1 | Тип врага |
| GhoulRush | GMyGameDoNotTouch | ABSENT | PORT | P1 | Тип врага |
| HumanTactical | GMyGameDoNotTouch | ABSENT | PORT | P1 | Тактический AI |
| RobotControl | GMyGameDoNotTouch | ABSENT | PORT | P1 | Роботизированный hostile logic |
| Awareness / Aggro | GMyGameDoNotTouch | ABSENT | PORT | P1 | Обнаружение и нарастание угрозы |
| Target selection | Все 3 репо | UNKNOWN | ADAPT | P1 | Связать с player/vehicle contexts |
| Attack cooldowns | GMyGameDoNotTouch | ABSENT | PORT | P1 | Не потерять stateful combat logic |
| Mechanical hostile damage | GMyGameDoNotTouch | ABSENT | ADAPT | P1 | Part-based damage for robots |
| Disabled / crippled states | GMyGameDoNotTouch | ABSENT | PORT | P1 | Слом сенсоров/оружия/мобильности |

---

# 5. VEHICLE / TITAN / TANK

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| Titan / Tank control | GMyGameDoNotTouch + Izometrical_Project | PARTIAL | ADAPT | P0 | Центральная система |
| Driver / Gunner role split | GMyGameDoNotTouch | ABSENT | PORT | P1 | Роли и ограничения |
| Seat swap | GMyGameDoNotTouch | ABSENT | PORT | P1 | Должно работать стабильно |
| tryMount / dismount | newer TitanAI | OUTDATED | PORT | P0 | Уже явно требуется |
| Vehicle damage by parts | GMyGameDoNotTouch | ABSENT | ADAPT | P1 | Компоненты и состояние |
| Vehicle disable states | GMyGameDoNotTouch | ABSENT | PORT | P1 | Потеря функций по частям |
| Bucket Rig | GMyGameDoNotTouch | ABSENT | PORT | P1 | Utility module |
| Ram Shield | GMyGameDoNotTouch | ABSENT | PORT | P1 | Utility module |
| Tow Coupler | GMyGameDoNotTouch | ABSENT | PORT | P1 | Utility module |
| Hangar repair | GMyGameDoNotTouch | ABSENT | PORT | P2 | Сервис танка |
| Hangar modification | GMyGameDoNotTouch | ABSENT | PORT | P2 | Модули/апгрейды |
| Service kits | GMyGameDoNotTouch | ABSENT | PORT | P2 | Расходники ремонта |
| Cockpit combat integration | GMyGameDoNotTouch | PARTIAL | ADAPT | P1 | Стрельба/состояния/приборы |
| Core-by-Core damage / systems core | newer TitanAI mention | OUTDATED | ADAPT | P1 | Проверить фактическую реализацию |

---

# 6. WORLD / INTERACTION / EVENTS

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| Terminal interaction | GMyGameDoNotTouch | ABSENT | PORT | P1 | Мир и сюжет |
| Terminal Sync / Pip-Pad sync | GMyGameDoNotTouch | ABSENT | PORT | P2 | Системная интеграция |
| Zone triggers | GMyGameDoNotTouch | ABSENT | PORT | P1 | Основа сюжетных зон |
| CryoLocker event | GMyGameDoNotTouch | ABSENT | PORT | P2 | Сюжетный блок |
| Archive event | GMyGameDoNotTouch | ABSENT | PORT | P2 | Сюжетный блок |
| Garage event | GMyGameDoNotTouch | ABSENT | PORT | P2 | Сюжетный блок |
| Exterior event | GMyGameDoNotTouch | ABSENT | PORT | P2 | Сюжетный блок |
| FirstCombat event | GMyGameDoNotTouch | ABSENT | PORT | P2 | Ввод в боёвку |
| Return event | GMyGameDoNotTouch | ABSENT | PORT | P2 | Сюжетное возвращение |
| Context interaction prompts | Все 3 репо | PARTIAL | ADAPT | P1 | Унифицировать |

---

# 7. SURVIVAL / STATUS / ENVIRONMENT

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| Stress system | GMyGameDoNotTouch | ABSENT | ADAPT | P1 | Влияет на survival |
| Second Wind | GMyGameDoNotTouch | ABSENT | PORT | P1 | Crit survival |
| SoulLine | GMyGameDoNotTouch | ABSENT | PORT | P1 | Last-chance system |
| Rations buffs | GMyGameDoNotTouch | ABSENT | PORT | P1 | Item-driven status buffs |
| EtherFog | GMyGameDoNotTouch | ABSENT | PORT | P1 | Gameplay + visibility |
| AcidRain | GMyGameDoNotTouch | ABSENT | PORT | P1 | Damage/hazard/weather |
| Visibility modifiers | GMyGameDoNotTouch | ABSENT | ADAPT | P1 | AI + render + player perception |
| Environmental damage | GMyGameDoNotTouch | ABSENT | PORT | P1 | Hazard integration |

---

# 8. BREAKABLES / PHYSICS REACTION

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| Reactive breakables framework | GMyGameDoNotTouch | ABSENT | PORT | P1 | Общая система |
| Glass break | GMyGameDoNotTouch | ABSENT | PORT | P2 | Частный случай |
| Vegetation reaction | GMyGameDoNotTouch | ABSENT | PORT | P2 | Частный случай |
| Crate destruction | GMyGameDoNotTouch | ABSENT | PORT | P2 | Частный случай |
| Physics impulse response | GMyGameDoNotTouch | ABSENT | ADAPT | P2 | Для wave/explosions/hits |

---

# 9. INVENTORY / LOOT / ITEMS

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| Inventory baseline | Izometrical_Project + GMyGameDoNotTouch | PARTIAL | ADAPT | P0 | Основа |
| Loot tables | GMyGameDoNotTouch | ABSENT | PORT | P2 | Таблицы по тирам |
| Loot generator | GMyGameDoNotTouch | ABSENT | PORT | P2 | Common → Legendary |
| Item definitions | Все 3 репо | UNKNOWN | REWRITE | P0 | Нужна единая item schema |
| Service kits | GMyGameDoNotTouch | ABSENT | PORT | P2 | Предметы ремонта |
| Tapes | GMyGameDoNotTouch | ABSENT | PORT | P2 | Коллекционные/сюжетные |
| Rations | GMyGameDoNotTouch | ABSENT | PORT | P1 | Предмет + бафф |
| Module items | GMyGameDoNotTouch | ABSENT | PORT | P2 | Utility/hangar integration |

---

# 10. UI / HUD / PIP-PAD / TERMINAL

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| HUD baseline | Izometrical_Project | PARTIAL | ADAPT | P0 | Основа интерфейса |
| Vehicle HUD | GMyGameDoNotTouch | ABSENT | PORT | P1 | Для техники |
| Pip-Pad baseline | GMyGameDoNotTouch | ABSENT | PORT | P2 | Системный интерфейс |
| Pip-Pad themes | GMyGameDoNotTouch | ABSENT | PORT | P2 | Кастомизация |
| Pip-Pad display modes | GMyGameDoNotTouch | ABSENT | PORT | P2 | Режимы отображения |
| Wearable integration | GMyGameDoNotTouch | ABSENT | PORT | P2 | Если реально используется |
| Terminal UI | GMyGameDoNotTouch | ABSENT | PORT | P1 | Нужен для world/story |
| Radio / tape UI feedback | GMyGameDoNotTouch | ABSENT | PORT | P2 | Логи/подсказки/субтитры |

---

# 11. STORY / PROGRESSION / SKILLS

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| Story route baseline | GMyGameDoNotTouch | ABSENT | DECOMPOSE | P2 | Нельзя копировать вслепую |
| BT-72 recovery route | GMyGameDoNotTouch | ABSENT | PORT | P2 | Ключевой сюжетный блок |
| Clearance progression | GMyGameDoNotTouch | ABSENT | PORT | P2 | Прогресс допусков |
| Surface progression | GMyGameDoNotTouch | ABSENT | PORT | P2 | Продвижение наружу |
| Skill framework | GMyGameDoNotTouch | ABSENT | PORT | P2 | Основа для пассивок |
| ArchiveSync | GMyGameDoNotTouch | ABSENT | PORT | P2 | Passive skill |
| FootKill | GMyGameDoNotTouch | ABSENT | PORT | P2 | Passive skill |
| TankAction | GMyGameDoNotTouch | ABSENT | PORT | P2 | Passive skill |
| StressSurvival | GMyGameDoNotTouch | ABSENT | PORT | P2 | Passive skill |

---

# 12. SAVE / LOAD / PROFILE / SESSION

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| Save system baseline | Izometrical_Project + GMyGameDoNotTouch | UNKNOWN | REWRITE | P0 | Нужна версия формата |
| Load system baseline | Izometrical_Project + GMyGameDoNotTouch | UNKNOWN | REWRITE | P0 | С миграцией |
| Profile data | GMyGameDoNotTouch | ABSENT | PORT | P2 | Meta progression |
| Session state | GMyGameDoNotTouch | ABSENT | PORT | P1 | Состояние текущего прохождения |
| Save versioning | GMyGameDoNotTouch | ABSENT | REWRITE | P0 | Обязательно |
| Save migration | GMyGameDoNotTouch | ABSENT | REWRITE | P1 | Иначе всё сломается |
| World persistence | Все 3 репо | UNKNOWN | ADAPT | P1 | События, лут, разрушения |
| Quest flag serialization | GMyGameDoNotTouch | ABSENT | PORT | P1 | Для story/event stability |

---

# 13. BUILDING / TOOLGUN / WORLD EDITOR

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| C.A.M.P. build system | Video_Game_Izom | PARTIAL | ADAPT | P2 | Уже отмечено как частично |
| Wall placement | Video_Game_Izom | PARTIAL | PORT | P2 | Подсистема строительства |
| Turret placement | Video_Game_Izom | ABSENT | PORT | P2 | Подсистема строительства |
| Crate placement | Video_Game_Izom | ABSENT | PORT | P2 | Подсистема строительства |
| Build validation | Video_Game_Izom | ABSENT | ADAPT | P2 | Коллизии/правила |
| Snap logic | Video_Game_Izom | ABSENT | PORT | P2 | Удобство placement |
| ToolGun | Video_Game_Izom | ABSENT | PORT | P3 | Runtime editor tool |
| World Editor | GMyGameDoNotTouch | ABSENT | DECOMPOSE | P3 | Undo/redo/semantic/export |
| Undo / Redo | GMyGameDoNotTouch | ABSENT | PORT | P3 | Editor-only likely |
| Semantic authoring | GMyGameDoNotTouch | ABSENT | DEFER | P3 | После стабилизации |
| Validation | GMyGameDoNotTouch | ABSENT | PORT | P3 | Полезно для editor |
| Export | GMyGameDoNotTouch | ABSENT | PORT | P3 | Финал editor pipeline |

---

# 14. NETWORK / MULTIPLAYER

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| Winsock baseline | Video_Game_Izom | ABSENT | DEFER | P3 | Не трогать до стабилизации SP |
| Lobby | GMyGameDoNotTouch / Video_Game_Izom | ABSENT | DEFER | P3 | Поздняя фаза |
| Chat | GMyGameDoNotTouch / Video_Game_Izom | ABSENT | DEFER | P3 | Поздняя фаза |
| Voice | GMyGameDoNotTouch | ABSENT | DEFER | P3 | Поздняя фаза |
| Session services | GMyGameDoNotTouch | ABSENT | DEFER | P3 | Поздняя фаза |
| Delivery services | GMyGameDoNotTouch | ABSENT | DEFER | P3 | Поздняя фаза |
| Up to 20 players logic | Video_Game_Izom | ABSENT | DEFER | P3 | Только после архитектуры simulation |

---

# 15. OPTIMIZATION / DATA / ASSETS

| Механика | Источник | Статус | Действие | Приоритет | Комментарий |
|---|---|---:|---|---|---|
| SpatialGrid | Video_Game_Izom | ABSENT | DEFER | P3 | Сначала стабилизировать entity/world |
| Broadphase helpers | Video_Game_Izom | UNKNOWN | DEFER | P3 | Проверить наличие |
| OBJ model loading | Video_Game_Izom | ABSENT | PORT | P3 | Полезно, но не блокирует ядро |
| Prefab Library | GMyGameDoNotTouch | ABSENT | PORT | P2 | Важно для контента |
| TextureGenerator integration | newer main.cpp mention | OUTDATED | ADAPT | P1 | Нужно сверить с текущим main |
| Asset path normalization | Все 3 репо | BROKEN | REWRITE | P0 | Enemies/Vehicles case issue |
| Fallback assets | Все 3 репо | UNKNOWN | ADAPT | P2 | Защита от битых ресурсов |

---

# 16. НЕМЕДЛЕННЫЕ ЗАДАЧИ ПЕРВОЙ ВОЛНЫ

## P0 — делать сразу
1. Обновить `TitanAI.hpp` до новой версии
2. Обновить `src/main.cpp` до новой версии
3. Нормализовать asset paths и регистр директорий
4. Выстроить единый main loop
5. Зафиксировать единый damage pipeline
6. Зафиксировать базовый player/vehicle split
7. Привести inventory/items к единой схеме
8. Зафиксировать save/load основу с version field

---

## P1 — вторая волна после стабилизации ядра
1. HostileAI framework
2. Awareness / Aggro
3. Mechanical hostile damage
4. Stress / SecondWind / SoulLine
5. Heal / Reload
6. Weather
7. Tank utility modules
8. Seat swap
9. Shock wave
10. Thermal load
11. Terminal interaction
12. Vehicle HUD / cockpit integration

---

## P2 — системное расширение
1. Hangar
2. Story routes / zone events
3. Skills
4. Loot generator
5. Pip-Pad
6. Radio / Tape
7. Prefab library
8. Profile / Session meta systems
9. C.A.M.P. full building system

---

## P3 — поздние подсистемы
1. ToolGun
2. World Editor
3. Multiplayer
4. SpatialGrid
5. OBJ loader polishing
6. Advanced authoring/export toolchain

---

# 17. Основные решения заранее

## Что почти наверняка нельзя просто копировать giant-file блоком
- HostileAI из огромного runtime
- Story/event routes
- Save/load/profile/session
- Editor systems
- Damage pipeline

## Что можно переносить ближе к прямому порту, но всё равно с адаптацией
- tryMount / dismount
- seat swap
- utility modules
- weather effects
- rations
- skills
- terminal interaction
- loot tables
- tape/radio content logic

## Что обязательно сначала стабилизировать архитектурно
- main loop
- entity lifecycle
- damage
- player/vehicle state split
- save schema
- asset path scheme

---

# 18. Следующий файл

После этой матрицы нужен файл:

`TARGET_ARCHITECTURE.md`

В нём будет:
- целевая структура проекта;
- основные подсистемы;
- как разделить gameplay, world, ai, ui, save, vehicle;
- какие данные где должны жить;
- как избежать конфликтов между legacy-системами.