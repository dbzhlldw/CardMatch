// ViewTypes — Presenter 与 View 之间的 DTO，无 cocos 依赖
#pragma once
#include <vector>
#include "data/CardDef.h"

struct Vec2f {
    float x = 0.f;
    float y = 0.f;
};

enum class CardPile {
    Tableau,
    Hand,
    Reserve,
};

enum class TableauTapResult {
    None,
    Matched,
    NoMatch,
    Blocked,
};

// 单张牌的渲染快照
struct CardViewSpec {
    int      cardId    = -1;
    CardKind kind      = CardKind::Normal;
    Suit     suit      = Suit::Heart;
    int      value     = 1;
    CardPile pile      = CardPile::Tableau;
    Vec2f    position;
    int      zOrder    = 0;
    int      slotIndex = -1; // 桌面 layout 槽位，发牌后固定；-1 表示非桌面牌
};

// 整屏渲染快照，View 只读此结构刷新
struct ViewState {
    std::vector<CardViewSpec> cards;
    int      reserveTopCardId = -1;
    bool     showRestartButton = false;
    bool     canUndo           = false;
    Vec2f    undoButtonPosition;
    Vec2f    restartButtonPosition;
};

// 用户操作结果：状态是否变更、反馈类型、动画目标
struct PresenterOutcome {
    bool             success       = false;
    bool             fullRestart   = false;
    TableauTapResult tableauResult = TableauTapResult::None;
    int              primaryCardId = -1;
    int              flyingCardId  = -1;
    std::vector<int> highlightCardIds;
};
