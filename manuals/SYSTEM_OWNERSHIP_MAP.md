# SYSTEM OWNERSHIP MAP

Цель: жёстко определить зоны ответственности подсистем в `Izometrical_Project`, чтобы при переносе механик из `GMyGameDoNotTouch` и `Video_Game_Izom` не возникало дублирующих центров логики.

---

## Главный принцип

У каждой важной области игры должен быть один основной owner.

Если одной и той же вещью “владеют” сразу несколько мест, появляются:
- дублирующийся код;
- двойная обработка событий;
- конфликты состояний;
- баги при save/load;
- сломанные интеграции.

---

# 1. Глобальная карта owner-систем

| Область | Основной owner | Вторичные связи | Что НЕ должно владеть этим |
|---|---|---|---|
| Main loop | EngineRuntime | UI, World, Gameplay systems | HUD, AI classes, random utility files |
| Player state | PlayerSystem | DamageSystem, InventorySystem, VehicleSeatSystem | UI, main.cpp giant logic |
| Combat flow | CombatSystem | DamageSystem, InventorySystem, Vehicle systems | UI, StorySystem |
| Final damage application | DamageSystem | CombatSystem, SurvivalSystem, Vehicle systems | Weapon render code, AI render code |
| AI perception/decision | AISystem | World queries, DamageSystem | UI, render code |
| Vehicle control | VehicleControlSystem | VehicleSeatSystem, VehicleDamageSystem | Player UI, StorySystem |
| Vehicle seats/roles | VehicleSeatSystem | PlayerSystem, VehicleControlSystem | Random input handlers |
| Vehicle subsystem damage | VehicleDamageSystem | DamageSystem, VehicleControlSystem | HUD as source of truth |
| Inventory/items | InventorySystem | LootSystem, PlayerSystem, SaveSystem | UI widgets only |
| Loot generation | LootSystem | InventorySystem, WorldSystem | Combat render code |
| World interactions | InteractionSystem | WorldSystem, StoryEventSystem | HUD directly |
| Story flags/events | StoryEventSystem | InteractionSystem, SaveSystem, WorldSystem | Terminal UI directly |
| Weather/environment state | EnvironmentSystem | AISystem, DamageSystem, Render | UI only |
| Breakables state | BreakableSystem | WorldSystem, LootSystem, SaveSystem | FX-only code |
| Save/load | SaveSystem | All major systems | UI alone, ad-hoc utility code |
| Profile/session | ProfileSystem / SessionSystem | SaveSystem, StoryEventSystem | Random gameplay files |
| Asset path resolution | AssetResolver | Render, Audio, UI, Content loaders | AI/combat/story code directly |
| UI state/rendering | UISystem | reads from all systems | owning gameplay truth |
| Tool/editor logic | ToolSystem | WorldSystem, ContentSystem | main gameplay loop directly |
| Networking | NetworkSystem | SessionSystem, entity replication layer | direct ownership of gameplay truth |

---

# 2. EngineRuntime ownership

## EngineRuntime владеет
- init/shutdown;
- frame lifecycle;
- ordering of updates;
- dispatch between systems;
- application-level timing;
- high-level scene/session bootstrap.

## EngineRuntime не владеет
- конкретной боевой логикой;
- правилами AI;
- инвентарём;
- сюжетными флагами;
- логикой урона.

## Почему это важно
`main.cpp` не должен быть “вторым gameplay god object”.

---

# 3. PlayerSystem ownership

## PlayerSystem владеет
- player state machine;
- on-foot movement;
- transitions:
  - mount
  - dismount
  - interact
  - heal
  - reload
- action gating;
- player intent interpretation.

## PlayerSystem не владеет
- финальным применением урона;
- лут-генерацией;
- погодой;
- финальной логикой техники;
- story progression как таковым.

## Взаимодействует с
- `InventorySystem`
- `DamageSystem`
- `VehicleSeatSystem`
- `InteractionSystem`
- `SurvivalSystem`

---

# 4. CombatSystem ownership

## CombatSystem владеет
- fire requests;
- melee/ranged attack execution;
- attack cooldown logic;
- projectile/hitscan resolution;
- explosion creation;
- weapon firing flow;
- cockpit fire requests.

## CombatSystem не владеет
- итоговым изменением HP/parts напрямую;
- сюжетными эффектами;
- состоянием UI;
- item database truth.

## Передаёт в
- `DamageSystem`
- `InventorySystem`
- `VehicleDamageSystem` при нужных кейсах
- `UISystem` только как событие/данные

---

# 5. DamageSystem ownership

## DamageSystem владеет
- единым путём применения урона;
- HP changes;
- armor interaction;
- part damage;
- subsystem damage;
- explosion damage application;
- environmental damage application;
- death/disable transitions;
- stress-related incoming hooks.

## DamageSystem не владеет
- тем, кто решил выстрелить;
- UI отрисовкой урона;
- story reward logic;
- загрузкой ассетов.

## Критическое правило
Ни player code, ни AI code, ни vehicle code не должны иметь собственный “тайный” финальный путь снятия HP в обход DamageSystem.

---

# 6. AISystem ownership

## AISystem владеет
- perception;
- awareness;
- aggro;
- target selection;
- decision state;
- archetype behavior;
- attack intent;
- disabled behavior;
- retreat/chase logic.

## AISystem не владеет
- world persistence;
- финальным уроном;
- UI;
- asset loading;
- story ownership.

## Подсобственные логические блоки
- `Perception`
- `AwarenessModel`
- `ArchetypeController`
- `AttackDecision`
- `AIDamageReaction`

---

# 7. VehicleControlSystem ownership

## VehicleControlSystem владеет
- движением техники;
- поворотом;
- управляющими ограничениями;
- реакцией на mobility-related damage;
- control state техники.

## Не владеет
- seat ownership;
- финальным subsystem damage;
- inventory;
- hangar modifications as database truth.

---

# 8. VehicleSeatSystem ownership

## VehicleSeatSystem владеет
- кто сидит в какой позиции;
- driver/gunner role;
- mount eligibility;
- dismount eligibility;
- seat swap transitions;
- allowed actions per seat.

## Не владеет
- движением техники;
- subsystem damage;
- story;
- UI как источником истины.

## Почему это важно
Именно здесь должен быть единственный источник правды для:
- player mounted state;
- seat role;
- occupancy consistency.

---

# 9. VehicleDamageSystem ownership

## VehicleDamageSystem владеет
- hull/core/subsystem health;
- sensors state;
- weapon state;
- mobility state;
- disable thresholds;
- integration с utility module damage if needed.

## Не владеет
- player input;
- HUD rendering;
- story;
- loot.

## Важно
HUD может только читать состояние техники, но не быть владельцем логики повреждений.

---

# 10. InventorySystem ownership

## InventorySystem владеет
- item stacks;
- ammo pools;
- equipped items;
- consumables;
- service kits;
- tapes;
- module items;
- quick slots/hotbar if present.

## Не владеет
- story progression;
- loot spawn tables;
- terminal UI;
- direct world placement logic.

## Важно
Все операции:
- consume item
- add item
- remove item
- check ammo
должны проходить через InventorySystem.

---

# 11. LootSystem ownership

## LootSystem владеет
- loot tables;
- rarity rolls;
- drop generation;
- crate drop logic;
- enemy loot generation;
- scripted reward generation.

## Не владеет
- инвентарём как хранилищем;
- story flags;
- breakable persistence.

---

# 12. InteractionSystem ownership

## InteractionSystem владеет
- поиск доступного interact target;
- interaction prompts;
- validation of interaction;
- запуск interaction command.

## Не владеет
- самой глубокой сюжетной логикой;
- содержимым terminal UI;
- save serialization.

## Делегирует
- terminal effect → `StoryEventSystem` / terminal subsystem
- loot container effect → `InventorySystem` / `LootSystem`
- vehicle entry → `VehicleSeatSystem`

---

# 13. StoryEventSystem ownership

## StoryEventSystem владеет
- story flags;
- zone progression;
- event preconditions;
- event postconditions;
- one-shot event state;
- scripted progression order.

## Не владеет
- terminal rendering;
- combat firing;
- inventory storage;
- movement input.

## Важно
Все события:
- CryoLocker
- Archive
- Garage
- Exterior
- FirstCombat
- Return
должны иметь owner здесь, а не быть размазаны по случайным cpp.

---

# 14. EnvironmentSystem ownership

## EnvironmentSystem владеет
- weather state;
- EtherFog;
- AcidRain;
- hazard intensity;
- visibility modifiers as environment data;
- environmental ticking.

## Не владеет
- AI decision напрямую;
- damage application напрямую;
- render-only fog visualization truth.

## Связи
- `AISystem` читает visibility data;
- `DamageSystem` применяет environmental damage;
- `RenderSystem/UISystem` визуализируют.

---

# 15. SurvivalSystem ownership

## SurvivalSystem владеет
- stress;
- second wind;
- soul line;
- survival thresholds;
- critical state transitions tied to survival rules.

## Не владеет
- базовым inventory storage;
- прямым fire logic;
- story progression.

## Важно
Если survival разрастётся, он должен быть отдельным owner, а не набором разрозненных bool-полей в player code.

---

# 16. BreakableSystem ownership

## BreakableSystem владеет
- destructible object states;
- destroyed/not destroyed;
- optional staged damage for breakables;
- persistence markers;
- break-triggered loot hooks.

## Не владеет
- visual FX entirely;
- audio playback as source of truth;
- world editor ownership.

---

# 17. SaveSystem ownership

## SaveSystem владеет
- serialization format;
- saveFormatVersion;
- migration pipeline;
- load validation;
- coordination of subsystem save/load.

## SaveSystem не владеет
- actual gameplay logic;
- item definitions;
- story design;
- AI decisions.

## Ключевое правило
Каждая major system должна экспортировать свои save/load данные,
но orchestration принадлежит SaveSystem.

---

# 18. ProfileSystem / SessionSystem ownership

## ProfileSystem владеет
- settings;
- meta progression;
- persistent unlocks;
- non-run-specific statistics.

## SessionSystem владеет
- текущее прохождение;
- runtime/session-level markers;
- active run state.

## Не должны смешиваться
- profile data
- save slot data
- temporary runtime state

---

# 19. AssetResolver ownership

## AssetResolver владеет
- canonical asset paths;
- path normalization;
- legacy aliases if temporarily needed;
- missing asset fallback resolution.

## Не владеет
- gameplay logic;
- AI logic;
- story triggers.

## Важно
AI/combat/story код не должен строить пути к ассетам вручную как источник истины.

---

# 20. UISystem ownership

## UISystem владеет
- visual state;
- HUD;
- menus;
- terminal presentation;
- pip-pad presentation;
- feedback overlays;
- prompts rendering.

## UISystem не владеет
- реальным количеством HP;
- реальным mounted state;
- реальным состоянием story flags;
- реальным содержимым inventory как источником истины.

## Правило
UI читает и отображает,
но не хранит окончательную gameplay-истину.

---

# 21. ToolSystem ownership

## ToolSystem владеет
- toolgun;
- editor overlays;
- undo/redo for tools;
- validation tools;
- export tools;
- debug/dev interactions.

## Не владеет
- production story progression;
- основным player progression;
- финальными gameplay правилами.

---

# 22. NetworkSystem ownership

## NetworkSystem владеет
- соединение;
- lobby/chat/voice if needed;
- replication transport layer;
- network session coordination.

## Не владеет
- core gameplay truth напрямую;
- story logic;
- raw asset logic.

## Важно
Иначе multiplayer сломает single-player архитектуру.

---

# 23. Ownership rules для пересечений

Если область затрагивает несколько систем, использовать правило:

## Кто принимает решение?
owner-system.

## Кто читает данные?
любая нужная subsystem через понятный интерфейс.

## Кто отображает?
обычно UI/render layer.

## Кто сериализует?
каждая subsystem под контролем SaveSystem.

---

# 24. Ownership конфликтные зоны

## 24.1. Player mounted state
- Owner: `VehicleSeatSystem`
- Reader: `PlayerSystem`, `UISystem`, `AISystem`, `SaveSystem`

## 24.2. Final HP / death
- Owner: `DamageSystem`
- Reader: `PlayerSystem`, `AISystem`, `UISystem`, `SaveSystem`

## 24.3. Story progression
- Owner: `StoryEventSystem`
- Reader: `InteractionSystem`, `TerminalSystem/UI`, `SaveSystem`

## 24.4. Vehicle role capabilities
- Owner: `VehicleSeatSystem`
- Reader: `CombatSystem`, `UISystem`, `PlayerSystem`

## 24.5. Weather impact
- Owner: `EnvironmentSystem`
- Reader: `AISystem`, `DamageSystem`, `RenderSystem`

## 24.6. Loot contents
- Owner: `LootSystem`
- Reader/consumer: `InventorySystem`, `WorldSystem`, `SaveSystem`

---

# 25. Ownership anti-patterns

## Плохо
- UI сам меняет HP “потому что кнопка лечения”.
- AI сам списывает HP игроку напрямую.
- `main.cpp` сам решает story progression.
- vehicle HUD хранит реальное состояние модулей.
- terminal screen сам меняет storyline flags без owner event system.
- save system сам выдумывает gameplay-поля, которых не знает subsystem.

---

# 26. Практическое применение карты ownership

При переносе каждого файла нужно задавать вопросы:

1. Какой системе это принадлежит?
2. Кто источник истины?
3. Кто только читатель?
4. Не дублирует ли этот файл уже существующего owner-а?
5. Не нарушает ли он архитектурную границу?

---

# 27. Что считать хорошей ownership-моделью

Модель хорошая, если:
- можно быстро понять, где искать баг;
- нет двух мест, “владеющих” одним состоянием;
- save/load не спорит с runtime;
- UI не превращается в gameplay controller;
- перенос legacy-кода становится управляемым.

---

# Следующий логичный файл

После этого можно делать:

`MERGE_EXECUTION_CHECKLIST.md`

В нём будет пошаговый практический чеклист:
- что открыть;
- что сравнить;
- что заменить;
- что проверить после каждого изменения;
- как двигаться без поломки репозитория.