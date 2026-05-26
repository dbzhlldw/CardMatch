# CardMatch 程序设计文档

## 设计总结

CardMatch 是一款纸牌消除游戏，基于 cocos2d-x 3.17.2（C++）开发。

整体设计以职责分离、数据驱动、扩展低成本为核心目标，经过多轮重构，最终形成以下几条关键设计决策：

1. **会话独立化**：游戏状态采用 GameSession 进行统一管理，不依赖全局共享状态，而是一局游戏对应一个 Session。每个 Session 独立维护 LevelDef、GameModel 与 GameController 的生命周期，实现状态隔离，避免多局运行时产生状态污染问题，同时支持重开、重复创建以及未来多实例扩展。
2. **视图逻辑分离**：在界面层（View）与控制层（Controller）之间引入 Presenter 作为逻辑门面，使界面表现与业务逻辑职责分离。View 仅负责渲染与反馈，只读 DTO 刷新而不直接访问 Model；Presenter 负责交互逻辑决策、规则判断以及状态组织。
3. **布局数据化**：关卡布局（LayoutDef）与关卡规则（LevelDef）从代码逻辑中抽离，通过配置数据描述牌面坐标、层级关系和遮挡依赖，而不是在逻辑代码中硬编码布局信息。后续新增关卡布局时只需添加一个配置文件，核心流程无需改动。
4. **规则与回退机制模块化**：桌面牌点击规则抽象为按 CardKind 注册的 Handler 集合；状态变更统一为 Command，由 UndoManager 门面管理历史栈。新增桌面牌类型或回退功能类型时只需扩展对应模块，而无需修改界面层或核心业务流程
5. **卡牌外观与行为分离**：同一种牌的「怎么响应点击」与「怎么画」分开配置。行为在 Handler 注册表，外观在 CardAppearance。View 创建牌面时只查外观配置，Presenter 会把类型传给 View。

注：窗口高度受设计尺寸约束，但开发设备屏幕可用高度不足，因此加入了 macOS 屏幕可用高度检测，在启动时动态收缩窗口，使游戏内容完整可见。

---

## 一、整体结构

```
AppDelegate
    └── GameScene                                                    ← 视图层：cocos 节点、动画、触摸
            ↕  ViewState / PresenterOutcome / cardId（ViewTypes.h）
            └── GamePresenter                                        ← 中间层：逻辑门面，无 cocos
                    └── GameSession                                  ← 会话层：一局生命周期
                            ├── GameModel                            ← Model：纯数据，无 cocos
                            └── GameController                       ← Controller：规则校验
                                    ├── TableauCardHandlerRegistry   ← 点击规则注册表（CardKind → Handler）
                                    ├── MatchCommand / DrawCommand   ← 可撤销命令
                                    └── UndoManager / CommandHistory ← 命令门面 + 历史栈
            ↑
      data: LevelDef / LayoutDef / CardAppearance（CardKind → 外观）
```

### 目录结构

| 目录/文件 | 层级 | 职责 |
|---|---|---|
| `data/` | 配置 | 花色与 `CardKind`、槽位与关卡配方、牌面外观 |
| `model/` | Model | 牌数据结构、三牌堆、遮挡系统、BFS 可解性 |
| `controller/` | Controller | 规则入口、Handler 注册表、Command 实现、命令历史栈 |
| `session/` | Session | 会话构造、发牌、restart |
| `presenter/` | Presenter | DTO 定义、buildViewState、用户操作入口 |
| `view/` | View | CardView、GameScene、触摸处理、反馈动画 |
| `platform/` | 平台隔离 | 屏幕可用高度（当前仅 macOS 实现） |

### 依赖规则

| 层 | 可依赖 | 禁止依赖 |
|---|---|---|
| View | Presenter 公开 API、ViewTypes、cocos | Model、Controller、CardModel |
| Presenter | Model、Controller、Session、ViewTypes | cocos |
| Model / Controller / data | 彼此 | View、Presenter、cocos |

---

## 二、游戏完整流程

### 2.1 启动与初始化

```
AppDelegate::applicationDidFinishLaunching
  → （Win / Linux 桌面）创建窗口，frameZoomFactor 默认 scale = 1.0（物理窗口 1080×2080）
  → （macOS）ScreenHelper::getAvailableScreenHeight
        → 减去标题栏与底边距后，若可用高度 < 设计高度，则 scale = 可用高度 / 2080
        → 否则保持 scale = 1.0
  → 设计分辨率 1080×2080，策略 FIXED_WIDTH
  → setDisplayStats(false)
  → GameScene::create()
```

### 2.2 场景初始化

```
GameScene::init
  → 创建 GamePresenter（传入 LevelDef）
      → 构造 GameSession（LevelDef）
          → GameModel::setupGame：洗牌、发牌、建遮挡关系
          → BFS 验证可解性
  → 绘制背景（桌面区 #7388A3 / 堆牌区 #A7BDD5）
  → createAllCardViews：按 ViewState 创建 CardView × N（_cardViewMap，无单独「牌堆」节点）
  → setupPileButtons（撤回）、setupRestartButton（重来，通关前隐藏）
  → applyViewState(animate=false) 摆位桌面 / 手牌 / 备用牌
  → setupSceneTouch（注册触摸监听，_inputLocked 防连点）
```

### 2.3 点击桌面牌

```
onTouchEnded
  → hitTestTableauCardId
  → GamePresenter::onTableauTapped(cardId)
      │
      ├─ Blocked
      │     → highlightCardIds
      │     → playBlockedFeedback（垂直轻弹 + 高亮）
      │
      ├─ NoMatch
      │     → playRejectShake（左右晃动）
      │
      └─ Matched
            → Handler::createCommand → MatchCommand::execute()
                  → onTableauCardRemoved（遮挡计数 -1）
                  → removeFromTableau、pushHand
                  → UndoManager::record
            → flyingCardId = 当前手牌顶（刚匹配的牌）
            → handlePresenterOutcome → applyViewState(animate=true, flyingCardId)
```

### 2.4 点击备用牌

```
hitTestReserveTopCardId
  → GamePresenter::onReserveTapped
      → GameController::tryDraw
          → DrawCommand::execute()（drawFromReserve、pushHand）
          → UndoManager::record
      → flyingCardId = 当前手牌顶（刚抽到的牌）
      → handlePresenterOutcome → applyViewState(animate=true, flyingCardId)
```

### 2.5 撤回

```
撤回按钮点击
  → GamePresenter::onUndoTapped
      → flyingCardId = 撤销前手牌顶
      → GameController::tryUndo
          → UndoManager::undo() → CommandHistory::undoLast → ICommand::undo()
                MatchCommand::undo：popHand、insertToTableau、onTableauCardRestored（遮挡 +1）
                DrawCommand::undo：popHand、pushToReserve
      → handlePresenterOutcome → applyViewState(animate=true, flyingCardId)
```

### 2.6 重新开始

```
重来按钮点击
  → GamePresenter::onRestartTapped
      → GamePresenter::restart()
          → GameSession::restart()
                → GameController::resetHistory()（UndoManager → CommandHistory::clear）
                → GameModel::setupGame（重新洗牌发牌）
          → 重建 _slotIndex（cardId → 布局槽位）
      → outcome.fullRestart = true
  → GameScene::handlePresenterOutcome
      → destroyAllCardViews → createAllCardViews → applyViewState(animate=false)
```

### 2.7 通关判定

每次操作后 `buildViewState` 调用 `GameModel::isTableauClear()`；若返回 true，`ViewState::showRestartButton = true`，GameScene 显示重来按钮（当前用作再来一局入口）。

---

## 三、各层职责

### 3.1 配置层（data/）

**职责**：存放所有与引擎无关的静态配置，不持有任何运行时状态。

- `CardDef.h`：花色枚举、`CardKind`（功能牌类型）、红黑/点数/花色字符串工具。注释中约定：**行为**在 Handler 注册，**外观**在 CardAppearance。
- `CardAppearance.h/.cpp`：`CardKind` → `CardVisualStyle`（是否绘制标准点数花色贴图、是否叠加中央 overlay 图）。与 Handler 注册表对称，专管「画成什么样」。
- `LayoutDef.h`：槽位定义（`SlotDef`：x/y/zOrder/blockedBy 列表）和布局容器（`LayoutDef`）。
- `LayoutDef_Pyramid.h/.cpp`：10 槽金字塔布局实例 `Layouts::PYRAMID`。新增形状只需新建同类文件。
- `LevelDef.h`：关卡配方（引用哪个 Layout、备用牌数量、是否要求可解）。
- `Levels.h/.cpp`：关卡目录，当前登记 `Levels::PYRAMID_TEST`。

---

### 3.2 Model 层（model/）

**职责**：持有所有运行时数据，提供数据读写接口，不包含任何规则判断或 cocos 调用。

- `CardModel`：单张牌（**cardId**、**CardKind**、suit、value、blockerCount / `isAccessible()`）。所有权由 `GameModel::_allCards`（`unique_ptr`）独占；Presenter 内部可用裸指针，View **只用 cardId**。
- `GameModel`：
  - 三牌堆：桌面堆（tableau）、手牌堆（hand）、备用堆（reserve）。
  - 发牌：洗 52 张后取本关所需张数（桌面槽位数 + 初始手牌 + 备用牌数）；**槽位与遮挡由 LayoutDef 固定，花色点数每局随机**。按槽位填入 `_tableau`，建立 `_unlocks` 遮挡逆映射。
  - 遮挡系统：`onTableauCardRemoved` / `onTableauCardRestored` 维护 `blockerCount`。
  - BFS 可解性：`requireSolvable` 时循环重洗直至 BFS 判定有解（当前实现未使用 `solvableMaxAttempts` 上限）。
  - 查询：`getCardById`、`getHandTop`、`isTableauClear`。

---

### 3.3 Controller 层（controller/）

**职责**：规则校验与命令调度，不持有 cocos 对象，不直接操作 View。

#### 规则入口（GameController）

- `canTableauAction(card)`：查 Handler，`preview()` 是否为 `CanExecute`（用于 NoMatch 反馈，不改 Model）。
- `tryTableauAction(card)`：`preview` 通过 → `createCommand` → `execute` → `UndoManager::record`。
- `tryDraw()` / `tryUndo()`：抽牌与撤销，同样走 Command + 历史栈。
- `TableauActionResult`：执行结果占位；复杂功能牌（如一次影响多张）可在此扩展字段，Presenter 不必再扫 Model。

#### 卡牌点击注册表（TableauCardHandlerRegistry）

每种 `CardKind` 对应一个 `ITableauCardHandler`（如 `MatchTableauHandler` 处理普通匹配：`preview` 看点数差是否为 1，`createCommand` 返回 `MatchCommand`）。注册表在 Controller 构造时填充。

#### 命令系统（command/ + history/）

| 类 | 职责 |
|---|---|
| `ICommand` | `execute()` / `undo()` 接口 |
| `MatchCommand` | 桌面牌→手牌：`execute` 先 `onTableauCardRemoved` 再移入 hand；`undo` 插回原槽并恢复遮挡 |
| `DrawCommand` | 备用牌→手牌；undo 压回备用堆 |
| `CommandHistory` | 线性撤销栈（push / undoLast / clear） |
| `UndoManager` | 撤销门面，后续可聚合 Redo、快照等扩展后端 |

新增撤销类型只需实现 `ICommand` 并在对应操作处压栈，UndoManager 接口不变。

---

### 3.4 Session 层（session/）

**职责**：绑定一局游戏的完整生命周期，隔离多局状态。

- `GameSession`：持有 `LevelDef`、`GameModel` + `GameController`，构造时发牌；`restart()` 调用 `resetHistory()` 并 `setupGame()` 重发。
- 废弃全局单例：每次开局构造新 Session，Presenter 持有 Session，历史操作随 Session 一起销毁，不同局之间状态完全隔离。

---

### 3.5 Presenter 层（presenter/）

**职责**：界面层与控制层之间的中间层，承担交互决策，不持有任何 cocos 节点。

- `ViewTypes.h`：定义 View ↔ Presenter 的数据契约（无 cocos）：
  - `CardViewSpec`：单张牌渲染快照（**cardId、CardKind**、suit、value、pile、position、zOrder、slotIndex）。
  - `ViewState`：整屏快照，View 每次刷新的唯一数据来源。
  - `PresenterOutcome`：操作结果（success、fullRestart、tableauResult、primaryCardId、flyingCardId、highlightCardIds）。
- `GamePresenter`：
  - 维护 `_slotIndex`（cardId → 布局槽位索引）；`restart()` 在 Session 重发后重建该映射。
  - `buildViewState()`：读 Model → 组装 `ViewState` DTO，View 只读此结构渲染，不直接访问 Model。
  - `onTableauTapped / onReserveTapped / onUndoTapped / onRestartTapped`：四个用户操作入口，调用 Controller 后返回 `PresenterOutcome`，由 View 决定播放何种动画。

---

### 3.6 View 层（view/）

**职责**：所有 cocos 节点管理、触摸处理、动画播放；不写入任何游戏状态。

- `CardView`：单张牌节点；`create(cardId, kind, suit, value)` 时根据 CardAppearance 决定画标准点数花色或仅底图 + overlay；尺寸 182×282；不持有 `CardModel*`。
- `GameScene`：
  - `createAllCardViews / destroyAllCardViews`：维护 `_cardViewMap`（cardId → CardView*）。
  - `applyViewState(state, animate, flyingCardId)`：唯一的界面刷新入口，按 ViewState 设置位置/zOrder/可见性；flying 牌 zOrder=200 飞向目标位置。
  - `handlePresenterOutcome(outcome)`：分发操作结果，决定播放 reject 晃动、blocked 弹动+高亮，还是触发 applyViewState 刷新。
  - `hitTestTableauCardId / hitTestReserveTopCardId`：几何 hit-test，按 zOrder 从高到低命中，返回 cardId。
  - `_inputLocked`：动画播放期间锁定输入，防止连点导致状态错乱。

---

### 3.7 Platform 层（platform/）

**职责**：隔离平台相关 API；头文件声明统一接口，按平台分文件实现。

- `ScreenHelper::getAvailableScreenHeight()`：**当前仅有 macOS 实现**（`platform/mac/ScreenHelper.mm`，读取 `NSScreen.visibleFrame`，已排除菜单栏与 Dock）。`AppDelegate` 仅在 `CC_PLATFORM_MAC` 下调用：默认 `scale = 1.0`，仅当可用高度小于设计高度时才降低 `scale`。Win / Linux 桌面版当前固定 `scale = 1.0`，尚未接入屏高检测。iOS 等平台尚未实现。
