// GamePresenter — 布局计算、触摸决策、驱动 Session/Controller，不算动画，只给目标坐标
#pragma once
#include <unordered_map>
#include <vector>
#include "data/LevelDef.h"
#include "session/GameSession.h"

class CardModel;

// 二维坐标（设计分辨率，与 cocos Vec2 解耦）
struct Vec2f {
    float x = 0.f;
    float y = 0.f;
};

// 单张牌的布局结果，供 GameScene 应用位置与 z-order
struct CardLayoutSpec {
    CardModel* card   = nullptr;
    Vec2f      position;
    int        zOrder = 0;
    bool       faceUp = true;
};

// Scene 几何 hit-test 后的桌面牌候选，Presenter 据此选最终目标
struct TableauHitCandidate {
    CardModel* card      = nullptr;
    int        zOrder    = 0;
    bool       matchable = false;
};

// 用户操作结果：是否成功、飞行动画牌、是否整局重开
struct PresenterOutcome {
    bool       success     = false;
    CardModel* flyingCard  = nullptr; // 需要做飞行动画的牌
    bool       fullRestart = false;
};

class GamePresenter {
public:
    struct Layout {
        static constexpr float DESIGN_WIDTH         = 1080.f;
        static constexpr float DESIGN_HEIGHT        = 2080.f;
        static constexpr float PILE_AREA_H          = 580.f;
        static constexpr float TABLEAU_AREA_H       = DESIGN_HEIGHT - PILE_AREA_H;
        static constexpr float RESERVE_STACK_OFFSET = 15.f;
    };

    explicit GamePresenter(const LevelDef& level);

    GameModel&            model();
    const GameModel&      model() const;
    GameController&       controller();
    const GameController& controller() const;
    const LevelDef&       level() const;

    void restart();

    // --- 布局 ---
    Vec2f handPilePosition() const;
    Vec2f undoButtonPosition() const;
    Vec2f restartButtonPosition() const;
    std::vector<CardLayoutSpec> buildCardLayout() const;
    bool isRestartButtonVisible() const;

    // 桌面牌槽位索引（发牌时固定，不随 remove/insert 改变）
    void registerTableauSlot(CardModel* card, int slotIndex);
    void clearSlotRegistry();

    // --- 触摸决策（Scene 负责几何检测，Presenter 负责选牌与规则）---
    CardModel* pickTableauCard(const std::vector<TableauHitCandidate>& candidates) const;
    std::vector<CardModel*> getDirectBlockers(CardModel* card) const;
    CardModel* reserveTopCard() const;
    bool       isReserveTop(CardModel* card) const;

    // --- 用户操作 ---
    bool canUndo() const;
    PresenterOutcome onTableauTapped(CardModel* card);
    PresenterOutcome onReserveTapped();
    PresenterOutcome onUndoTapped();
    PresenterOutcome onRestartTapped();

private:
    GameSession _session;
    std::unordered_map<CardModel*, int> _slotIndex;

    Vec2f reservePositionFor(int index, int total) const;
    Vec2f slotPositionFor(CardModel* card) const;
    int   slotZOrderFor(CardModel* card) const;
};
